/*
 * Copyright (c) 2000-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 * dhcp_thread.h
 * - functions implemented in dhcp.c
 * - common type definitions for DHCP-related functions
 */

#ifndef _S_DHCP_THREAD_H
#define _S_DHCP_THREAD_H

/* 
 * Modification History
 *
 * June 26, 2009		Dieter Siegmund (dieter@apple.com)
 * - split out from ipconfigd_threads.h
 */

#include "dhcp_options.h"

#define DHCPCLIENT_DIR			"/var/db/dhcpclient"

struct saved_pkt {
    dhcpol_t			options;
    uint8_t			pkt[1500];
    int				pkt_size;
    unsigned 			rating;
    boolean_t			is_dhcp;
    struct in_addr		our_ip;
    struct in_addr		server_ip;
};

void
dhcp_set_default_parameters(uint8_t * params, int n_params);

void
dhcp_set_additional_parameters(uint8_t * params, int n_params);

bool
dhcp_parameter_is_ok(uint8_t param);

void
dhcp_get_lease_from_options(dhcpol_t * options, dhcp_lease_time_t * lease, 
			    dhcp_lease_time_t * t1, dhcp_lease_time_t * t2);

struct in_addr *
dhcp_get_router_from_options(dhcpol_t * options_p, struct in_addr our_ip);

#endif /* _S_DHCP_THREAD_H */
