#pragma once

#include <cstdint>

namespace beerio::reminder {

std::uint32_t Frequency();
std::uint32_t FrequencyStart();

// This is the rule that the later post-race hook will use.
//
// Example:
//
// Frequency Start = 3
// Frequency       = 4
//
// Reminder races:
// 3, 7, 11, 15, ...
bool ShouldShowReminder(
    std::uint32_t completedRaceNumber
);

} // namespace beerio::reminder
