#include "beerio/hooks.hpp"

#include "beerio/addresses.hpp"
#include "beerio/game_types.hpp"
#include "beerio/reminder_dialog.hpp"
#include "beerio/reminder_settings.hpp"
#include "beerio/vs_settings.hpp"

#include "lib.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace beerio::hooks {

namespace {

constexpr std::ptrdiff_t kRaceStartPageOffset =
    0x50;

constexpr std::ptrdiff_t kRaceStartFlowOffset =
    0x158;

constexpr std::ptrdiff_t kPageIdOffset =
    0xE8;

constexpr std::ptrdiff_t kFlowTargetPageIdOffset =
    0x0C;

constexpr std::uint32_t kFirstRaceSourcePageId =
    0x3A;

constexpr std::uint32_t kNextRaceSourcePageId =
    0x72;

constexpr std::uint32_t kRaceIntroTargetPageId =
    0x6E;

// The verified race-start handler and its helper only consume these
// three fields from the input/event object.
constexpr std::size_t kSyntheticEventSize =
    0x38;

constexpr std::ptrdiff_t kEventValue2COffset =
    0x2C;

constexpr std::ptrdiff_t kEventValue30Offset =
    0x30;

constexpr std::ptrdiff_t kEventValue34Offset =
    0x34;

constexpr std::uint32_t kExpectedEventValue2C =
    0;

constexpr std::uint32_t kExpectedEventValue30 =
    1;

constexpr std::uint8_t kExpectedEventValue34 =
    0;

struct PendingRaceStart
{
    void* self = nullptr;
    std::uint32_t sourcePageId = 0;
    std::uint32_t raceNumber = 0;
    bool active = false;
};

PendingRaceStart gPendingRaceStart{};

bool gDialogCompletionPending = false;

reminder_dialog::Result gDialogCompletionResult =
    reminder_dialog::Result::None;

template <typename T>
T& Field(
    void* object,
    std::ptrdiff_t offset)
{
    return *reinterpret_cast<T*>(
        reinterpret_cast<std::uint8_t*>(object) +
        offset
    );
}

template <typename T>
const T& Field(
    const void* object,
    std::ptrdiff_t offset)
{
    return *reinterpret_cast<const T*>(
        reinterpret_cast<const std::uint8_t*>(object) +
        offset
    );
}

bool IsVerifiedSourcePage(
    std::uint32_t pageId)
{
    return
        pageId == kFirstRaceSourcePageId ||
        pageId == kNextRaceSourcePageId;
}

bool IsVerifiedRaceStartAction(
    void* self,
    const void* event,
    std::uint32_t& sourcePageId)
{
    sourcePageId = 0;

    if (self == nullptr ||
        event == nullptr) {
        return false;
    }

    void* page =
        Field<void*>(
            self,
            kRaceStartPageOffset
        );

    void* flow =
        Field<void*>(
            self,
            kRaceStartFlowOffset
        );

    if (page == nullptr ||
        flow == nullptr) {
        return false;
    }

    const std::uint32_t pageId =
        Field<std::uint32_t>(
            page,
            kPageIdOffset
        );

    const std::uint32_t flowTargetPageId =
        Field<std::uint32_t>(
            flow,
            kFlowTargetPageIdOffset
        );

    // flow+0x08 is not a stable discriminator here. The verified
    // UIPage::doFlow helper at 0x8CAB10 writes the source page ID
    // into flow+0x08 later in the transition, so at this handler
    // entry it can still be 0xFFFFFFFF. Filter on the owning page,
    // destination, and verified event shape instead.
    if (!IsVerifiedSourcePage(pageId) ||
        flowTargetPageId !=
            kRaceIntroTargetPageId) {
        return false;
    }

    // We observed exactly the same event values for Race 1 and
    // Next Race. Keep them as an additional guard so this generic
    // handler is never intercepted for a different action.
    if (Field<std::uint32_t>(
            event,
            kEventValue2COffset) !=
            kExpectedEventValue2C ||
        Field<std::uint32_t>(
            event,
            kEventValue30Offset) !=
            kExpectedEventValue30 ||
        Field<std::uint8_t>(
            event,
            kEventValue34Offset) !=
            kExpectedEventValue34) {
        return false;
    }

    sourcePageId =
        pageId;

    return true;
}

void BuildSyntheticRaceStartEvent(
    std::uint8_t (&event)[kSyntheticEventSize])
{
    std::memset(
        event,
        0,
        sizeof(event)
    );

    Field<std::uint32_t>(
        event,
        kEventValue2COffset
    ) = kExpectedEventValue2C;

    Field<std::uint32_t>(
        event,
        kEventValue30Offset
    ) = kExpectedEventValue30;

    Field<std::uint8_t>(
        event,
        kEventValue34Offset
    ) = kExpectedEventValue34;
}

// Forward declaration so the Page_Dialog update hook can replay the
// suppressed race-start action through the trampoline's original function.
struct RaceStartHandleActionHook;

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

            if (vs_settings::InjectCustomRows(
                    menu)) {

                count =
                    9;

                // Entering the Beerio local-VS settings screen
                // marks the beginning of a new series.
                reminder::ArmSeries();

                // Any stale pending action belongs to an older UI
                // flow and must never be replayed into a new series.
                gPendingRaceStart = {};
                gDialogCompletionPending = false;
                gDialogCompletionResult =
                    reminder_dialog::Result::None;
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
// Calling Orig() first lets Nintendo update the Race Count selector.
// We then derive the new maximum and clamp the Beerio selectors.
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

        // If Race Count changed while still in the pre-series
        // settings screen, capture the new race limit.
        reminder::ArmSeries();
    }
};

