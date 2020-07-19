#!/usr/bin/env python
# coding=utf-8

import os
import random
import binascii
from hashlib import md5
from Crypto.Cipher import AES
from Crypto.Util import Counter

def random_string(length):
    charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    return ''.join(random.choice(charset) for i in range(length))

def gen_aes_key(id):
	key = md5(id.encode("utf-8")).digest()
	key = key.hex().encode("utf-8")
	return key

def int_of_string(s):
	return int(binascii.hexlify(s), 16)

def aes_encrypt(key, plaintext):
	iv = random_string(16).encode("utf-8")
	ctr = Counter.new(128, initial_value=int_of_string(iv))
	aes = AES.new(key, AES.MODE_CTR, counter=ctr)
	return (aes.encrypt(plaintext) + iv).hex()

def aes_decrypt(key, ciphertext):
	ciphertext = bytes.fromhex(ciphertext)
	iv = ciphertext[-16:]
	ctr = Counter.new(128, initial_value=int_of_string(iv))
	aes = AES.new(key, AES.MODE_CTR, counter=ctr)
	decrypted = aes.decrypt(ciphertext[:-16])
	try:
		return decrypted.decode("utf-8")
	except :
		try:
			return decrypted.decode("cp850");
		except :
			return decrypted.decode("cp1252")

