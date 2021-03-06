/* FreeEMS - the open source engine management system
 *
 * Copyright 2010, 2011 Fred Cooke
 *
 * This file is part of the FreeEMS project.
 *
 * FreeEMS software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FreeEMS software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with any FreeEMS software.  If not, see http://www.gnu.org/licenses/
 *
 * We ask that if you make any changes to this file you email them upstream to
 * us at admin(at)diyefi(dot)org or, even better, fork the code on github.com!
 *
 * Thank you for choosing FreeEMS to run your engine!
 */


/**	@file decoderInterface.h
 * @ingroup allHeaders
 * @ingroup enginePositionRPMDecoders
 *
 * @brief Objects through which a decoder communicates.
 *
 * This file contains the declarations of objects used as a common interface
 * between all of the different wheel decoders and the main loop scheduler.
 *
 * @author Fred Cooke
 */


/* Header file multiple inclusion protection courtesy eclipse Header Template	*/
/* and http://gcc.gnu.org/onlinedocs/gcc-3.1.1/cpp/ C pre processor manual		*/
#ifndef FILE_DECODER_INTERFACE_H_SEEN
#define FILE_DECODER_INTERFACE_H_SEEN


#ifdef EXTERN
#warning "EXTERN already defined by another header, please sort it out!"
#undef EXTERN /* If fail on warning is off, remove the definition such that we can redefine correctly. */
#endif


#ifdef DECODER_IMPLEMENTATION_C
#define EXTERN
#else
#define EXTERN extern
#endif


#define BENCH_TEST_NAME "BenchTest"

// http://www.google.com/search?hl=en&biw=1283&bih=670&q=1250+*+60%29+*+50&aq=f&aqi=h1&aql=&oq=
// 1250 ticks per milli second
// 1250000 ticks per second
// 60 seconds per minute
// 360 degrees per revolution
// 10 ticks per degree multiplier
// 2 rpm scaler
// unused, already excluded 50 units per degree
// 1250000 * 60 = 75000000
// 75000000 / 360 = 208333.333333333333333
// 208333.333333333333333 * 2 * 10 = 4166666.666666666666667
#define degreeTicksPerMinute 4166667 // rounded up by 1/3
#define ticks_per_degree_multiplier (10 * oneDegree)
/// @todo TODO make this ^ scaling better x10 yields 64rpm minimum functional engine speed.
#define oneDegree 50U // Scaler for all scheduler and decoder angles, not tables etc. Suffix is necessary otherwise 8 bit is assumed. TODO Mount Messenger road to New Plymouth! Recommended Kim@bach with NAZZZ and Steve!


// ADC


// RPM - need some sort of state to say not to use these first time through...
EXTERN unsigned short ticksPerDegree0; // to be replaced with logging scheme for teeth.
EXTERN unsigned short ticksPerDegree1; // to be replaced with logging scheme for teeth.
EXTERN unsigned short* ticksPerDegree; // final output variable, probably move into inputVars struct?
EXTERN unsigned short* ticksPerDegreeRecord; // intermediate storage variable, do something with this?

// Time/Sync

