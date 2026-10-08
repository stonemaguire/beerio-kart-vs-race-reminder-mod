#include "beerio/reminder_settings.hpp"

#include "beerio/custom_selector.hpp"

namespace beerio::reminder {

namespace {

bool gSeriesArmed = false;

std::uint32_t gUpcomingRaceNumber =
    1;

std::uint32_t gRaceLimit =
    0;

} // namespace

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

void ArmSeries()
{
    const auto* selector =
        custom_selector::FrequencySelector();

    // Both Beerio selectors use the selected Race Count
    // as their optionCount/max, so this also gives us the
    // current total race count for the series.
    gRaceLimit =
        selector != nullptr
            ? selector->optionCount
            : 0;

    gUpcomingRaceNumber =
        1;

    gSeriesArmed =
        gRaceLimit != 0;
}

void DisarmSeries()
{
    gSeriesArmed =
        false;

    gUpcomingRaceNumber =
        1;

    gRaceLimit =
        0;
}

bool SeriesArmed()
{
    return
        gSeriesArmed;
}

std::uint32_t UpcomingRaceNumber()
{
    if (!gSeriesArmed) {
        return 0;
    }

    if (gUpcomingRaceNumber == 0 ||
        gUpcomingRaceNumber > gRaceLimit) {
        return 0;
    }

    return
        gUpcomingRaceNumber;
}

void AdvanceUpcomingRaceNumber()
{
    if (!gSeriesArmed) {
        return;
    }

    if (gUpcomingRaceNumber >=
        gRaceLimit) {

        DisarmSeries();
        return;
    }

    ++gUpcomingRaceNumber;
}

bool ShouldShowReminder(
    std::uint32_t upcomingRaceNumber)
{
    const std::uint32_t start =
        FrequencyStart();

    const std::uint32_t frequency =
        Frequency();

    if (frequency == 0) {
        return false;
    }

    if (upcomingRaceNumber < start) {
        return false;
    }

    return
        ((upcomingRaceNumber - start) %
         frequency) == 0;
}

} // namespace beerio::reminder
