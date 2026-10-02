#pragma once

#include "beerio/game_types.hpp"

namespace beerio::vs_settings {

// Checks that this is the exact native 7-row local VS layout we
// reverse-engineered before modifying anything.
bool ShouldInject(game::VsSettingsMenuView& menu);

// Adds Frequency and Frequency Start as native-style rows 7 and 8.
bool InjectCustomRows(game::VsSettingsMenuView& menu);

// Reads the live Race Count selector and updates the allowed range of
// both custom selectors.
void UpdateLimitsFromRaceCount();

// Refresh the visible numeric strings for the two custom rows.
void RefreshCustomValues();

// Called after Nintendo handles input for the native Race Count row.
void OnRaceCountRowEvent(void* row);

void* ActiveMenu();
void* RaceCountRow();
void* FrequencyRow();
void* FrequencyStartRow();

} // namespace beerio::vs_settings
