#!/usr/bin/env python
# coding=utf-8

import re

class Sysinfo:

    def __init__(self):
        self.hostname = "N/A"
        self.os = "N/A"
        self.mac = []
        self.ip = []
        self.mask = []
        self.gtw = []

    def parse(self,data):
        data = data.split(";")
        self.hostname = data[0]
        self.os = data[1]
        networks = re.findall("{(.*?)}", data[2])
        self.mac = []
        self.ip = []
        self.mask = []
        self.gtw = []
        for net in networks:
            net = net.split("|")
            self.mac.append(net[0])
            self.ip.append(net[1])
            self.mask.append(net[2])
            self.gtw.append(net[3])

    def debug(self):
        print("Hostname : "+self.hostname)
        print("OS : "+self.os)
        print("Network adapters : ")
        for i in range(len(self.mac)):
            print("\tMAC : "+self.mac[i])
            print("\tIP : "+self.ip[i])
            print("\tMASK : "+self.mask[i])
            print("\tGTW : "+self.gtw[i])
            print("")


#enum = "DESKTOP-GJBVPRT;10.0;{58-A0-23-28-D9-FD|192.168.1.105|255.255.255.0|192.168.1.1}{58-A0-23-28-D9-FE|192.168.1.106|255.255.255.0|192.168.1.1}{58-A0-23-28-D9-FF|192.168.1.107|255.255.255.0|192.168.1.1}"