/*
what is common between different setups? Add others here as required/desired/discussed.

		M-N setup - to sync we need gap that has just passed, and gap before, cases are: two gaps approx equal = who knows where. current gap = N * bigger means tooth after gap, current gap = N times smaller = second tooth after gap. Two opportunities to sync. by definition we always have these two gap measurements once synced. from these two gap measurements we can obtain a rough first RPM, then later RPM can be got from opposing teeth.
		4and1 setup - sync is obtained via logic, to get a rough RPM we need only one gap measurement, to verify noise free operation we need two. When we sync we may or may not have a previous gap measurement. For one sync opportunity we could have 0, 1, 2 gaps, for another we could have 0 or 1 gaps, and for the rest we will not have a gap measurement till a subsequent  event.
		24and2 setup - sync is obtained simply by watching for second trigger, sync is maintained by checking that second trigger falls when it should again. When a second trigger arrives, we could have 0, 1, 2 - ~12 primary gaps measured
		Simple setup - sync is not required, fire on all valid pulses. check for noise based on previous gaps, one event per ign event, correctly spaced timing required. Can be V twin, dizzy/points/outer of dsm/4and1 cas units, etc.

From this we need:

this stamp - have while running only. - so not required in header
previous stamp
stamp before previous stamp - only required on previous execution, at which time it is actually previous stamp, so not required.

this gap - have while running only. - so not required in header
previous gap - stored last time

flags to know if gap and stamp are valid - if gap is valid, stamp is not the oldest we effectively have.
previous gap valid - one flag
previous stamp valid - one flag

event counter - single unsigned value, 256 enough? probably can't cope with more than that anyway, except at low rpm, which is useless.

event angles - array of shorts, range required is 0 - 719.9999999999999 - can we provide sub degree accuracy with an unsigned short array? yes, what is best scale figure?

States:
* no sync (not of or of other three, no flag required, obviously)
* cylinder sync - one flag
* crank sync - one flag
* cam sync - one flag

from the above we can check one gap+angle with the next gap+angle and ensure smooth noise free operation is occurring.

*/

// unsigned long thisEventTimeStamp; recommended variable naming, may be enforced for/with macro use
// unsigned long thisInterEventPeriod; ditto
/// @todo TODO sync loss/gain semantics - how paranoid? under what circumstances? should we make it configurable whether a decoder that is in a situation where it would find sync if not synced, resets sync, or loses sync. Likewise, at initial sync gain time, should it go "prelim sync found" and only verify sync on the second lap around, or start firing events straight off the bat. Starting will suck if paranoid, but if there is noise at high load/rpm and events get mis-scheduled before sync is lost, that is serious. This is philosophical, and the reality is that you must assume that your signal is clean to some level and verified clean under lower risk conditions.
EXTERN unsigned char syncLostWithThisID;
EXTERN unsigned char syncLostOnThisEvent;
EXTERN unsigned char syncCaughtOnThisEvent;
EXTERN unsigned long lastEventTimeStamp;
EXTERN unsigned long lastPrimaryEventTimeStamp;
EXTERN unsigned long lastSecondaryEventTimeStamp;
EXTERN unsigned short lastTicksPerDegree;
EXTERN unsigned short lastPrimaryTicksPerDegree;
EXTERN unsigned short lastSecondaryTicksPerDegree;
EXTERN unsigned char currentEvent;
EXTERN unsigned char decoderFlags;
EXTERN unsigned long skipEventFlags;
EXTERN unsigned long engineCyclePeriod;
EXTERN unsigned char numberScheduled; /// @todo TODO remove DEBUG

/// @todo Introduce the concept of sync level to schedule for if NOT synced
/// @todo and a way of deciding what to do in different sync states
/// @todo and proper dividers for pulsewidths
/// @todo and ability to lock pulsewidht/dwell for scheduling
/// @todo and generalise scheduling to all pins
/// @todo and provide a way of choosing a source of pulsewidth dwell or fuel duration
/// @todo and a way of allowing overly advanced scheduling instead of none, when its fuel
#define COMBUSTION_SYNC      BIT0 // Dizzy/Batch Injection
#define CRANK_SYNC           BIT1 // Wasted Spark/Semi-Sequential
#define CAM_SYNC             BIT2 // COP/CNP/Sequential
#define LAST_TIMESTAMP_VALID BIT3
#define LAST_PERIOD_VALID    BIT4
#define CLEAR_COMBUSTION_SYNC      NBIT0
#define CLEAR_CRANK_SYNC           NBIT1
#define CLEAR_CAM_SYNC             NBIT2
#define CLEAR_LAST_TIMESTAMP_VALID NBIT3
#define CLEAR_LAST_PERIOD_VALID    NBIT4


#define ARBITRARY_DECODER_NAME_MAX_LENGTH 64
#define SIZE_OF_EVENT_ARRAYS 256
#if (SIZE_OF_EVENT_ARRAYS > 256)
#error "Event array size larger than variable used to index it!"
#endif // Is it paranoid to check myself like this? :-)


