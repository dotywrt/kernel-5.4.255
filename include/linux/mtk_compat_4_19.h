/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_MTK_COMPAT_4_19_H
#define _LINUX_MTK_COMPAT_4_19_H

/*
 * Compatibility helpers for legacy MediaTek 4.19 vendor drivers carried
 * on Linux 5.4.255. Keep conversions here instead of reintroducing removed
 * kernel APIs globally.
 */

#include <linux/time.h>
#include <linux/timekeeping.h>
#include <linux/types.h>
#include <clocksource/arm_arch_timer.h>

static inline void mtk_compat_do_gettimeofday(struct timeval *tv)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}

static inline void
mtk_compat_get_monotonic_boottime(struct timespec *ts)
{
	struct timespec64 ts64;

	ktime_get_boottime_ts64(&ts64);
	ts->tv_sec = ts64.tv_sec;
	ts->tv_nsec = ts64.tv_nsec;
}

static inline u64 mtk_compat_arch_counter_get_cntvct(void)
{
	return arch_timer_read_counter();
}

#endif /* _LINUX_MTK_COMPAT_4_19_H */
