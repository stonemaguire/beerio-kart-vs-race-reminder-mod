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

// Generic menu/action handler used by both race-start transitions we
// verified:
//
//     Race 1:  Page 0x3A -> 0x6E
//     Race 2+: Page 0x72 -> 0x6E
//
// Returning from this function at entry leaves the source page fully
// interactive, so this is the Beerio pre-race gate.
constexpr std::uintptr_t RaceStart_HandleAction =
    0x435A34;

// Native dialog request constructor.
constexpr std::uintptr_t UIDialogReq_Ctor =
    0x49B7E0;

// Configures:
// +0x0C dialog mode
// +0x10 secondary selector
// +0x18 message label
// +0x50 request flag
constexpr std::uintptr_t UIDialogReq_Configure =
    0x49B480;

// Native Page_Dialog open routine.
constexpr std::uintptr_t PageDialog_Open =
    0x49BA90;

// Page_Dialog per-frame/state update. The native cleanup path at
// 0x49BFAC executes inside this function. Beerio waits until this
// outer function has returned before replaying an accepted race start.
constexpr std::uintptr_t PageDialog_Update =
    0x49E5AC;

// Page_Dialog cleanup. At entry, the copied UIDialogReq and its result
// are still intact. Beerio captures the result here before Nintendo
// resets the request.
constexpr std::uintptr_t PageDialog_Cleanup =
    0x49BFAC;

// gear::GetUIPage(EUIPageID)
constexpr std::uintptr_t GetUIPage =
    0x8CF600;

} // namespace beerio::addr
