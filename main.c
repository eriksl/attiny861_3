#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usbdrv.h"
#include "oddebug.h"

static void calibrateOscillator(void)
{
	uint8_t		step = 128;
	uint8_t		trialValue = 0, optimumValue;
	uint16_t	x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
 
	/* do a binary search: */

	do
	{
		OSCCAL = trialValue + step;
		x = usbMeasureFrameLength();	// proportional to current real frequency
		if(x < targetValue)				// frequency still too low
			trialValue += step;
		step >>= 1;
	}
	while(step > 0);

	/* We have a precision of +/- 1 for optimum OSCCAL here */
	/* now do a neighborhood search for optimum value */

	optimumValue	= trialValue;
	optimumDev		= x; // this is certainly far away from optimum

	for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++)
	{
		x = usbMeasureFrameLength() - targetValue;

		if(x < 0)
			x = -x;
		if(x < optimumDev)
		{
			optimumDev = x;
			optimumValue = OSCCAL;
		}
	}

	OSCCAL = optimumValue;
}
 
void usbEventResetReady(void)
{
	cli(); // usbMeasureFrameLength() counts CPU cycles, so disable interrupts.
	calibrateOscillator();
	sei();
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	//usbRequest_t	*rq = (void *)data;
	return 0;		/* default for not implemented requests: return no data back to host */
}

int main(void)
{
	cli();

	PRR =		(0 << 7)		|
				(0 << 6)		|	// reserved
				(0 << 5)		|
				(0 << 4)		|
				(0 << PRTIM1)	|	// timer1
				(0 << PRTIM0)	|	// timer0
				(1 << PRUSI)	|	// usi
				(1 << PRADC);		// adc / analog comperator

	set_sleep_mode(SLEEP_MODE_IDLE);

	odDebugInit();
	DBG1(0x00, 0, 0);
	usbInit();
	usbDeviceDisconnect();
	_delay_ms(250);
	usbDeviceConnect();

	sei();

	DBG1(0x01, 0, 0);

	for(;;)
	{
		DBG1(0x02, 0, 0);
		usbPoll();
	}

	return(0);
}
