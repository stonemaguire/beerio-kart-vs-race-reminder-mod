#include "beerio/reminder_settings.hpp"

#include "beerio/custom_selector.hpp"

namespace beerio::reminder {

std::uint32_t Frequency()
{
    return
        custom_selector::Frequency();
}

std::uint32_t FrequencyStart()
{
    return
        custom_selector::FrequencyStart();
}

bool ShouldShowReminder(
    std::uint32_t completedRaceNumber)
{
    const std::uint32_t start =
        FrequencyStart();

    const std::uint32_t frequency =
        Frequency();

    if (frequency == 0) {
        return false;
    }

    if (completedRaceNumber < start) {
        return false;
    }

    return
        ((completedRaceNumber - start) %
         frequency) == 0;
}

} // namespace beerio::reminder
