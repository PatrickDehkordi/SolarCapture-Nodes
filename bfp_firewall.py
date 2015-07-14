'''
This file is part of Solarflare SolarCapture.

You may freely copy code from this sample to incorporate into your own
code.


Copyright 2012-2015  Solarflare Communications Inc.
                     7505 Irvine Center Drive, Irvine, CA 92618, USA

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''

#!/usr/bin/python
######################################################################

import sys, time, os

top = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '../../..'))
sys.path.append(os.path.join(top, 'src', 'python'))
import solar_capture as sc


usage_text = """\
usage:
  %me% <source-interface> <dest-interface> <bpf-filter>

description:
  Forward packets from source-interface to dest-interface.  Packets
  matching the filter are discarded.  The filter is specified using BPF
  syntax.  (See 'man pcap-filter').

examples:
  # Forward from eth1 to eth2, blocking packets to/from TCP port 80.
  %me% eth1 eth2 "tcp port 80"

"""


def usage_msg(strm):
    me = os.path.basename(sys.argv[0])
    strm.write(usage_text.replace('%me%', me))


def usage_err(msg=None):
    if msg:
        sys.stderr.write()
    usage_msg(strm=sys.stderr)
    sys.exit(1)


######################################################################
# main()

# Get command line arguments.
args = sys.argv[1:]
while args and args[0] and args[0][0] == '-':
    if args[0] == '-h' or args[0] == '--help':
        usage_msg(sys.stdout)
        sys.exit(0)
    else:
        usage_err()
if len(args) != 3:
    usage_err()
if_in = args[0]
if_out = args[1]
bpf_filter = args[2]

scs = sc.new_session()
thrd = scs.new_thread()

# Create a VI to capture received packets.  Forward them via an sc_filter
# node to the destination interface.
vi = thrd.new_vi(if_in)
vi.add_stream(scs.new_stream("all"))
filter = thrd.new_node('sc_filter', args=dict(bpf=bpf_filter))
sc.connect(vi, filter)
sc.connect(filter, 'not_matched', to_interface=if_out)

scs.go()
while True:
    time.sleep(10000)
