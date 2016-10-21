Introduction
============

 SolarCapture is a set of tools for capturing, timestamping, processing and
 injecting network traffic.  SolarCapture is able to capture packets
 received from the network at very high rates, apply filtering in hardware
 and software, perform custom processing, and write packets to disk in PCAP
 format.

 SolarCapture includes APIs for customisation and for embedding into
 applications, and a flexible framework for putting together applications
 from pluggable processing components.

 SolarCapture is written by Solarflare.  Please see LICENSE.txt for terms
 of use.

 This README file includes a brief summary of SolarCapture features.
 Please see the SolarCapture User Guide for more details.

 SolarCapture uses the kernel-bypass features of Solarflare network
 adapters to achieve very high levels of capture and injection performance.

 SolarCapture assigns highly accurate timestamps to captured packets.  When
 used with Solarflare's PTP-enabled adapters the capture timestamps are
 synchronised across the network.  On Flareon and AOE adapters timestamps
 can be taken in hardware by the adapter clock, giving very precise
 timestamps.

 SolarCapture consists of the following components:

 - Command line tools for capture, replay and monitoring.

 - Python bindings for creating custom configurations and applications.

 - C bindings for extending SolarCapture's processing pipeline and adding
   custom features.

 - C bindings for embedding SolarCapture into applications.
 
 https://rawgit.com/PatrickDehkordi/SolarCapture-Nodes/master/c_api/index.html


Dependencies
============

 SolarCapture uses device drivers that are included in the OpenOnload
 distribution.  OpenOnload is available from www.openonload.org.

 This release of SolarCapture is compatible with the following
 versions of Onload:

   * OpenOnload 201405-u2 and newer
   * EnterpriseOnload 3.1 and newer

 SolarCapture is tested on the following Linux distributions:

   Redhat Enterprise Linux 6.x
   Redhat Enterprise Linux 7.x
   Suse Enterprise Linux 11.x
   Suse Enterprise Linux 12.x
   Ubuntu 12.04 LTS
   Ubuntu 14.04 LTS
   Ubuntu 14.10


Command line usage
==================

 SolarCapture can be invoked from the command line.  By default it
 captures all traffic on an interface, which requires root privileges.
 For example, to capture all traffic arriving on interface eth2 and
 eth3:

   $ solar_capture eth2=./eth2.pcap eth3=./eth3.pcap

 To write timestamps with nanosecond resolution instead of the default
 microseconds:

   $ solar_capture format=pcap-ns eth2=./eth2.pcap

 Note that the nano-second PCAP format is understood by a number of
 tools including wireshark, but not by tcpdump.

 There are a number of tunables and options for managing what streams
 of packets are captured.  For details run:

   $ solar_capture help

 To use the libpcap bindings, prefix your command line with solar_libpcap
 as follows:

   $ solar_libpcap tcpdump -i eth2


Monitoring
==========

 The internal state of a SolarCapture process can be monitored using
 the solar_capture_monitor tool.

 To get a list of running SolarCapture processes:

   $ solar_capture_monitor

 To monitor the state of a SolarCapture process:

   $ watch -d -n1 solar_capture_monitor <pid> dump

 Running solar_capture_monitor has very little impact on the performance of
 the capture process. Other solar_capture_monitor commands include
 "line_rate" and "line_total" to show packet rate and bandwidth.


libpcap
=======

 SolarCapturePro provides a modified version of libpcap, which can
 capture and inject packets using the SolarCapturePro architecture,
 bypassing the kernel.  This can be used in two ways, either by
 statically linking to the modified libpcap, or for existing
 applications that dynamically link against libpcap, a wrapper script
 is provided to enable the SolarCapturePro version of libpcap to be
 used.

 The wrapper script can be used as follows:
  $ solar_libpcap <application>


Software filtering
==================

 As well as filtering in hardware using the "streams=" option,
 SolarCapturePro adds support for filtering in software with filters
 specified in the BPF syntax.  Packets that have been captured will be
 matched against the specified filter, and those that do not match
 will be discarded.  The filter can be specified on the command line
 using the "filter=<bpf-filter-string>" option.  For example:

  $ solar_capture eth2=/tmp/pcap filter="src host 172.16.132.99"

 Software filtering is applied to all packets captured.  The captured
 packets can be limited by using the streams option in the normal way.
 For example to capture all unicast udp traffic for a specific host
 the following options could be used:

  $ solar_capture eth2=/tmp/pcap streams="eth:00:0F:53:01:7D:40" filter="udp"


Sniff mode 
==========

 Packets captured by SolarCapture are not available by default to
 the kernel stack, OpenOnload or any other process.

 On Flareon and AOE adapters SolarCapturePro can be run in sniff mode
 (add mode=sniff to the command line) whereby packets captured by
 SolarCapture will be replicated in hardware and so also be made
 available to other receivers.
 
 The behaviour of sniff mode can be modified using the promiscuous
 option.  If promiscuous is enabled ("promiscuous=1" which is the
 default) then all packets arriving at the sniffed interface will also
 be delivered to SolarCapturePro.  If promiscuous is not enabled
 ("promiscuous=0") then only packets that would anyway be delivered to
 the host will be delivered to SolarCapturePro.

 On other adapters is not possible to use SolarCapture to monitor
 streams that are consumed by applications on the same server.  We
 recommend using SolarCapture with mirror/span switch ports.


Hardware timestamps
===================

 On Flareon and AOE adapters SolarCapturePro will attempt to assign
 timestamps in hardware, but fall back to software if they are not
 available, e.g. due to a resource shortage or missing license.


Notes
=====

 - By default, packets captured by SolarCapture are not available to the
   kernel stack, OpenOnload or any other process.  On Flareon and AOE
   adapters you can use "mode=sniff" to capture a copy of packets arriving
   at the host so that applications can continue to communicate over the
   same interface.

 - Timestamps are assigned either by the adapter (Flareon adapters with
   SolarCapture Pro license and AOE adapters) or by the SolarCapture
   software.  Software timestamps are subject to system jitter caused by
   the OS kernel, BIOS and other processes.  To get timestamps that are as
   accurate as possible, SolarCapture should be run on isolated cores which
   are configured to minimise interruptions from system interrupts and
   processes.

 - Capture performance depends on many factors.  In most deployments the
   sustained capture rate is likely to be limited by storage performance.
   Other factors that affect capture rate include:

   * The I/O performance of the server.
   * The size of the internal packet buffer pool.
   * Spreading of load using receive-side scaling and application clustering.