// These are defined per decoder and used elsewhere!
EXTERN const unsigned char decoderName[ARBITRARY_DECODER_NAME_MAX_LENGTH];
EXTERN const unsigned char numberOfRealEvents; // How many unique events the decoder sees.
EXTERN const unsigned char numberOfVirtualEvents; // How many of the members of the eventAngles array are valid. (multiples of real events (1 - 12))
EXTERN const unsigned short eventAngles[SIZE_OF_EVENT_ARRAYS]; /// @todo TODO From 0 - totalEventAngleRange degrees, scale: x50
EXTERN const unsigned char eventValidForCrankSync[SIZE_OF_EVENT_ARRAYS]; // For decoders with crank sync possible before cam sync, mark which events are eligble for crank scheduling here 0 = not valid, anything else = valid
EXTERN const unsigned short totalEventAngleRange;  // 720 for a four stroke, 360 for a two stroke, ? for a rotary. move this to code with a single setting for engine type and generate transformations based on that? All decoders will be 720 for now and only support 4 strokes without hackage.
EXTERN const unsigned short decoderMaxCodeTime; // The max of how long the primary and secondary ISRs take to run with worst case scheduling loop time!

#define SCHEDULE_ONE_ECT_OUTPUT() \
if(outputEventExtendNumberOfRepeats[outputEventNumber] > 0){                                                      \
	*injectorMainControlRegisters[pin] &= injectorMainDisableMasks[pin];                                          \
	outputEventExtendNumberOfRepeatsRealtime[pin] = outputEventExtendNumberOfRepeats[outputEventNumber];          \
	outputEventExtendNumberOfRepeatsRealtime[pin]--;                                                              \
	outputEventExtendRepeatPeriodRealtime[pin] = outputEventExtendRepeatPeriod[outputEventNumber];                \
	outputEventExtendFinalPeriodRealtime[pin] = outputEventExtendFinalPeriod[outputEventNumber];                  \
	*injectorMainTimeRegisters[pin] = timeStamp.timeShorts[1] + outputEventExtendRepeatPeriod[outputEventNumber]; \
	Counters.pinScheduledWithTimerExtension++;                                                                    \
}else{                                                                                                            \
	*injectorMainControlRegisters[pin] |= injectorMainEnableMasks[pin];                                           \
	*injectorMainTimeRegisters[pin] = startTime;                                                                  \
	Counters.pinScheduledToGoHigh++;                                                                              \
}                                                                                                                 \
TIE |= injectorMainOnMasks[pin];                                                                                  \
TFLG = injectorMainOnMasks[pin];                                                                                  \
injectorMainPulseWidthsRealtime[pin] = injectorMainPulseWidthsMath[outputEventNumber];                            \
selfSetTimer &= injectorMainOffMasks[pin];                                                                        // End of macro block!


#ifdef DECODER_IMPLEMENTATION_C // See above for information on how to set these values up.

/// @todo TODO behave differently depending upon sync level? Genericise this loop/logic? YES, move this to macro/function and call from all decoders.
#define SCHEDULE_ECT_OUTPUTS() \
numberScheduled = 0;                                                                        \
unsigned char outputEventNumber;                                                            \
for(outputEventNumber=0;outputEventNumber<MAX_NUMBER_OF_OUTPUT_EVENTS;outputEventNumber++){ \
	if(outputEventInputEventNumbers[outputEventNumber] == currentEvent){                    \
		skipEventFlags &= ~(1UL << outputEventNumber);                                      \
		schedulePortTPin(outputEventNumber, timeStamp);                                     \
		numberScheduled++;                                                                  \
	}else if(skipEventFlags & (1UL << outputEventNumber)){                                  \
		unsigned char eventBeforeCurrent = 0;                                               \
		if(currentEvent == 0){                                                              \
			eventBeforeCurrent = numberOfRealEvents - 1;                                    \
		}else{                                                                              \
			eventBeforeCurrent = currentEvent - 1;                                          \
		}                                                                                   \
                                                                                            \
		if(outputEventInputEventNumbers[outputEventNumber] == eventBeforeCurrent){          \
			schedulePortTPin(outputEventNumber, timeStamp);                                 \
			numberScheduled++;                                                              \
		}                                                                                   \
	}                                                                                       \
}                                                                                           // End of macro block!



