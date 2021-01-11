#! /usr/bin/env python

import subprocess

for i in range(200):
    subprocess.call(["./main.o" , "output/sample.png" , "600", "400" , "output/sample.png"])