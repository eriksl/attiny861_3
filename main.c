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

	DDRA = 0xff;
	PORTA = 0x00;

	PRR =		(0 << 7)		|
				(0 << 6)		|	// reserved
				(0 << 5)		|
				(0 << 4)		|
				(0 << PRTIM1)	|	// timer1
				(0 << PRTIM0)	|	// timer0
				(1 << PRUSI)	|	// usi
				(1 << PRADC);		// adc / analog comperator

	PORTA |= _BV(0);

	set_sleep_mode(SLEEP_MODE_IDLE);

	PORTA |= _BV(1);

	odDebugInit();
	PORTA |= _BV(2);
	DBG1(0x00, 0, 0);
	usbInit();
	PORTA |= _BV(3);
	usbDeviceDisconnect();
	PORTA |= _BV(4);
	_delay_ms(250);
	PORTA |= _BV(5);
	usbDeviceConnect();
	PORTA |= _BV(6);

	sei();

	PORTA |= _BV(7);

	DBG1(0x01, 0, 0);

	for(;;)
	{
		DBG1(0x02, 0, 0);
		usbPoll();
	}

	return(0);
}