// These give a warning in eclipse because they aren't defined in this file, they are defined per decoder and enforced here.
#ifndef DECODER_MAX_CODE_TIME
#error "Define your code max runtime conservatively at first, then optimise once the code is complete."
#endif
#ifndef NUMBER_OF_REAL_EVENTS
#error "Define how many unique events your decoder sees!"
#endif
#ifndef NUMBER_OF_VIRTUAL_EVENTS
#error "Define the length of the event array!"
#endif
#if ((NUMBER_OF_VIRTUAL_EVENTS % NUMBER_OF_REAL_EVENTS) != 0)
#error "Virtual events should be a multiple of real events!"
#endif

const unsigned char numberOfRealEvents = NUMBER_OF_REAL_EVENTS;
const unsigned char numberOfVirtualEvents = NUMBER_OF_VIRTUAL_EVENTS;
const unsigned short totalEventAngleRange = 720 * oneDegree; //TOTAL_EVENT_ANGLE_RANGE;
const unsigned short decoderMaxCodeTime = DECODER_MAX_CODE_TIME;

#endif


/// @todo TODO two unsigned chars, and two unsigned shorts, which is the MAP ADC value, the MAP value is sampled on every event in a cycle, and if less than the previous stored value, which is reset at every zeroth event, with the old value and old event number stored globally.
/// @todo TODO the same thing could be done, but with a median filter or similar, perhaps map sampling could be done dymanically like this, though it could yield unpredictable results, it could also yield the best running engines, just a thought...


// Scheduling

//// Config items: These must exist in flash only config, not here...
//EXTERN const unsigned char ADCSampleEvents[12];
//EXTERN const unsigned char numberOfOutputEvents;
//
//// Live vars for subprocess intercommunication
#define MAX_NUMBER_OF_OUTPUT_EVENTS 24
EXTERN unsigned char outputEventExtendNumberOfRepeats[MAX_NUMBER_OF_OUTPUT_EVENTS];
EXTERN unsigned short outputEventExtendRepeatPeriod[MAX_NUMBER_OF_OUTPUT_EVENTS];
EXTERN unsigned short outputEventExtendFinalPeriod[MAX_NUMBER_OF_OUTPUT_EVENTS];
EXTERN unsigned char outputEventExtendNumberOfRepeatsHolding[INJECTION_CHANNELS];
EXTERN unsigned short outputEventExtendRepeatPeriodHolding[INJECTION_CHANNELS];
EXTERN unsigned short outputEventExtendFinalPeriodHolding[INJECTION_CHANNELS];
EXTERN unsigned char outputEventExtendNumberOfRepeatsRealtime[INJECTION_CHANNELS];
EXTERN unsigned short outputEventExtendRepeatPeriodRealtime[INJECTION_CHANNELS];
EXTERN unsigned short outputEventExtendFinalPeriodRealtime[INJECTION_CHANNELS];
EXTERN unsigned char outputEventPinNumbers[MAX_NUMBER_OF_OUTPUT_EVENTS];            // 0xFF (disabled) by default, populated to actual pin numbers by the scheduler
EXTERN unsigned char outputEventInputEventNumbers[MAX_NUMBER_OF_OUTPUT_EVENTS];     // 0xFF (disabled) by default, populated to actual input event numbers by the scheduler
// see injectorMainPulseWidthsMath below, rename to this? EXTERN unsigned short outputEventDurations[MAX_NUMBER_OF_OUTPUT_EVENTS];            // Unused if above are not configured, set from either dwell (stretched or not) or pulsewidth (scaled for number of shots or not)
// older array immediately below used, rename to this? EXTERN unsigned short outputEventPostInputEventDelays[MAX_NUMBER_OF_OUTPUT_EVENTS]; // Unused if above are not configured, set either fixed or from angle calculations (always the latter for ignition)
EXTERN unsigned short postReferenceEventDelays[MAX_NUMBER_OF_OUTPUT_EVENTS];
// old, remove it... EXTERN unsigned char pinEventDurations[6];                 // Set from decoder when setting timer registers etc, set from outputEventDurations, along with other data from there.
/// @todo TODO back this ^ array with flags saying set, and then clear them when fired, check the flag before setting, and if required buffer in a secondary array, maybe mimic that to several levels such that a queue is formed, and shuffle them through the queue as we go, or move a pointer around or somthing like that.

