// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include <linux/delay.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>

#include "proslic_sys.h"

/*****************************************************************************************************/

/* DOTY-PROSLIC-LINUX54-TIMER-COMPAT-BEGIN */
/*
 * Vendor ProSLIC used proslic_current_kernel_time() + proslic_timespec_sub().
 * Keep proslic_timeStamp as struct timespec and convert only here.
 */
static struct timespec proslic_current_kernel_time(void)
{
	struct timespec64 now64;
	struct timespec now;

	ktime_get_real_ts64(&now64);

	now.tv_sec = now64.tv_sec;
	now.tv_nsec = now64.tv_nsec;

	return now;
}

static struct timespec proslic_timespec_sub(struct timespec lhs,
					   struct timespec rhs)
{
	struct timespec delta;

	delta.tv_sec = lhs.tv_sec - rhs.tv_sec;
	delta.tv_nsec = lhs.tv_nsec - rhs.tv_nsec;

	if (delta.tv_nsec < 0) {
		delta.tv_sec--;
		delta.tv_nsec += 1000000000L;
	}

	return delta;
}
/* DOTY-PROSLIC-LINUX54-TIMER-COMPAT-END */

static int proslic_sys_delay(void *hTimer, int timeInMsec)
{
	if(likely(timeInMsec < SILABS_MIN_MSLEEP_TIME))
	{
		mdelay(timeInMsec);
	}
	else
	{
		msleep((timeInMsec-SILABS_MSLEEP_SLOP));
	}
	return PROSLIC_SPI_OK;
}

/*****************************************************************************************************/
/* Code assumes time value has been allocated */
static int proslic_sys_getTime(void *hTimer, void *time)
{
	if(time != NULL)
	{
		((proslic_timeStamp *)time)->timerObj = proslic_current_kernel_time();
		return PROSLIC_SPI_OK;
	}
	else
	{
		return PROSLIC_TIMER_ERROR;
	}
}
/*****************************************************************************************************/
 
static int proslic_sys_timeElapsed(void *hTimer, void *startTime, int *timeInMsec)
{
	if( (startTime != NULL) && (timeInMsec != NULL) )
	{
		struct timespec now = proslic_current_kernel_time();
		struct timespec ts_delta = proslic_timespec_sub(now, ((proslic_timeStamp *) startTime)->timerObj);
		*timeInMsec = ( (ts_delta.tv_sec *1000) + (ts_delta.tv_nsec / NSEC_PER_MSEC) );
		return PROSLIC_SPI_OK;
	}
	else
	{
		return PROSLIC_TIMER_ERROR;
	}
}


proslic_timer_fptrs_t proslic_timer_if =
{
	proslic_sys_delay,
	proslic_sys_timeElapsed,
	proslic_sys_getTime
};
