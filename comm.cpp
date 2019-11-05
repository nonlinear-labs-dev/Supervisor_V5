// BBB communication
// KSTR 2019-11-04

#include "comm.h"

#include "bit_manipulation.h"
#include "pin_manipulation.h"
#include "hardware.h"



static uint8_t	data;

static uint8_t	step=0;

void COMM_Proccess(void)
{
	if ( BitGet(UCSRA, RXC) )		// receive register full ?
	{
		data = UDR;
		switch (data)
		{
			case 'L' :
				if ( config.GetRunState() == RS_RUNNING )
					BitSet(config.status, STAT_SYSTEM_LOCKED);
			break;
			case 'l' :
				BitClr(config.status, STAT_SYSTEM_LOCKED);
			break;
			
			case 'O' :
				BitSet(config.status, STAT_MUTING_OVERRIDE_ENABLE);
			break;
			case 'o' :
				BitClr(config.status, STAT_MUTING_OVERRIDE_ENABLE);
			break;
			
			case 'U' :
				BitSet(config.status, STAT_MUTING_OVERRIDE_VALUE);
			break;
			case 'u' :
				BitClr(config.status, STAT_MUTING_OVERRIDE_VALUE);
			break;
		}
	}

	if ( BitGet(UCSRA, UDRE) )		// send register empty ?
	{
		switch (step)
		{
			case 0 :	UDR = 0x80 + config.status; step++; break;
			case 1 :	UDR = config.hardware_id; step++; break;
			case 2 :	UDR = config.firmware_id; step=0; break;
		}
		
	}
}

// end of file