/// @todo TODO Perhaps use some of the space freed by shrinking all timing tables for this:
////unsigned long wheelEventTimeStamps[numberOfWheelEvents]; // For logging wheel patterns as observed. LOTS of memory :-/ may not be possible except by sending lastStamp rapidly at low RPM



/* WAS injector stuff, now GP output stuff, and soon to be ign stuff, maybe, refactor all names... */

/* Register addresses */
EXTERN volatile unsigned short * volatile injectorMainTimeRegisters[INJECTION_CHANNELS];
EXTERN volatile unsigned char * volatile injectorMainControlRegisters[INJECTION_CHANNELS];

/* Timer holding vars (init not required) */
EXTERN unsigned short injectorMainStartOffsetHolding[INJECTION_CHANNELS];
EXTERN unsigned long injectorMainEndTimes[INJECTION_CHANNELS];

// TODO make these names consistent
/* Code time to run variables (init not required) */
EXTERN unsigned short injectorCodeOpenRuntimes[INJECTION_CHANNELS];
EXTERN unsigned short injectorCodeCloseRuntimes[INJECTION_CHANNELS];

/* individual channel pulsewidths (init not required) */
EXTERN unsigned short injectorMainPulseWidthsMath[MAX_NUMBER_OF_OUTPUT_EVENTS];
EXTERN unsigned short injectorMainPulseWidthsRealtime[INJECTION_CHANNELS];
EXTERN unsigned short injectorMainPulseWidthsHolding[INJECTION_CHANNELS];

/* Channel latencies (init not required) */
EXTERN unsigned short injectorCodeLatencies[INJECTION_CHANNELS];





// Helpers - force all these to be inlined!
EXTERN void decoderInitPreliminary(void);
EXTERN void perDecoderReset(void);
EXTERN void resetToNonRunningState(unsigned char uniqueLossID);
EXTERN void schedulePortTPin(unsigned char pin, LongTime timeStamp);
/** @todo TODO add shared function here that takes a long time stamp and stores
 * it in an array pointed to by a var with a flag saying "do it or not",
 * populate array entry, check pointer, set send flag, and unset record flag OR
 * increment pointer and return. Add call to this from all decoders. Add code
 * to interact with this in commsCore.c and/or main.c
 *
 * probably need to think it through a bit more to support both inputs at the
 * same time. Should also have 16 bits as an option for the purposes of
 * increased storage and not needing 32 bit resolution at higher revs. Better
 * to record stamps or diffs? Perhaps set the relative array sizes in a config
 * var such that if we expect 8 primary for every 2 secondary, then one array
 * is 4x as big as the other, and the population routine knows this. Think
 * about how to decode it later too in olv/mtx.
 */


/// @todo TODO add xgate scheduling functions here! Sean, looking forward to it, but after LT1 goes :-)


#undef EXTERN


/** @todo TODO IDEA: use a two stage mapping scheme for sched. Such that you
 * have event number to joiner number in an unsigned char array such that event
 * 4 and event 7 both are assigned join number 2, then pin X is scheduled to
 * run on join number 2 which could be on any number of real events!! GREAT!
 */



