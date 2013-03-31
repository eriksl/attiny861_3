#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "usbdrv.h"
#include "oddebug.h"

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
