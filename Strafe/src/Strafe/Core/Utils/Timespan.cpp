// Copyright Epic Games, Inc. All Rights Reserved.

#include "Timespan.h"



uint32 GetTypeHash(const Timespan& Timespan)
{
	return GetTypeHash(Timespan.Ticks);
}


/* Timespan implementation
 *****************************************************************************/

void Timespan::Assign(int32 Days, int32 Hours, int32 Minutes, int32 Seconds, int32 FractionNano)
{
	int64 TotalTicks = 0;

	TotalTicks += Days * TimespanConstants::TicksPerDay;
	TotalTicks += Hours * TimespanConstants::TicksPerHour;
	TotalTicks += Minutes * TimespanConstants::TicksPerMinute;
	TotalTicks += Seconds * TimespanConstants::TicksPerSecond;
	TotalTicks += FractionNano / TimespanConstants::NanosecondsPerTick;

	/*check((TotalTicks >= TimespanConstants::MinTicks) && (TotalTicks <= TimespanConstants::MaxTicks));*/

	Ticks = TotalTicks;
}
