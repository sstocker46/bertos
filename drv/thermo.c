/**
 * \file
 * <!--
 * Copyright 2005 Develer S.r.l. (http://www.develer.com/)
 * This file is part of DevLib - See README.devlib for information.
 * -->
 *
 * \brief Thermo-control driver
 *
 * \version $Id$
 *
 * \author Giovanni Bajo <rasky@develer.com>
 * \author Francesco Sacchi <batt@develer.com>
 */

/*#*
 *#* $Log$
 *#* Revision 1.2  2006/07/19 12:56:26  bernie
 *#* Convert to new Doxygen style.
 *#*
 *#* Revision 1.1  2005/11/04 17:59:47  bernie
 *#* Import into DevLib.
 *#*
 *#*/
#include <thermo_map.h>
#include <hw_thermo.h>

#include <drv/thermo.h>
#include <drv/timer.h>
#include <drv/ntc.h>

#include <cfg/macros.h>
#include <cfg/debug.h>


/** Interval at which thermo control is performed. */
#define THERMO_INTERVAL_MS      100

/** Number of different samples we interpolate over to get the hifi temperature. */
#define THERMO_HIFI_NUM_SAMPLES 10

/** Timer for thermo-regulation. */
static Timer thermo_timer;

typedef struct ThermoControlDev
{
	deg_t          hifi_samples[THERMO_HIFI_NUM_SAMPLES];
	deg_t          cur_hifi_sample;
	deg_t          target;
	thermostatus_t status;
	ticks_t        expire;
} ThermoControlDev;

/** Array of thermo-devices. */
ThermoControlDev devs[THERMO_CNT];


/**
 * Return the status of the specific \a dev thermo-device.
 */
thermostatus_t thermo_status(ThermoDev dev)
{
	return devs[dev].status;
}


/**
 * Do a single thermo control for device \a dev.
 */
static void thermo_do(ThermoDev index)
{
	ThermoControlDev* dev = &devs[index];
	deg_t cur_temp;
	deg_t tolerance = thermo_hw_tolerance(index);

	cur_temp = thermo_hw_read(index);

	// Store the sample into the hifi FIFO buffer for later interpolation
	dev->hifi_samples[dev->cur_hifi_sample] = cur_temp;
	if (++dev->cur_hifi_sample == THERMO_HIFI_NUM_SAMPLES)
		dev->cur_hifi_sample = 0;

	cur_temp = thermo_read_temperature(index);

	if (cur_temp == NTC_SHORT_CIRCUIT || cur_temp == NTC_OPEN_CIRCUIT)
	{
		if (cur_temp == NTC_SHORT_CIRCUIT)
		{
			#ifdef _DEBUG
			if (!(dev->status & THERMOERRF_NTCSHORT))
				kprintf("dev[%d], thermo_do: NTC_SHORT\n",index);
			#endif
			dev->status |= THERMOERRF_NTCSHORT;
		}
		else
		{
			#ifdef _DEBUG
			if (!(dev->status & THERMOERRF_NTCOPEN))
				kprintf("dev[%d], thermo_do: NTC_OPEN\n", index);
			#endif
			dev->status |= THERMOERRF_NTCOPEN;
		}

		/* Reset timeout when there is an ntc error */
		dev->expire = thermo_hw_timeout(index) + timer_clock();
		thermo_hw_off(index);
		return;
	}
	dev->status &= ~(THERMOERRF_NTCOPEN | THERMOERRF_NTCSHORT);

	if ((cur_temp < dev->target - tolerance) || (cur_temp > dev->target + tolerance))
	{
		dev->status &= ~THERMO_TGT_REACH;

		/* Check for timeout */
		if (timer_clock() - dev->expire > 0)
		{
			dev->status |= THERMOERRF_TIMEOUT;
			kprintf("dev[%d], thermo_do: TIMEOUT\n", index);
		}
	}
	else /* In target */
	{
		/* Clear errors */
		dev->status &= ~THERMO_ERRMASK;
		dev->status |= THERMO_TGT_REACH;

		/* Reset timeout in case we go out of target in the future */
		dev->expire = thermo_hw_timeout(index) + timer_clock();
	}

	if (cur_temp < dev->target)
		dev->status = (dev->status | THERMO_HEATING) & ~THERMO_FREEZING;
	else
		dev->status = (dev->status & ~THERMO_HEATING) | THERMO_FREEZING;

	thermo_hw_set(index, dev->target, cur_temp);

}


/**
 * Thermo soft interrupt.
 */
static void thermo_softint(void)
{
	int i;
	for (i = 0; i < THERMO_CNT; ++i)
		if (devs[i].status & THERMO_ACTIVE)
			thermo_do((ThermoDev)i);

	timer_add(&thermo_timer);
}


/**
 * Set the target temperature \a temperature for a specific \a dev thermo-device.
 */
void thermo_setTarget(ThermoDev dev, deg_t temperature)
{
	ASSERT(dev < THERMO_CNT);
	devs[dev].target = temperature;
	devs[dev].expire = timer_clock() + thermo_hw_timeout(dev);

	kprintf("setTarget dev[%d], T[%d.%d]\n", dev, temperature / 10, temperature % 10);
}

/**
 * Starts a thermo-regulation for channel \a dev.
 */
void thermo_start(ThermoDev dev)
{
	int i;
	deg_t temp;

	ASSERT(dev < THERMO_CNT);

	devs[dev].status |= THERMO_ACTIVE;

	/* Initialize the hifi FIFO with a constant value (the current temperature) */
	temp = thermo_hw_read(dev);
	for (i = 0; i < THERMO_HIFI_NUM_SAMPLES; ++i)
		devs[dev].hifi_samples[i] = temp;
	devs[dev].cur_hifi_sample = 0;

	/* Reset timeout */
	devs[dev].expire = timer_clock() + thermo_hw_timeout(dev);
}

/**
 * Stops a thermo-regulation for channel \a dev.
 */
void thermo_stop(ThermoDev dev)
{
	ASSERT(dev < THERMO_CNT);

	devs[dev].status &= ~THERMO_ACTIVE;
	thermo_hw_off(dev);
}


/**
 * Clear errors for channel \a dev.
 */
void thermo_clearErrors(ThermoDev dev)
{
	ASSERT(dev < THERMO_CNT);
	devs[dev].status &= ~(THERMO_ERRMASK);
}


/**
 * Read the temperature of the thermo-device \a dev using mobile mean.
 */
deg_t thermo_read_temperature(ThermoDev dev)
{
	int i;
	long accum = 0;

	for (i = 0; i < THERMO_HIFI_NUM_SAMPLES; i++)
		accum += devs[dev].hifi_samples[i];

	return (deg_t)(accum / THERMO_HIFI_NUM_SAMPLES);
}


/**
 * Init thermo-control and associated hw.
 */
void thermo_init(void)
{
	THERMO_HW_INIT;

	/* Set all status to off */
	for (int i = 0; i < THERMO_CNT; i++)
		devs[i].status = THERMO_OFF;

	timer_setDelay(&thermo_timer, ms_to_ticks(THERMO_INTERVAL_MS));
	timer_set_event_softint(&thermo_timer, (Hook)thermo_softint, 0);
	timer_add(&thermo_timer);
}
