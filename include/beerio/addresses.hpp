#pragma once

#include <cstdint>

namespace beerio::addr {

// Mario Kart 8 Deluxe 4.0.0
//
// Build ID:
// 2c336a9bcf79c3040ce506cdd391b57800000000000000000000000000000000
//
// All addresses below are relative to the start of TurboS.nss.

// Called after the native VS rows have been registered and immediately
// before Nintendo activates/layouts the list.
constexpr std::uintptr_t VsSettings_SetActiveListRange =
    0x453E28;

// Handles left/right input for the native Race Count row.
constexpr std::uintptr_t RaceCountRow_HandleInput =
    0x44E660;

// Refreshes the right-side value text for a settings row.
constexpr std::uintptr_t OptionRow_RefreshValueText =
    0x44F434;

// Known selector helpers. These are not hooked directly, but are useful
// documentation for the custom-selector ABI.
//
// Generic option-selection logic.
constexpr std::uintptr_t OptionSelector_ChangeSelection =
    0x44B294;

// Race Count label getter.
constexpr std::uintptr_t CountSelector_GetLabelText =
    0x44CC84;

// Race Count option -> text resolver.
constexpr std::uintptr_t RaceCount_OptionToText =
    0x44CCC8;

// Builds the VS settings rows for the selected mode.
constexpr std::uintptr_t VsSettings_ConfigureRows =
    0x463BE4;

} // namespace beerio::addr
