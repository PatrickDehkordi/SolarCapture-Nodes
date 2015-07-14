/*
** This file is part of Solarflare SolarCapture.
**
** Copyright 2012-2015  Solarflare Communications Inc.
**                      7505 Irvine Center Drive, Irvine, CA 92618, USA
**
** Proprietary and confidential.  All rights reserved.
**
** Please see LICENSE.txt included in this distribution for terms of use.
*/

#ifndef __SOLAR_CAPTURE_H__
#define __SOLAR_CAPTURE_H__


#include <solar_capture_ext.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <solar_capture/dlist.h>
#include <solar_capture/attr.h>
#include <solar_capture/misc.h>
#include <solar_capture/node.h>
#include <solar_capture/mailbox.h>
#include <solar_capture/session.h>
#include <solar_capture/thread.h>
#include <solar_capture/vi.h>
#include <solar_capture/args.h>
#include <solar_capture/stream.h>
#include <solar_capture/iovec.h>

#if SC_API_VER >= 1
#include <solar_capture/pkt_pool.h>
#include <solar_capture/event.h>
#include <solar_capture/time.h>
#include <solar_capture/predicate.h>
#endif

#ifdef __cplusplus
}
#endif


#endif  /* __SOLAR_CAPTURE_H__ */
