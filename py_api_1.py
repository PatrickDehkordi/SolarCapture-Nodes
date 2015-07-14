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

'''
This file illustrates how to use the SolarCapture python bindings to
construct a custom capture configuration.  The example is deliberately kept
very simple, to illustrate the main concepts in SolarCapture.
'''

import sys, os, time, signal

top = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]), '../../..'))
sys.path.append(os.path.join(top, 'src', 'python'))
import solar_capture as sc


usage_text = """\
usage:
  %me% [--single-thread] <interface> <filename.pcap>

description:
  Capture packets from the named interface and write to a file.
"""


def usage_msg(strm):
    me = os.path.basename(sys.argv[0])
    strm.write(usage_text.replace('%me%', me))


def usage_err(msg=None):
    if msg:
        sys.stderr.write()
    usage_msg(strm=sys.stderr)
    sys.exit(1)


def main(args):
    # Get command line arguments.
    two_threads = True
    while args and args[0] and args[0][0] == '-':
        if args[0] == '-h' or args[0] == '--help':
            usage_msg(sys.stdout)
            sys.exit(0)
        elif args[0] == '--single-thread':
            two_threads = False
            args.pop(0)
        else:
            usage_err()
    if len(args) != 2:
        usage_err()
    intf = args[0]
    filename = args[1]

    # A SolarCapture session binds together a set of threads and components
    # that are doing a particular job.
    scs = sc.new_session()

    # Create the threads that will be used for capture and writing to disk.
    # It is usually a good idea to keep these in separate threads, as
    # otherwise writing to disk can block capture for long periods of time,
    # leading to packet loss in some configurations.
    cap_thread = scs.new_thread()
    if two_threads:
        writer_thread = scs.new_thread()
    else:
        writer_thread = cap_thread

    # Create a VI, which is used to receive packets from the network
    # adapter.
    vi = cap_thread.new_vi(intf)

    # Add the streams we want to capture.  This VI will capture all packets
    # arriving at the interface (except for any streams explicitly steered
    # elsewhere).
    vi.add_stream(scs.new_stream('all'))

    # SolarCapture nodes perform packet processing functions such as
    # monitoring, packet modification, writing to disk, I/O etc.  When
    # allocating nodes you can specify node-specific arguments, which may
    # be required or optional.
    #
    # The 'sc_writer' node writes packets to disk in pcap format.  The
    # 'snap' argument indicates the maximum number of bytes of each packet
    # that should be saved in the capture file.
    writer_args = dict(filename=filename, snap=60)
    writer = writer_thread.new_node('sc_writer', args=writer_args)

    # Connect the VI to the writer node.
    sc.connect(vi, writer)

    # Once we have created the necessary components, and linked them
    # together as desired, we kick off the actual packet handling.  This
    # call starts the managed threads and begins packet processing.
    scs.go()

    # Stop python from swallowing SIGINT!
    signal.signal(signal.SIGINT, signal.SIG_DFL)

    while True:
        time.sleep(10000)


if __name__ == '__main__':
    main(sys.argv[1:])
    sys.exit(0)
