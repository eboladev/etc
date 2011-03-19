#!/usr/bin/env python

import sys
import os
import struct
import array
import time
import select
import binascii
import math
import getopt
import string
import socket

# total size of data (payload)
ICMP_DATA_STR = 56  

# initial values of header variables
ICMP_TYPE = 8
ICMP_TYPE_IP6 = 128
ICMP_CODE = 0
ICMP_CHECKSUM = 0
ICMP_ID = 0
ICMP_SEQ_NR = 0

def _construct(id, size):
    """Constructs a ICMP echo packet of variable size
    """

    # size must be big enough to contain time sent
    if size < int(struct.calcsize("d")):
        _error("packetsize to small, must be at least %d" % int(struct.calcsize("d")))
    
    # construct header
    header = struct.pack('bbHHh', ICMP_TYPE, ICMP_CODE, ICMP_CHECKSUM, \
                         ICMP_ID, ICMP_SEQ_NR+id)

    # if size big enough, embed this payload
    load = "-- IF YOU ARE READING THIS YOU ARE A NERD! --"
    
    # space for time
    size -= struct.calcsize("d")

    # construct payload based on size, may be omitted :)
    rest = ""
    if size > len(load):
        rest = load
        size -= len(load)

    # pad the rest of payload
    rest += size * "X"

    # pack
    data = struct.pack("d", time.time()) + rest
    packet = header + data          # ping packet without checksum
    checksum = _in_cksum(packet)    # make checksum

    # construct header with correct checksum
    header = struct.pack('bbHHh', ICMP_TYPE, ICMP_CODE, checksum, ICMP_ID, \
                         ICMP_SEQ_NR+id)

    # ping packet *with* checksum
    packet = header + data 

    # a perfectly formatted ICMP echo packet
    return packet

def _in_cksum(packet):
    """THE RFC792 states: 'The 16 bit one's complement of
    the one's complement sum of all 16 bit words in the header.'

    Generates a checksum of a (ICMP) packet. Based on in_chksum found
    in ping.c on FreeBSD.
    """

    # add byte if not dividable by 2
    if len(packet) & 1:              
        packet = packet + '\0'

    # split into 16-bit word and insert into a binary array
    words = array.array('h', packet) 
    sum = 0

    # perform ones complement arithmetic on 16-bit words
    for word in words:
        sum += (word & 0xffff) 

    hi = sum >> 16 
    lo = sum & 0xffff 
    sum = hi + lo
    sum = sum + (sum >> 16)
    
    return (~sum) & 0xffff # return ones complement

def pingNode(node):

    """Pings a node."""

    # Send 1 ping
    timeout = 1.0
    size = ICMP_DATA_STR
    number = 1
    start = 1; mint = 999; maxt = 0.0; avg = 0.0
    lost = 0; tsum = 0.0; tsumsq = 0.0
    pong = None
    iwtd = []

    # create the IPv4 socket and the ping packet; send the ping
    pingSocket = socket.socket(socket.AF_INET, socket.SOCK_RAW, \
                               socket.getprotobyname("icmp"))
    packet = _construct(start, size)
    pingSocket.sendto(packet,(node,1))

    # wait until there is data in the socket
    while 1:
        # input, output, exceptional conditions
        iwtd, owtd, ewtd = select.select([pingSocket], [], [], timeout)
        break

    # data on socket - this means we have an answer
    if iwtd:
        pong, address = pingSocket.recvfrom(size+48)
        rawPongHop = struct.unpack("s", pong[8])[0]
        pongHop = int(binascii.hexlify(str(rawPongHop)), 16)
        pongHeader = pong[20:28]
        pongType, pongCode, pongChksum, pongID, pongSeqnr = \
                  struct.unpack("bbHHh", pongHeader)
        if not pongSeqnr == start:
            pong = None

    pingSocket.close()
    return pong != None
    
def _error(err):
        raise Exception, str(err)
    
if __name__ == '__main__':
    print pingNode(sys.argv[1])
    sys.exit(0)
