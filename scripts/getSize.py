#!/usr/bin/env python3

from math import ceil
from os import path
from sys import argv

from ledgerblue.hexParser import IntelHexParser

model = argv[1] if len(argv) != 1 else "nanos"
if model == "s" or model == "s2" or model == "x":
    model = "nano" + model
hex_path = "app/build/" + model + "/bin/app.hex"
block_size = 32  # nanosp and stax
if model == "nanos":
    block_size = 2048
elif model == "nanox":
    block_size = 4096

if not path.isfile(hex_path):
    raise ValueError("hex file not found in" + hex_path)

parser = IntelHexParser(hex_path)
bytes = parser.maxAddr() - parser.minAddr()
size_in_bytes = ceil(bytes / block_size) * block_size
print(ceil(size_in_bytes / 1024))  # in KiB
