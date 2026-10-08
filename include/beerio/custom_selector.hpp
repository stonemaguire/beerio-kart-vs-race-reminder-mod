#pragma once

#include "beerio/game_types.hpp"

#include <cstdint>

namespace beerio::custom_selector {

struct CustomSelector
{
    game::OptionSelector base;

    // Used for generated value strings such as:
    //
    // "Every 32 Races"
    // "27th Race"
    //
    // 32 UTF-16 code units is comfortably larger than anything
    // required by the current 1..96 range.
    char16_t valueText[32];
};

// Clone the native Race Count selector vtable and replace only the
// virtual functions needed for the two custom integer selectors.
bool Initialize(game::OptionSelector* raceSelector);

game::OptionSelector* FrequencySelector();
game::OptionSelector* FrequencyStartSelector();

// Updates the maximum selectable value for both custom selectors.
// Values are 1..maximum.
void SetMaximum(std::uint32_t maximum);

std::uint32_t Frequency();
std::uint32_t FrequencyStart();

} // namespace beerio::custom_selector
