"""UDP echo server for the network probe (lib/src/net/probe.cc).

Usage: python tools/net_echo.py [port]
Replies "ECHO:<payload>" to every datagram and prints who sent it.
"""
import socket
import sys

port = int(sys.argv[1]) if len(sys.argv) > 1 else 5000
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", port))
print(f"listening on udp/{port}")
while True:
    data, addr = sock.recvfrom(2048)
    print(addr, data)
    sock.sendto(b"ECHO:" + data, addr)
