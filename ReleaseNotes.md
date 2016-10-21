New Features in v1.6.0
----------------------

1) Documentation: This release expands the C bindings documentation to
   cover all public APIs.  Built-in nodes are also now documented.  This
   documentation is included in the SolarCapture packages in HTML form and
   can be downloaded as a PDF (SF-115721-CD) from support.solarflare.com.

2) Shared memory: Shared memory allows the transfer of packets between two
   or more SolarCapture processes (solar_balancer, solar_capture,
   solar_libpcap, custom applications) via a shared memory channel.  This
   mechanism supports fanout, and can be used to decouple different parts
   of a processing pipeline.

3) TCP tunnels: Tunnel nodes allow the transfer of packets between two
   SolarCapture processes via a TCP socket.  Each tunnel supports multiple
   input and output links, so that control and data-paths can be
   multiplexed together.  Tunnel nodes allow packets to be streamed to
   other hosts, and allow SolarCapture processing to be distributed over
   multiple hosts.

4) Load balancing: The new solar_balancer utility offers active flow-aware
   load balancing.  It distributes flows to consumers over the new shared
   memory channels.  Consumers can use the C bindings, libpcap or DAQ.

5) Packets with the cPacket trailer can now be decoded with the
   'cpacket_ts' option or sc_cpacket_ts node.

6) A 'postrotate_command' option is supported, to allow a command to be
   executed after each file rotation.

7) New filters on ethertype and IP protocol are supported, subject to
   restrictions (see known issues below).  You may need to upgrade adapter
   firmware to enable these new filter types.

8) Many bugs have been fixed.  Please see the ChangeLog for a complete
   list.

9) There are numerous other improvements: More flexibility, new APIs,
   improved performance, more built-in nodes.  Please see the ChangeLog and
   User Guides for details.


Known Issues in v1.6.0
----------------------

1) When using a vlan only filter to capture
  a. Multicast packets that match the filter are captured and also
     delivered to the kernel (if it wants them).
  b. Unicast packets with destination MAC equal to the port's MAC address
     are not captured.

2) Ethertype filters do not match IPv4 or IPv6 protocols.  (This is a
   limitation of the adapter firmware rather than of SolarCapture).

3) IP protocol filters do not match UDP and TCP protocols.  (This is a
   limitation of the adapter firmware rather than of SolarCapture).


New Features in v1.3.1
----------------------

This section summarises the main features added since the 1.3.0 release.
Refer to the ChangeLog for a full list of bug fixes and other changes.

1) Performance improvements in the core, libpcap and Snort DAQ.

2) Support for forwarding packets at very high rates with packed-stream
   mode.

3) Much improved documentation for the C API, installed at
   /usr/share/doc/solar_capture-$version/c_api.

4) Create directories (if needed) for output files.  This is particularly
   useful when using file rotation and putting output files into
   directories that include the time in their names.

5) Support for RHEL7 (requires OpenOnload-201502 or later).
   Support for RHEL5 is dropped.
   Support for Ubuntu 12.04 LTS, Ubuntu 14.04 LTS and Ubuntu 14.10;
   the release package now includes ".deb" files.
   Refer to INSTALL-deb.txt for installation instructions on Ubuntu.


Known Issues in v1.3.1
----------------------

1) The "require_huge_pages" and "request_huge_pages" attributes only take
   effect on kernel versions 2.6.32 and newer.  On older kernels, the
   attributes are ignored.

2) Packed-stream mode is only supported on kernel versions 2.6.32 and newer.

3) Setting EF_VI_PD_FLAGS in the environment results in a failure if the
   vi_mode attribute is set to "auto" (default) or "packed_stream".

4) If traffic is to be captured from an adapter other than an AOE while
   solar_aoed is running, vi_mode must be set to "normal" or
   "packed_stream".

5) If sfptpd is not running when using hardware timestamps, it is possible for
   all packets to have identical timestamps.  The solution is to start sfptpd.

6) Apparmor, if running, blocks operation of solar_libpcap.
   To use solar_libpcap, apparmor must be disabled or an apparmor profile for
   solar_libpcap must be created.


New Features in v1.3.0
----------------------

This section summarises the main features added since the 1.2.2
release:

1) A new capture-packed-stream firmware variant is now supported.  This allows
   for line rate capture on 7000 series adapters.  Packed stream mode is enabled
   by default in SolarCapture if the capture-packed-stream firmware variant is
   loaded.  This behaviour can be controlled by using the new "vi_mode"
   attribute.

2) Egress capture is now supported on 7000 series adapters.  This adds to
   existing support for egress capture on AOE adapters.

3) SolarCapture threads can operate in interrupt driven mode as well as busy
   waiting.  To select between these, two new command line options have been
   added: capture_busy_wait and writeout_busy_wait.

4) A new packet playback tool called solar_replay has been added.  This allows
   packets from a file in pcap format to be replayed through a Solarflare
   adapter interface with flexible control over replay speed and bandwidth.

5) On 7000 series adapters, SolarCapture can be configured to capture packets
   while preserving the frame check sequence (FCS).

6) A Snort DAQ is included in the package.  This supports the read-file, passive
   and inline modes.

7) Added a new environment variable SC_PCAP_RECV_BATCH to control polling rate
   when using solar_libpcap.

8) solar_capture_doc has been added to help document SolarCapture.  Currently it
   documents all attributes that can be set via SC_ATTR.

9) solar_capture can now accept a config_file option, which specifies a file
   containing a list of arguments, one per line.


New Features in v1.2.2
----------------------

1) Added a new environment variable SC_PCAP_NANOSEC=1 to select nanosecond
   precision when using the libpcap bindings.  This is an alternative to
   using the pcap_set_tstamp_precision() call.  NB. The application using
   libpcap must separately be configured to expect nanosecond precision.


New Features in v1.2.1
----------------------

This section summarises the main features added since the 1.2.0
release:

1) The SolarCapture AOE package has been updated to include
   OpenOnload-201310-u2.

2) Disk capture performance has been improved.

3) A new configuration item for AOE SolarCapture allows balancing of
   transmission buffer space (RHD buffer) across the number of RHD channels
   actually being used.

4) Updated libpcap in solar_libpcap to v1.5.3.
   The following libpcap APIs are now supported:
     pcap_set_tstamp_type()       -- used to request h/w timestamps
     pcap_set_tstamp_precision()  -- used to request nanosec precision

5) New syntax for enabling Arista hardware timestamps.  Instead of:

     solar_capture ... "sc_arista_ts:kf_ip_dest=224.1.2.3;kf_eth_dhost=..."

   instead do:

     solar_capture ... "arista_ts=kf_ip_dest=224.1.2.3"

   If using multicast for the keyframe IP address, you no longer need to
   specify the mac address (kf_eth_dhost).

   When using this new syntax solar_capture will automatically install a
   stream filter to capture the keyframes if needed.

6) Arista hardware timestamps can now be used with multiple capture cores,
   provided that a Flareon (or future) adapter is used.


New Features in v1.2.0
----------------------

This section summarises the main features added since the 1.0.2
release:

1) libpcap support
 - applications using the libpcap API can link against SolarCapturePro
 and capture and/or inject packets.

2) Hardware timestamp support for the SFN7000-series and AOE adapters
 - SolarCapturePro can utilise the hardware receive timestamps

3) Support for software based filtering.
 - SolarCapturePro can filter packets in software using user-provided
 filters in the BPF syntax.

4) "Sniff" mode
 - On suitable hardware (7000-series and AOE) SolarCapturePro can
 capture packets without stealing them from regular receivers

5) Improved diagnostics and statistics
 - solar_capture_monitor can show packet rate, bandwidth and Arista
 timestamping statistics.

6) AOE support
 * Capturing in sniff and steal modes.
 * Richer filtering specification.
 * Capture of rx and tx on both ports.
 * 2Gb of buffering per capture point.
 * Loss-less delivery to the host CPU.
 * Hardware timestamps.

7) Application Clustering
 - spread capture load across a set of VIs so that multiple
 threads/processes on different CPUs can cooperate to process it.



Support
-------

 Please contact your local Solarflare support representative or email
 <support@solarflare.com>.


Copyright
---------

 Copyright (c) 2015, Solarflare Communications Inc.  All rights reserved.
 Please see LICENSE.txt for terms of use.
