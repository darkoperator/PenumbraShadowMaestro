#!/usr/bin/env python3
"""
Generate a random 16-byte ESPNOW LMK as 32 hex characters.

Usage:
  python3 gen_lmk.py           # prints one key
  python3 gen_lmk.py 5         # prints 5 keys
"""
import os, sys, binascii

def gen_key():
    return binascii.hexlify(os.urandom(16)).decode('ascii').upper()

def main():
    n = 1
    if len(sys.argv) > 1:
        try:
            n = max(1, int(sys.argv[1]))
        except Exception:
            pass
    for _ in range(n):
        print(gen_key())

if __name__ == '__main__':
    main()

