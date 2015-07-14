/*
** This file is part of Solarflare SolarCapture.
**
** You may freely copy code from this sample to incorporate into your own
** code.
**
**
** Copyright 2012-2015  Solarflare Communications Inc.
**                      7505 Irvine Center Drive, Irvine, CA 92618, USA
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** * Redistributions of source code must retain the above copyright notice,
**   this list of conditions and the following disclaimer.
**
** * Redistributions in binary form must reproduce the above copyright
**   notice, this list of conditions and the following disclaimer in the
**   documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#define SC_API_VER 2
#include <solar_capture.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


#define TRY(x)                                                  \
  do {                                                          \
    int __rc = (x);                                             \
    if( __rc < 0 ) {                                            \
      fprintf(stderr, "ERROR: TRY(%s) failed\n", #x);           \
      fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__); \
      fprintf(stderr, "ERROR: rc=%d errno=%d (%s)\n",           \
              __rc, errno, strerror(errno));                    \
      exit(1);                                                  \
    }                                                           \
  } while( 0 )


#define TEST(x)                                                 \
  do {                                                          \
    if( ! (x) ) {                                               \
      fprintf(stderr, "ERROR: TEST(%s) failed\n", #x);          \
      fprintf(stderr, "ERROR: at %s:%d\n", __FILE__, __LINE__); \
      exit(1);                                                  \
    }                                                           \
  } while( 0 )


int main(int argc, char* argv[])
{
  if( argc != 2 ) {
    printf("Usage: %s <intf>\n", argv[0]);
    exit(1);
  }
  const char* intf = argv[1];

  struct sc_attr* attr;
  TRY(sc_attr_alloc(&attr));

  struct sc_attr* um_attr = sc_attr_dup(attr);
  TRY(sc_attr_set_int(um_attr, "managed", 0));

  struct sc_session* scs;
  TRY(sc_session_alloc(&scs, attr));

  /* Create a managed thread. */
  struct sc_thread* thrd;
  TRY(sc_thread_alloc(&thrd, attr, scs));

  /* An unmanaged thread.  This allows us to create SolarCapture objects
   * that will be used in a thread that we control.
   */
  struct sc_thread* um_thrd;
  TRY(sc_thread_alloc(&um_thrd, um_attr, scs));

  /* Capture all packets on [intf]. */
  struct sc_vi* vi;
  TRY(sc_vi_alloc(&vi, attr, thrd, intf));
  struct sc_stream* stream;
  TRY(sc_stream_alloc(&stream, attr, scs));
  TRY(sc_stream_set_str(stream, "all"));
  TRY(sc_vi_add_stream(vi, stream));

  /* Mailboxes are used to pass packets between threads. */
  struct sc_mailbox* mbox;
  struct sc_mailbox* um_mbox;
  TRY(sc_mailbox_alloc(&mbox, attr, thrd));
  TRY(sc_mailbox_alloc(&um_mbox, um_attr, um_thrd));
  TRY(sc_mailbox_connect(mbox, um_mbox));

  /* VI sends packets to the mailbox. */
  TRY(sc_vi_set_recv_node(vi, sc_mailbox_get_send_node(mbox), NULL));

  /* Tell SolarCapture that packets arriving at [um_mbox] will be returned
   * via the reverse path.
   */
  TRY(sc_mailbox_set_recv(um_mbox, sc_mailbox_get_send_node(um_mbox), NULL));

  /* Start managed threads. */
  sc_session_go(scs);

  while( 1 ) {
    /* Poll the mailbox.  As well as retrieving packets passed to us by the
     * managed thread, this ensures that packets sent with
     * sc_mailbox_send() get delivered.
     */
    struct sc_packet_list pl;
    sc_packet_list_init(&pl);
    if( sc_mailbox_poll(um_mbox, &pl) ) {
      struct sc_packet* next;
      struct sc_packet* pkt;
      for( next = pl.head; (pkt = next) && ((next = next->next), 1); ) {
        /* Do something useful with the packets! */
        printf("%lld.%09d\n", (unsigned long long) pkt->ts_sec, pkt->ts_nsec);
      }
      /* Free the packets back to the managed thread.  They'll make their
       * way back to the appropriate packet pool.
       */
      sc_mailbox_send_list(um_mbox, &pl);
    }
  }

  return 0;
}
