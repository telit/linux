/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BACKPORT_MHI_H
#define _BACKPORT_MHI_H

/*
 * Protect against 64-bit values tearing on 32-bit architectures. This is
 * typically used for statistics read/update in different subsystems.
 *
 * Key points :
 *
 * -  Use a seqcount on 32-bit
 * -  The whole thing is a no-op on 64-bit architectures.
 *
 * Usage constraints:
 *
 * 1) Write side must ensure mutual exclusion, or one seqcount update could
 *    be lost, thus blocking readers forever.
 *
 * 2) Write side must disable preemption, or a seqcount reader can preempt the
 *    writer and also spin forever.
 *
 * 3) Write side must use the _irqsave() variant if other writers, or a reader,
 *    can be invoked from an IRQ context. On 64bit systems this variant does not
 *    disable interrupts.
 *
 * 4) If reader fetches several counters, there is no guarantee the whole values
 *    are consistent w.r.t. each other (remember point #2: seqcounts are not
 *    used for 64bit architectures).
 *
 * 5) Readers are allowed to sleep or be preempted/interrupted: they perform
 *    pure reads.
 *
 * Usage :
 *
 * Stats producer (writer) should use following template granted it already got
 * an exclusive access to counters (a lock is already taken, or per cpu
 * data is used [in a non preemptable context])
 *
 *   spin_lock_bh(...) or other synchronization to get exclusive access
 *   ...
 *   u64_stats_update_begin(&stats->syncp);
 *   u64_stats_add(&stats->bytes64, len); // non atomic operation
 *   u64_stats_inc(&stats->packets64);    // non atomic operation
 *   u64_stats_update_end(&stats->syncp);
 *
 * While a consumer (reader) should use following template to get consistent
 * snapshot for each variable (but no guarantee on several ones)
 *
 * u64 tbytes, tpackets;
 * unsigned int start;
 *
 * do {
 *         start = u64_stats_fetch_begin(&stats->syncp);
 *         tbytes = u64_stats_read(&stats->bytes64); // non atomic operation
 *         tpackets = u64_stats_read(&stats->packets64); // non atomic operation
 * } while (u64_stats_fetch_retry(&stats->syncp, start));
 *
 *
 * Example of use in drivers/net/loopback.c, using per_cpu containers,
 * in BH disabled context.
 */
#if BITS_PER_LONG == 64
#include <asm/local64.h>

typedef struct {
	local64_t	v;
} u64_stats_t ;

static inline u64 u64_stats_read(const u64_stats_t *p)
{
	return local64_read(&p->v);
}

static inline void u64_stats_set(u64_stats_t *p, u64 val)
{
	local64_set(&p->v, val);
}

static inline void u64_stats_add(u64_stats_t *p, unsigned long val)
{
	local64_add(val, &p->v);
}

static inline void u64_stats_inc(u64_stats_t *p)
{
	local64_inc(&p->v);
}

#else /* 64 bit */

typedef struct {
	u64		v;
} u64_stats_t;

static inline u64 u64_stats_read(const u64_stats_t *p)
{
	return p->v;
}

static inline void u64_stats_set(u64_stats_t *p, u64 val)
{
	p->v = val;
}

static inline void u64_stats_add(u64_stats_t *p, unsigned long val)
{
	p->v += val;
}

static inline void u64_stats_inc(u64_stats_t *p)
{
	p->v++;
}
#endif /* !64 bit */

#endif /* _BACKPORT_MHI_H */
