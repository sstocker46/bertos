/*!
 * \file
 * <!--
 * Copyright (C) 2000 Bernardo Innocenti
 * Copyright (C) 2003,2004 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \version $Id$
 *
 * \author Bernardo Innocenti <bernie@develer.com>
 *
 * \brief Low-level timer module for AVR
 */

/*
 * $Log$
 * Revision 1.1  2004/05/23 18:23:30  bernie
 * Import drv/timer module.
 *
 */

#ifndef TIMER_I196_H
#define TIMER_I196_H

	/*!
	 * Retrigger TIMER2, adjusting the time to account for
	 * the interrupt prologue latency.
	 */
#	define TIMER_RETRIGGER (TIMER2 -= TICKS_RATE)

#	define TIMER_INIT \
		TIMER2 = (65535 - TICKS_RATE); \
		INT_MASK1 |= INT1F_T2OVF; \
		\
		DISABLE_INTS; \
		WSR = 1; \
		IOC3 |= IOC3F_T2_ENA; \
		WSR = 0; \
		ENABLE_INTS

#define DEFINE_TIMER_ISR \
	INTERRUPT(0x38) void TM2_OVFL_interrupt(void);  \
	INTERRUPT(0x38) void TM2_OVFL_interrupt(void)

#endif /* DRV_TIMER_I196_H */
