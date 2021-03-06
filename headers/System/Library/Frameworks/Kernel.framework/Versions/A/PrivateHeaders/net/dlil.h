/*
 * Copyright (c) 1999-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#ifndef DLIL_H
#define DLIL_H

#include <sys/kernel_types.h>
#include <net/kpi_interface.h>

enum {
	BPF_TAP_DISABLE,
	BPF_TAP_INPUT,
	BPF_TAP_OUTPUT,
	BPF_TAP_INPUT_OUTPUT
};

/* Ethernet specific types */
#define DLIL_DESC_ETYPE2	4
#define DLIL_DESC_SAP		5
#define DLIL_DESC_SNAP		6
/*
 * DLIL_DESC_ETYPE2 - native_type must point to 2 byte ethernet raw protocol,
 *                    variants.native_type_length must be set to 2
 * DLIL_DESC_SAP - native_type must point to 3 byte SAP protocol
 *                 variants.native_type_length must be set to 3
 * DLIL_DESC_SNAP - native_type must point to 5 byte SNAP protocol
 *                  variants.native_type_length must be set to 5
 *
 * All protocols must be in Network byte order.
 *
 * Future interface families may define more protocol types they know about.
 * The type implies the offset and context of the protocol data at native_type.
 * The length of the protocol data specified at native_type must be set in
 * variants.native_type_length.
 */


#include <net/if.h>
#include <net/if_var.h>
#include <sys/kern_event.h>
#include <kern/thread.h>
#include <kern/locks.h>

#if __STDC__

struct ifnet;
struct mbuf;
struct ether_header;
struct sockaddr_dl;

#endif

struct iff_filter;

#define	DLIL_THREADNAME_LEN	32

struct dlil_threading_info {
	decl_lck_mtx_data(, input_lck);
	lck_grp_t	*lck_grp;	/* lock group (for lock stats) */
	mbuf_t		mbuf_head;	/* start of mbuf list from if */
	mbuf_t		mbuf_tail;
	u_int32_t	mbuf_count;
	boolean_t	net_affinity;	/* affinity set is available */
	u_int32_t	input_waiting;	/* DLIL condition of thread */
	struct thread	*input_thread;	/* thread data for this input */
	struct thread	*workloop_thread; /* current workloop thread */
	u_int32_t	tag;		/* current affinity tag */
	char		input_name[DLIL_THREADNAME_LEN];
#if IFNET_INPUT_SANITY_CHK
	u_int32_t	input_wake_cnt;	/* number of times the thread was awaken with packets to process */
	u_long		input_mbuf_cnt;	/* total number of mbuf packets processed by this thread */
#endif
};

/*
 * The following are shared with kpi_protocol.c so that it may wakeup
 * the input thread to run through packets queued for protocol input.
*/
#define	DLIL_INPUT_RUNNING	0x80000000
#define	DLIL_INPUT_WAITING	0x40000000
#define	DLIL_PROTO_REGISTER	0x20000000
#define	DLIL_PROTO_WAITING	0x10000000
#define	DLIL_INPUT_TERMINATE	0x08000000

extern void dlil_init(void);

extern errno_t dlil_set_bpf_tap(ifnet_t, bpf_tap_mode, bpf_packet_func);

/*
 * Send arp internal bypasses the check for IPv4LL.
 */
extern errno_t dlil_send_arp_internal(ifnet_t, u_int16_t,
    const struct sockaddr_dl *, const struct sockaddr *,
    const struct sockaddr_dl *, const struct sockaddr *);

extern int dlil_output(ifnet_t, protocol_family_t, mbuf_t, void *,
    const struct sockaddr *, int);

extern void dlil_input_packet_list(struct ifnet *, struct mbuf *);

extern errno_t dlil_resolve_multi(struct ifnet *,
    const struct sockaddr *, struct sockaddr *, size_t);

extern errno_t dlil_send_arp(ifnet_t, u_int16_t, const struct sockaddr_dl *,
    const struct sockaddr *, const struct sockaddr_dl *,
    const struct sockaddr *);

extern int dlil_attach_filter(ifnet_t, const struct iff_filter *,
    interface_filter_t *);
extern void dlil_detach_filter(interface_filter_t);

extern void dlil_proto_unplumb_all(ifnet_t);

extern void dlil_post_msg(struct ifnet *, u_int32_t, u_int32_t,
    struct net_event_data *, u_int32_t);

/*
 * dlil_if_acquire is obsolete. Use ifnet_allocate.
 */
extern int dlil_if_acquire(u_int32_t, const void *, size_t, struct ifnet **);
/*
 * dlil_if_release is obsolete. The equivalent is called automatically when
 * an interface is detached.
 */
extern void dlil_if_release(struct ifnet *ifp);

extern u_int32_t ifnet_aggressive_drainers;

extern errno_t dlil_if_ref(struct ifnet *);
extern errno_t dlil_if_free(struct ifnet *);

#endif /* DLIL_H */