// This is the generic handler we verified for both:
//
//     Race 1:  Page 0x3A -> 0x6E
//     Race 2+: Page 0x72 -> 0x6E
//
// Returning without calling Orig() at this exact entry point leaves
// the source page fully interactive. That lets the reminder act as a
// true gate rather than interrupting a half-completed transition.
HOOK_DEFINE_TRAMPOLINE(
    RaceStartHandleActionHook)
{
    static void Callback(
        void* self,
        void* event)
    {
        std::uint32_t sourcePageId =
            0;

        if (!IsVerifiedRaceStartAction(
                self,
                event,
                sourcePageId)) {

            Orig(
                self,
                event
            );
            return;
        }

        const std::uint32_t raceNumber =
            reminder::UpcomingRaceNumber();

        // If this is not an armed Beerio series, preserve Nintendo's
        // behavior exactly.
        if (raceNumber == 0) {
            Orig(
                self,
                event
            );
            return;
        }

        // A Beerio dialog is already gating this exact race-start
        // action. Never allow a second input path to bypass it.
        if (gPendingRaceStart.active) {
            return;
        }

        if (!reminder::ShouldShowReminder(
                raceNumber)) {

            Orig(
                self,
                event
            );

            reminder::AdvanceUpcomingRaceNumber();
            return;
        }

        gPendingRaceStart.self =
            self;

        gPendingRaceStart.sourcePageId =
            sourcePageId;

        gPendingRaceStart.raceNumber =
            raceNumber;

        gPendingRaceStart.active =
            true;

        const bool opened =
            reminder_dialog::Show(
                sourcePageId
            );

        if (opened) {
            Logging.Log(
                "Beerio reminder opened before race %u (page 0x%X).",
                raceNumber,
                sourcePageId
            );

            // Deliberately suppress Orig(). We proved in GDB that
            // returning here leaves the source menu fully usable.
            return;
        }

        // Failing open is safer than trapping the player on the
        // source page. Let Nintendo start the race normally.
        Logging.Log(
            "Beerio reminder failed before race %u; releasing race start.",
            raceNumber
        );

        gPendingRaceStart = {};

        Orig(
            self,
            event
        );

        reminder::AdvanceUpcomingRaceNumber();
    }
};

// At Page_Dialog cleanup entry, Nintendo has already written the
// verified result field but has not yet reset the copied UIDialogReq:
//
//     dialog + 0x224 == 1  -> Yes
//     dialog + 0x224 == 2  -> No
//
// Capture only the result here. The cleanup function itself runs inside
// PageDialog_Update, so the suppressed race-start action is deliberately
// NOT replayed from this nested cleanup call.
HOOK_DEFINE_TRAMPOLINE(
    PageDialogCleanupHook)
{
    static void Callback(
        void* dialog)
    {
        if (gPendingRaceStart.active &&
            reminder_dialog::IsBeerioDialog(
                dialog)) {

            gDialogCompletionResult =
                reminder_dialog::GetResult(
                    dialog
                );

            gDialogCompletionPending =
                true;
        }

        Orig(
            dialog
        );
    }
};

// 0x49BFAC is called from inside this Page_Dialog state/update function.
// Waiting until Orig() returns here guarantees Nintendo has finished the
// complete dialog-close state transition before Beerio replays the
// suppressed race-start handler.
HOOK_DEFINE_TRAMPOLINE(
    PageDialogUpdateHook)
{
    static void Callback(
        void* dialog)
    {
        Orig(
            dialog
        );

        if (!gDialogCompletionPending) {
            return;
        }

        const PendingRaceStart pending =
            gPendingRaceStart;

        const reminder_dialog::Result dialogResult =
            gDialogCompletionResult;

        gDialogCompletionPending =
            false;

        gDialogCompletionResult =
            reminder_dialog::Result::None;

        gPendingRaceStart = {};

        if (!pending.active) {
            return;
        }

        if (dialogResult ==
            reminder_dialog::Result::No) {

            Logging.Log(
                "Beerio reminder declined before race %u.",
                pending.raceNumber
            );

            // Do not advance the race number. The source page remains
            // interactive, so the next start attempt prompts again.
            return;
        }

        if (dialogResult !=
            reminder_dialog::Result::Yes) {

            Logging.Log(
                "Beerio reminder closed with unknown result before race %u.",
                pending.raceNumber
            );

            // Keep the race number unchanged so the reminder is still
            // required on the next attempt.
            return;
        }

        alignas(8)
        std::uint8_t event[kSyntheticEventSize]{};

        BuildSyntheticRaceStartEvent(
            event
        );

        Logging.Log(
            "Beerio reminder accepted before race %u; releasing race start.",
            pending.raceNumber
        );

        // Advance exactly once, immediately before releasing the
        // suppressed native action.
        reminder::AdvanceUpcomingRaceNumber();

        RaceStartHandleActionHook::Orig(
            pending.self,
            event
        );
    }
};

} // namespace

void Install()
{
    VsSettingsSetActiveListRangeHook::
        InstallAtOffset(
            addr::
                VsSettings_SetActiveListRange
        );

    RaceCountRowHandleInputHook::
        InstallAtOffset(
            addr::
                RaceCountRow_HandleInput
        );

    RaceStartHandleActionHook::
        InstallAtOffset(
            addr::
                RaceStart_HandleAction
        );

    PageDialogCleanupHook::
        InstallAtOffset(
            addr::
                PageDialog_Cleanup
        );

    PageDialogUpdateHook::
        InstallAtOffset(
            addr::
                PageDialog_Update
        );
}

} // namespace beerio::hooks