/**
 * RPM Calculations:
 *
 * Either need a per decoder function that is called from the main loop OR...
 * RPM calculation is done in real time inside one of the RPM interrupts OR...
 * The choice of either, up to the decoder to decide how it is done, in this
 * case the function can either do nothing, or swap some pointers/var values
 * around or similar.
 *
 * MAP Sampling:
 *
 * Max number of samples = max number of cylinders, has to be handled by
 * decoder due to potential mismatch between wheel pattern and cylinder firing
 * pattern unless it is done on a rough multiple sample basis in generic code
 * that runs often and can approximate timing/position for sampling.
 *
 * Scheduling:
 *
 * Fueling pins could be expected to fire once per cylinder event (1 - 12), or
 * once per engine cycle, or something in between, but what is a reasonable
 * maximum, and is it workable to allow some cases and not others?
 *
 *            |-----------------------------------------------------------|
 *            |                           Pins                            |
 *            |-----------------------------------------------------------|
 *            |  1 |  2 |  3 |  4 |  5 |  6 |  7 |  8 |  9 | 10 | 11 | 12 |
 * -----------|-----------------------------------------------------------|
 * |     |  1 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  2 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  3 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  4 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  5 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  6 |    |    |    |    |    |    |    |    |    |    |    |    |
 * | Cyl |----|-----------------------------------------------------------|
 * |     |  7 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  8 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     |  9 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     | 10 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     | 11 |    |    |    |    |    |    |    |    |    |    |    |    |
 * |     |----|-----------------------------------------------------------|
 * |     | 12 |    |    |    |    |    |    |    |    |    |    |    |    |
 * -----------|-----------------------------------------------------------|
 *
 *
 * Ignition pins will only need to be fired once per cycle (COP/CNP), twice per
 * cycle (Wasted Spark) or once per cylinder event (distributor) unless
 * multiple-spark startup is required, however this could be done with cascaded
 * dwell events, timer self-set:
 *
 * on dwell off spark on dwell off spark on dwell off and disable spark
 *
 * This example is for triple spark, 2 or more than 3 are also possible.
 *
 * When coils are wired one per cylinder for COP/CNP, during starting or loss
 * of cam sync two opposing coils will be fired at the same time in pseudo
 * wasted spark mode.
 *
 * Possible states of sync are as follows:
 *
 * Full cycle engine sync - COP/CNP/Sequential
 * Half cycle revolution sync - Wasted Spark/Semi Sequential
 * Cylinder sync - Distributor/Many pulses or un-timed batch pulses
 *
 * Wheels will have various patterns of rising/falling edges. Scheduling may be
 * done from either the rising or falling edge on some, or only on one edge of
 * others. VR sensors have only one reliable edge, the other varies with speed
 * and associated rise/fall times of the approximately sinusoidal wave form.
 * Simple patterns shall be required to be timed to the engine such that at low
 * engine speeds the timer delays available with high accuracy are sufficient
 * to properly time ignition events. At the least, there should be a wheel
 * event per cylinder event, and in close proximity to that cylinder event.
 * Allowing even more relaxed wheel patterns would mean compromising the
 * performance for other more common setups and/or increasing code complexity
 * by an unacceptable amount.
 *
 * OLD notes:
 *
 * arrays of output channels, iterate and check for wheel event number, if matched, fire:
 * doesn't allow for firing a pin more than once a cycle!! no use.
 * allows multi channel on a single wheel event (virtually useless) but is slow, one loop
 * and array per type of output channel.
 *
 * array of wheel events, do lookup for wheel event, then if output channel valid, schedule it.
 * single channel per tooth event (acceptable, wire in parallel if required), fast, memory hog,
 * need one array per type of channel, array length = max expected tooth count! do we need to
 * support every single tooth on a Nissan 360 style decoder or just cyl event count, what about
 * Porsche 130? next stop is 60, seems good. can we use bit-fields to save memory, 1 bit =
 * 2 possible pins, 2 bits = 4, etc, this will be slower, though. probably just an unsigned char[]
 */


// Init routine:
//
// Allow configuration of timer details? tick size? If so, need to introduce scaling to calcs to
// produce correct tick count and therefore pulsewidth. Migrate dead time to time units and scale
// to get ticks to add to final pw.
//
// We require some configuration to allow the Nissan style decoder to use the pulse accumulators to
// count those one degree slots accurately to a high rpm without excessive cpu load.


// move the following to  fuel calcs or sched code header, it doesn't belong here...
//
// stuff to do with timing and sync etc. ie, figuring out upon which



#else
	/* let us know if we are being untidy with headers */
	#warning "Header file DECODER_INTERFACE_H seen before, sort it out!"
/* end of the wrapper ifdef from the very top */
#endif
