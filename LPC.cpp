// LPC processing (muting communication)
// KSTR 2019-11-04

#include "LPC.h"

#include "bit_manipulation.h"
#include "pin_manipulation.h"
#include "hardware.h"
#include "coos.h"
#include "audio.h"

static int8_t		LPC_Monitor_ID;
static uint8_t		mstate = 0xAA;
static uint8_t		old_mstate;
static uint8_t		high_time, low_time;
static uint8_t		signal_timeout;
static uint8_t		had_a_high_transition;
static uint8_t		active_cntr;
static uint8_t		inactive_cntr;
static uint8_t		unknown_cntr;
static uint16_t		default_timeout;

#define				PERIOD		(10)
#define				INACTIVE	(3)
#define				ACTIVE		(7)
#define				ENOUGH_PATTERNS	(3)

void LPC_Start_MonitorAudioEngineStatus(int32_t timeout)
{
	high_time = low_time = 0;
	signal_timeout = 10;
	mstate = 0xAA;
	had_a_high_transition = 0;
	active_cntr = 0;
	inactive_cntr = 0;
	unknown_cntr = 0;
	if (timeout > 0)
		default_timeout = timeout / MONENGINE_TIMESLICE;			// default timeout to unmute	
	else
		default_timeout = 0;
	LPC_Monitor_ID = coos_add_task( LPC_MonitorAudioEngineStatus_Process, 3, - MONENGINE_TIMESLICE );
}

void LPC_Stop_MonitorAudioEngineStatus(void)
{
	coos_delete_task(LPC_Monitor_ID);
}


void LPC_MonitorAudioEngineStatus_Process(void)
{
	if (default_timeout)
		default_timeout--;

	old_mstate = mstate;
	mstate = GetAudioEngineSignalPin();
	LED_A(mstate);
	if ( mstate && (high_time < 127) )
		high_time++;
	if ( !mstate && (low_time < 127) )
		low_time++;
		
	if (mstate != old_mstate)		// pin transitioned
	{
		signal_timeout = PERIOD+2;
		if (mstate)			// pin went high, so one cycle is over
		{
			if ( (high_time == ACTIVE-1 || high_time == ACTIVE || high_time == ACTIVE+1) \
			&&   (low_time == INACTIVE-1 || low_time == INACTIVE || low_time == INACTIVE+1) )
			{ // "active" pattern found
				inactive_cntr = 0;
				unknown_cntr = 0;
				if ( active_cntr<ENOUGH_PATTERNS )
					active_cntr++;
				else if (active_cntr == ENOUGH_PATTERNS)	// enough "active" patterns ?
				{
					Audio.UnMute();
					active_cntr++;			// block further unmutes
				}
			}
			else
			if ( (high_time == INACTIVE-1 || high_time == INACTIVE || high_time == INACTIVE+1 ) \
			&&   (low_time == ACTIVE-1 || low_time == ACTIVE || low_time == ACTIVE+1) )
			{ // "inactive" pattern found
				active_cntr = 0;
				unknown_cntr = 0;
				if ( inactive_cntr<ENOUGH_PATTERNS )
					inactive_cntr++;
				else if (inactive_cntr == ENOUGH_PATTERNS)	// enough "inactive" patterns ?
				{
					Audio.Mute();
					inactive_cntr++;			// block further mutes
				}
			}
			else // illegal pattern found
			{
				active_cntr = 0;
				inactive_cntr = 0;
				if ( unknown_cntr<ENOUGH_PATTERNS )
					unknown_cntr++;
			}

			high_time = 0;
			low_time = 0;
			
		}
	}
	
	if (signal_timeout)
		signal_timeout--;
	if (signal_timeout == 0)
	{
		signal_timeout = PERIOD+2;
		if ( unknown_cntr<ENOUGH_PATTERNS )
			unknown_cntr++;
	}
	
	if (default_timeout==0)		// timed out?
	{
	    if (unknown_cntr >= ENOUGH_PATTERNS)
		{
			Audio.UnMute();
			unknown_cntr++;
			active_cntr = 0;
			inactive_cntr = 0;
		}
	}


/*
		signal_timeout = PERIOD+2;
		if (!mstate)				// now a low ?
		{	// then check for a correct high time pattern
			if (  high_time == ACTIVE-1 || high_time == ACTIVE || high_time == ACTIVE+1 )		// long high means "active"
			{
				// while (high_time--) { LED_B(1), LED_B(0); };
				if ( active_cntr<2 )
					active_cntr++;
				else if (active_cntr == 2)	// enough "active" patterns ?
				{
					Audio.UnMute();
					active_cntr++;		// block further unmutes
					inactive_cntr = 0;		// arm for mute
					unknown_cntr = 0;
				}
			}
			else
			if ( high_time == INACTIVE-1 || high_time == INACTIVE || high_time == INACTIVE+1 )		// short high means "inactive"
			{
				// while (high_time--) { LED_B(1), LED_B(0); };
				if ( inactive_cntr<2 )
					inactive_cntr++;
				else if (inactive_cntr == 2)	// enough "inactive" patterns ?
				{
					Audio.Mute();
					inactive_cntr++;		// block further mutes
					active_cntr = 0;		// arm for unmute
					unknown_cntr = 0;
				}
			}
			else		// illegal pattern
			{
				if (had_a_high_transition)	// signal only when signal sequence was valid, that is low-->high-->low
					// LED_A(1);
					// LED_B(1); LED_B(0);
					if ( unknown_cntr<3 )
						unknown_cntr++;
			}
			high_time = 0;
		}
		else
		{
			if (mstate && (old_mstate==0))
				had_a_high_transition = 1;
		}
	}
	if (signal_timeout)
		signal_timeout--;
	if (signal_timeout == 0)
	{
		signal_timeout = PERIOD+2;
		had_a_high_transition=0;
		// LED_B(1); LED_B(0);
		if ( unknown_cntr<3 )
			unknown_cntr++;
	}
	
	if (default_timeout==0)		// timed out?
	{
	    if (unknown_cntr == 3)
		{
			Audio.UnMute();
			unknown_cntr++;
			active_cntr = 0;
			inactive_cntr = 0;
		}
	}
*/


}

// end of file
