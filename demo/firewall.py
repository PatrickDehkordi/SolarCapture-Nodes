#!/usr/bin/python
######################################################################

import sys, time, os
import solar_capture as sc


usage_text = """
usage:
  %prog% <external-intf> <internal-intf> <ssfe-config-file>

"""


def usage_msg(f):
    prog = os.path.basename(sys.argv[0])
    f.write(usage_text.replace('%prog%', prog))


def usage_err():
    usage_msg(sys.stderr)
    sys.exit(1)


def make_config_fifo(scs, fifo_path):
    # Set up a FIFO to accept OFE configuration updates at runtime
    if os.path.exists(fifo_path):
        os.unlink(fifo_path)
    fifo = os.mkfifo(fifo_path)
    fd = os.open(fifo_path, os.O_NONBLOCK | os.O_RDWR)
    thrd = scs.new_thread(attr=dict(busy_wait=0))
    fd_reader = thrd.new_node('sc_fd_reader', args=dict(fd=fd))
    line_reader = thrd.new_node('sc_line_reader')
    sc.connect(fd_reader, line_reader)
    return sc.connect(line_reader, thrd.new_node('sc_no_op'))


def make_inj(thrd, interface):
    args = dict(csum_ip=1, csum_tcpudp=1)
    return thrd.new_node('sc_injector', args=dict(csum_ip=1, csum_tcpudp=1,
                                                  interface=interface))


def make_slice(scs, ext_intf, int_intf, config_file, config_pipe):
    thrd = scs.new_thread()

    # NB. vi_mode=packed_stream isn't strictly required, but gives the
    # best level of performance.
    discard_mask = 3  # SC_CSUM_ERROR | SC_CRC_ERROR
    vi_attr = dict(discard_mask=discard_mask, vi_mode='packed_stream')

    ext_vi = thrd.new_vi(ext_intf, attr=dict(vi_attr, name="outside"))
    ext_vi.add_stream(scs.new_stream('all'))
    int_vi = thrd.new_vi(int_intf, attr=dict(vi_attr, name="inside"))
    int_vi.add_stream(scs.new_stream('all'))

    ext_inj = make_inj(thrd, ext_intf)
    int_inj = make_inj(thrd, int_intf)

    synp = thrd.new_node('sc_syn_proxy')
    ssfe = thrd.new_node('sc_solsec_fe', args=dict(config_file=config_file))

    sc.connect(config_pipe[0], 'config', ssfe, 'config')
    config_pipe[0] = ssfe

    sc.connect(ext_vi, ssfe, "inbound")
    sc.connect(ssfe, "inbound-1", int_inj)
    sc.connect(ssfe, "inbound-2", synp, "outside")
    sc.connect(synp, "inside", int_inj)

    sc.connect(int_vi, ssfe, "outbound")
    sc.connect(ssfe, "outbound-1", ext_inj)
    sc.connect(ssfe, "outbound-2", synp, "inside")
    sc.connect(synp, "outside", ext_inj)


def main(args):
    if len(args) != 3:
        usage_err()
    ext_intf, int_intf, config_file = args

    scs = sc.new_session()
    config_pipe = [make_config_fifo(scs, '/tmp/firewall_cfg')]
    make_slice(scs, ext_intf, int_intf, config_file, config_pipe)

    open('/tmp/firewall_cfg', 'w').write('enable inbound start_inbound\n')
    open('/tmp/firewall_cfg', 'w').write('enable outbound start_outbound\n')

    scs.go()
    while True:
        time.sleep(10000)


if __name__ == '__main__':
    main(sys.argv[1:])
