#!/usr/bin/env python
# coding=utf-8

import os
import uuid
import pickle
from sysinfo import *
from datetime import datetime

clients  = {}

class Client:

    def __init__(self, ip, client_id=None):
        if client_id == None:
            self.gen_id()
        else:
            self.id = client_id
        self.ip = ip
        self.heartbeat()
        self.order = []
        self.info = Sysinfo()
        self.save()
        self.frequency = 2000
        self.replication = 0
        self.is_admin = "N/A"

    def gen_id(self):
        self.id = str(uuid.uuid4().hex[:6])
        while os.path.isdir("./clients/" + self.id):
            self.id = str(uuid.uuid4().hex[:6])
        os.mkdir("./clients/" + self.id)

    def heartbeat(self):
        self.last_seen = datetime.now()

    def get_info(self):
        info = {}
        info["id"] = self.id
        info["ip"] = self.ip
        info["hostname"] = self.info.hostname
        info["os"] = self.info.os
        info["last_seen"] = self.last_seen.strftime("%m/%d/%Y %H:%M:%S")
        info["order"] = self.order
        info["replication"] = self.replication
        info["is_admin"] = self.is_admin
        networks = []
        for i in range(len(self.info.mac)):
            networks.append({'mac':self.info.mac[i], 'ip':self.info.ip[i], 'mask':self.info.mask[i], 'gtw':self.info.gtw[i]})
        info["networks"] = networks
        return info

    def save(self):
        with open("./clients/" + self.id + "/" + self.id + ".obj", "wb") as f:
            pickler = pickle.Pickler(f, pickle.HIGHEST_PROTOCOL)
            pickler.dump(self)


def load(client_id):
    with open("./clients/" + client_id + "/" + client_id + ".obj", "rb") as f:
        unpickler = pickle.Unpickler(f)
        return unpickler.load()


def check_pulse(clients, timeout):
    now = datetime.now()
    for i in list(clients):
        if (now.minute - clients[i].last_seen.minute) >= timeout and (now.second >= clients[i].last_seen.second):
            del clients[i]