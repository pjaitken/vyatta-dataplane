/*
 * Copyright (c) 2019-2021, AT&T Intellectual Property.  All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef _CGN_MAP_H_
#define _CGN_MAP_H_

#include <stdbool.h>
#include <stdint.h>
#include "vrf.h"
#include "npf/nat/nat_proto.h"

struct cgn_policy;
struct cgn_source;
struct nat_pool;
struct apm;

/*
 * Context that is required to create or release a CGNAT mapping.
 *
 * The subscriber pointer, cmi_src, is only valid if there is a current
 * mapping.  The mapping itself will have taken a reference on the subscriber.
 * Therefore if cmi_reserved is set and cmi_srci set then we know the cmi_src
 * pointer is valid.
 *
 * The 'reserved' flag indicates that this cmi structure is responsible for
 * the CGNAT mapping it contains.  It is only ever cleared when either:
 *
 *   1. The mapping is transferred to another cmi struct,
 *   2. The mapping is used to create a session, or
 *   3. The mapping is not used, and is released back to the apm pool
 *
 * Addresses and ports/IDs are in network order.
 */
struct cgn_map {
	uint8_t		cmi_reserved:1;	/* Contains a mapping? */
	enum nat_proto	cmi_proto;	/* Proto mapping space */
	uint16_t	cmi_oid;	/* Orig source port */
	uint16_t	cmi_tid;	/* Translation port */
	uint32_t	cmi_oaddr;	/* Orig source address */
	uint32_t	cmi_taddr;	/* Translation port */
	struct cgn_source *cmi_src;	/* Subscriber struct */
};

/*
 * Addresses are in network byte-order.  proto is of type enum npf_proto_idx.
 */
int cgn_map_get(struct cgn_map *cmi, struct cgn_policy *cp, vrfid_t vrfid);

/* Use the mapping given in taddr and tport.  Used by PCP. */
int cgn_map_get2(struct cgn_map *cmi, struct cgn_policy *cp, vrfid_t vrfid);

int cgn_map_put(struct cgn_map *cmi, vrfid_t vrfid);

void cgn_alloc_pool_available(struct nat_pool *np, struct apm *apm);

#endif
