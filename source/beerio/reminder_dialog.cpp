#include "beerio/reminder_dialog.hpp"

#include "beerio/addresses.hpp"

#include "lib.hpp"

#include <cstddef>
#include <cstdint>

namespace beerio::reminder_dialog {

namespace {

constexpr std::uint32_t kDialogPageId =
    0x07;

constexpr std::uint32_t kFirstRaceSourcePageId =
    0x3A;

constexpr std::uint32_t kNextRaceSourcePageId =
    0x72;

// Proven native Yes/No configuration.
constexpr std::uint32_t kDialogMode =
    2;

constexpr std::uint32_t kSecondaryMessageSelector =
    0x65;

// Beerio custom LBL1 label:
// decimal 60000 = 0xEA60
constexpr std::uint32_t kBeerioMessageLabel =
    0xEA60;

constexpr std::uint32_t kRequestFlag =
    1;

// UIDialogReq size established from the native request queue.
constexpr std::size_t kDialogRequestSize =
    0x128;

// Page_Dialog contains a copied UIDialogReq at +0x1D0.
constexpr std::ptrdiff_t kCopiedRequestOffset =
    0x1D0;

constexpr std::ptrdiff_t kRequestSourcePageIdOffset =
    0x00;

constexpr std::ptrdiff_t kRequestDialogModeOffset =
    0x0C;

constexpr std::ptrdiff_t kRequestMessageLabelOffset =
    0x18;

// Verified native Yes/No result field on Page_Dialog.
constexpr std::ptrdiff_t kDialogResultOffset =
    0x224;

alignas(16)
std::uint8_t gRequest[kDialogRequestSize]{};

bool gRequestConstructed = false;

using UIDialogReqCtorFn =
    void (*)(
        void* request
    );

using UIDialogReqConfigureFn =
    void (*)(
        void* request,
        const std::uint32_t* dialogMode,
        std::uint32_t messageLabel,
        std::uint32_t secondaryMessageSelector,
        const std::uint32_t* requestFlag
    );

using GetUIPageFn =
    void* (*)(
        const std::uint32_t* pageId
    );

using PageDialogOpenFn =
    std::uint64_t (*)(
        void* dialog,
        void* request,
        const std::uint32_t* sourcePageId
    );

template <typename T>
T FunctionAt(
    std::uintptr_t offset)
{
    return reinterpret_cast<T>(
        exl::util::modules::GetTargetStart() +
        offset
    );
}

const std::uint8_t* Bytes(
    const void* object)
{
    return reinterpret_cast<const std::uint8_t*>(
        object
    );
}

std::uint32_t ReadU32(
    const void* object,
    std::ptrdiff_t offset)
{
    return *reinterpret_cast<const std::uint32_t*>(
        Bytes(object) + offset
    );
}

bool IsVerifiedSourcePage(
    std::uint32_t sourcePageId)
{
    return
        sourcePageId == kFirstRaceSourcePageId ||
        sourcePageId == kNextRaceSourcePageId;
}

bool EnsureRequestConstructed()
{
    if (gRequestConstructed) {
        return true;
    }

    const auto ctor =
        FunctionAt<UIDialogReqCtorFn>(
            addr::UIDialogReq_Ctor
        );

    ctor(
        gRequest
    );

    gRequestConstructed = true;

    return true;
}

} // namespace

bool Show(
    std::uint32_t sourcePageId)
{
    if (!IsVerifiedSourcePage(
            sourcePageId)) {
        return false;
    }

    if (!EnsureRequestConstructed()) {
        return false;
    }

    const auto configure =
        FunctionAt<UIDialogReqConfigureFn>(
            addr::UIDialogReq_Configure
        );

    const auto getUIPage =
        FunctionAt<GetUIPageFn>(
            addr::GetUIPage
        );

    const auto openDialog =
        FunctionAt<PageDialogOpenFn>(
            addr::PageDialog_Open
        );

    const std::uint32_t dialogMode =
        kDialogMode;

    const std::uint32_t requestFlag =
        kRequestFlag;

    configure(
        gRequest,
        &dialogMode,
        kBeerioMessageLabel,
        kSecondaryMessageSelector,
        &requestFlag
    );

    const std::uint32_t dialogPageId =
        kDialogPageId;

    void* dialog =
        getUIPage(
            &dialogPageId
        );

    if (dialog == nullptr) {
        return false;
    }

    const std::uint64_t result =
        openDialog(
            dialog,
            gRequest,
            &sourcePageId
        );

    return
        (result & 1U) != 0;
}

bool IsBeerioDialog(
    const void* dialog)
{
    if (dialog == nullptr) {
        return false;
    }

    const auto* request =
        Bytes(dialog) +
        kCopiedRequestOffset;

    const std::uint32_t sourcePageId =
        ReadU32(
            request,
            kRequestSourcePageIdOffset
        );

    const std::uint32_t dialogMode =
        ReadU32(
            request,
            kRequestDialogModeOffset
        );

    const std::uint32_t messageLabel =
        ReadU32(
            request,
            kRequestMessageLabelOffset
        );

    return
        IsVerifiedSourcePage(sourcePageId) &&
        dialogMode == kDialogMode &&
        messageLabel == kBeerioMessageLabel;
}

Result GetResult(
    const void* dialog)
{
    if (dialog == nullptr) {
        return Result::None;
    }

    switch (ReadU32(
        dialog,
        kDialogResultOffset)) {
    case static_cast<std::uint32_t>(
        Result::Yes):
        return Result::Yes;

    case static_cast<std::uint32_t>(
        Result::No):
        return Result::No;

    default:
        return Result::None;
    }
}

} // namespace beerio::reminder_dialog
