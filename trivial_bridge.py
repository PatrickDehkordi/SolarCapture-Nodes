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


usage_text = """\
usage:
  %me% <interface1>:<interface2>...

description:
  Forward packets between network interfaces.  Each command line argument
  is a pair of interfaces that are connected together with a
  uni-directional channel.

examples:
  # Forward packets from eth2 to eth3
  %me% eth2:eth3

  # Bidirectional link between eth2 and eth3
  %me% eth2:eth3 eth3:eth2
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

top = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '../../..'))
sys.path.append(os.path.join(top, 'src', 'python'))

import solar_capture as sc


# Get command line arguments.
args = sys.argv[1:]
if_pairs = []
for arg in args:
    if arg == '-h' or arg == '--help':
        usage_msg(sys.stdout)
        sys.exit(0)
    ifs = arg.split(':')
    if len(ifs) != 2:
        usage_err()
    if_pairs.append(ifs)
if not if_pairs:
    usage_err()

# Create a session and a thread.
scs = sc.new_session()
thrd = scs.new_thread()

for intf_a, intf_b in if_pairs:
    # Create a VI on each interface to capture received packets.
    vi_a = thrd.new_vi(intf_a)
    vi_a.add_stream(scs.new_stream("all"))
    vi_b = thrd.new_vi(intf_b)
    vi_b.add_stream(scs.new_stream("all"))

    # Forward packets to the other interface.
    sc.connect(vi_a, to_interface=intf_b)
    sc.connect(vi_b, to_interface=intf_a)

scs.go()
while True:
    time.sleep(10000)
