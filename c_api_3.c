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

/*
 * This file contains an example how to export content of packets
 * captured with SolarCapture C API for external processing.
 *
 * The example creates n threads and the incoming traffic is split
 * so that each of the threads receives subset of the traffic based on
 * source and destination IP address hash.
 *
 * The example utilizes SolarCapture custom node split_to_rings
 * to perform traffic exporting and splitting.
 */

#define SC_API_VER 2
#include <solar_capture.h>

#include "split_to_rings.h"
#include "pkt_ring.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
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


static void* worker_thread(void* arg)
{
  struct pkt_ring* pkt_ring = arg;
  void* payload;
  int len;

  while( 1 )
    if( pkt_ring_get(pkt_ring, &payload, &len) == 0 ) {
      /* Do something useful with packet contents here. */
      printf("%8p: processing %d bytes\n", arg, len);
    }

  return NULL;
}


int main(int argc, char* argv[])
{
  if( argc != 3 ) {
    printf("Usage: %s <intf> <n_consumers>\n", argv[0]);
    exit(1);
  }
  const char* interface = argv[1];
  int n_consumers = atoi(argv[2]);

  /* Create SolarCapture session. */
  struct sc_attr* attr;
  TRY(sc_attr_alloc(&attr));
  struct sc_session* scs;
  TRY(sc_session_alloc(&scs, attr));
  struct sc_thread* thrd;
  TRY(sc_thread_alloc(&thrd, attr, scs));

  /* Create VI to capture all packets on the interface. */
  struct sc_vi* vi;
  TRY(sc_vi_alloc(&vi, attr, thrd, interface));
  struct sc_stream* stream;
  TRY(sc_stream_alloc(&stream, attr, scs));
  TRY(sc_stream_all(stream));
  TRY(sc_vi_add_stream(vi, stream));

  /* Allocate packet rings and start worker threads. */
  struct pkt_ring* pkt_rings[n_consumers];
  int i;
  for( i = 0; i < n_consumers; ++i ) {
    struct pkt_ring* pkt_ring;
    TEST(pkt_ring_alloc(&pkt_ring, 32*1024*1024) == 0);
    pkt_rings[i] = pkt_ring;
    pthread_t thread_id;
    TEST(pthread_create(&thread_id, NULL, worker_thread, pkt_ring) == 0);
  }

  /* Allocate packet handler to distribute packets over the packet rings. */
  struct sc_object* pkt_rings_obj;
  /* Pack pointer to rings into solar capture object. */
  TRY(sc_opaque_alloc(&pkt_rings_obj, pkt_rings));
  struct sc_arg str_node_args[] = {
    /* Provide values of arguments the custom node requires. */
    SC_ARG_INT("n_pkt_rings", n_consumers),
    SC_ARG_OBJ("pkt_rings", pkt_rings_obj),
  };
  struct sc_node* str_node;
  /* The "split to rings" custom node is allocated with reference to its
   * factory object.
   */
  TRY(sc_node_alloc(&str_node, attr, thrd, &split_to_rings_sc_node_factory,
                          str_node_args,
                          sizeof(str_node_args) / sizeof(str_node_args[0])));
  TRY(sc_vi_set_recv_node(vi, str_node, NULL));

  sc_session_go(scs);
  while( 1 )
    sleep(100000);
  return 0;
}
[root@r710d c_api_export]#  ls
c_api_export.c  Makefile  pkt_ring.c  pkt_ring.h  README  split_to_rings.c  split_to_rings.h
[root@r710d c_api_export]# more c_api_export.c
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

/*
 * This file contains an example how to export content of packets
 * captured with SolarCapture C API for external processing.
 *
 * The example creates n threads and the incoming traffic is split
 * so that each of the threads receives subset of the traffic based on
 * source and destination IP address hash.
 *
 * The example utilizes SolarCapture custom node split_to_rings
 * to perform traffic exporting and splitting.
 */

#define SC_API_VER 2
#include <solar_capture.h>

#include "split_to_rings.h"
#include "pkt_ring.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
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


static void* worker_thread(void* arg)
{
  struct pkt_ring* pkt_ring = arg;
  void* payload;
  int len;

  while( 1 )
    if( pkt_ring_get(pkt_ring, &payload, &len) == 0 ) {
      /* Do something useful with packet contents here. */
      printf("%8p: processing %d bytes\n", arg, len);
    }

  return NULL;
}


int main(int argc, char* argv[])
{
  if( argc != 3 ) {
    printf("Usage: %s <intf> <n_consumers>\n", argv[0]);
    exit(1);
  }
  const char* interface = argv[1];
  int n_consumers = atoi(argv[2]);

  /* Create SolarCapture session. */
  struct sc_attr* attr;
  TRY(sc_attr_alloc(&attr));
  struct sc_session* scs;
  TRY(sc_session_alloc(&scs, attr));
  struct sc_thread* thrd;
  TRY(sc_thread_alloc(&thrd, attr, scs));

  /* Create VI to capture all packets on the interface. */
  struct sc_vi* vi;
  TRY(sc_vi_alloc(&vi, attr, thrd, interface));
  struct sc_stream* stream;
  TRY(sc_stream_alloc(&stream, attr, scs));
  TRY(sc_stream_all(stream));
  TRY(sc_vi_add_stream(vi, stream));

  /* Allocate packet rings and start worker threads. */
  struct pkt_ring* pkt_rings[n_consumers];
  int i;
  for( i = 0; i < n_consumers; ++i ) {
    struct pkt_ring* pkt_ring;
    TEST(pkt_ring_alloc(&pkt_ring, 32*1024*1024) == 0);
    pkt_rings[i] = pkt_ring;
    pthread_t thread_id;
    TEST(pthread_create(&thread_id, NULL, worker_thread, pkt_ring) == 0);
  }

  /* Allocate packet handler to distribute packets over the packet rings. */
  struct sc_object* pkt_rings_obj;
  /* Pack pointer to rings into solar capture object. */
  TRY(sc_opaque_alloc(&pkt_rings_obj, pkt_rings));
  struct sc_arg str_node_args[] = {
    /* Provide values of arguments the custom node requires. */
    SC_ARG_INT("n_pkt_rings", n_consumers),
    SC_ARG_OBJ("pkt_rings", pkt_rings_obj),
  };
  struct sc_node* str_node;
  /* The "split to rings" custom node is allocated with reference to its
   * factory object.
   */
  TRY(sc_node_alloc(&str_node, attr, thrd, &split_to_rings_sc_node_factory,
                          str_node_args,
                          sizeof(str_node_args) / sizeof(str_node_args[0])));
  TRY(sc_vi_set_recv_node(vi, str_node, NULL));

  sc_session_go(scs);
  while( 1 )
    sleep(100000);
  return 0;
}
