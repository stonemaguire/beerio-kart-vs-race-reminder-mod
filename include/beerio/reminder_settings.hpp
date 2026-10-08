#pragma once

#include <cstdint>

namespace beerio::reminder {

std::uint32_t Frequency();
std::uint32_t FrequencyStart();

// Arms a new local-VS series and resets the upcoming race number
// to race 1.
//
// The current Race Count maximum is captured at this point.
void ArmSeries();

void DisarmSeries();

bool SeriesArmed();

// Returns the race number that the next verified race-start action
// would launch.
//
// Returns 0 when there is no active Beerio local-VS series.
std::uint32_t UpcomingRaceNumber();

// Advances the series only after a race-start action has actually been
// released to Nintendo.
void AdvanceUpcomingRaceNumber();

// Example:
//
// Frequency Start = 3
// Frequency       = 4
//
// Reminder races:
//
// 3, 7, 11, 15, ...
bool ShouldShowReminder(
    std::uint32_t upcomingRaceNumber
);

} // namespace beerio::reminder
