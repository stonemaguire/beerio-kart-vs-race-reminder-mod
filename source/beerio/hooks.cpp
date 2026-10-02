#include "beerio/hooks.hpp"

#include "beerio/addresses.hpp"
#include "beerio/game_types.hpp"
#include "beerio/vs_settings.hpp"

#include "lib.hpp"

#include <cstdint>

namespace beerio::hooks {

namespace {

// Nintendo calls this after constructing/configuring the seven
// native local-VS rows.
//
// We intercept only the exact:
//
//     group = 1
//     count = 7
//
// local-VS configuration that we have reverse-engineered.
HOOK_DEFINE_TRAMPOLINE(
    VsSettingsSetActiveListRangeHook)
{
    static void Callback(
        void* rawMenu,
        std::uint32_t group,
        std::uint32_t count)
    {
        if (rawMenu != nullptr &&
            group == 1 &&
            count == 7) {

            game::VsSettingsMenuView menu(
                rawMenu
            );

            if (vs_settings::InjectCustomRows(menu)) {
                count = 9;
            }
        }

        Orig(
            rawMenu,
            group,
            count
        );
    }
};

// We observed this function handling native Race Count input.
//
// The exact function has additional event parameters that are opaque
// to the Beerio feature. They are preserved unchanged.
//
// Calling Orig() first lets Nintendo update the Race Count selector.
// We then derive the new race maximum and clamp the Beerio selectors.
HOOK_DEFINE_TRAMPOLINE(
    RaceCountRowHandleInputHook)
{
    static void Callback(
        std::uintptr_t param1,
        std::uintptr_t param2,
        std::uintptr_t param3,
        std::uintptr_t param4,
        std::uintptr_t param5,
        std::uintptr_t param6,
        std::uintptr_t param7,
        std::uintptr_t param8)
    {
        Orig(
            param1,
            param2,
            param3,
            param4,
            param5,
            param6,
            param7,
            param8
        );

        vs_settings::OnRaceCountRowEvent(
            reinterpret_cast<void*>(param1)
        );
    }
};

} // namespace

void Install()
{
    VsSettingsSetActiveListRangeHook::
        InstallAtOffset(
            addr::VsSettings_SetActiveListRange
        );

    RaceCountRowHandleInputHook::
        InstallAtOffset(
            addr::RaceCountRow_HandleInput
        );
}

} // namespace beerio::hooks
