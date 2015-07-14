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
  %me% <interface>

description:
  Reflect packets received on an interface back to the sender.
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
while args and args[0] and args[0][0] == '-':
    if args[0] == '-h' or args[0] == '--help':
        usage_msg(sys.stdout)
        sys.exit(0)
    else:
        usage_err()
if len(args) != 1:
    usage_err()
interface = args[0]

scs = sc.new_session()
thrd = scs.new_thread()

# Create a VI to capture received packets.  Filter out multicast and
# broadcast, and forward unicast to a reflect node and then back out of the
# same interface.
vi = thrd.new_vi(interface)
vi.add_stream(scs.new_stream("all"))
pipeline = vi
pipeline = sc.connect(pipeline, thrd.new_node('sc_filter',
                                              args=dict(bpf='not multicast')))
pipeline = sc.connect(pipeline, thrd.new_node('reflect'))
pipeline = sc.connect(pipeline, to_interface=interface)

scs.go()
while True:
    time.sleep(10000)
