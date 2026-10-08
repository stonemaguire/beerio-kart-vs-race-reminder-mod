#pragma once

#include <cstdint>

namespace beerio::reminder_dialog {

enum class Result : std::uint32_t
{
    None = 0,
    Yes = 1,
    No = 2
};

// Opens the native Yes/No dialog using Beerio's custom MSBT label.
//
// sourcePageId must be the page that attempted the verified transition
// to Page 0x6E:
//
//     0x3A for Race 1
//     0x72 for Race 2+
bool Show(
    std::uint32_t sourcePageId
);

// Identifies Beerio's copied UIDialogReq while Page_Dialog is closing.
bool IsBeerioDialog(
    const void* dialog
);

// Reads the native result field that we verified in Page_Dialog:
//
//     1 = Yes
//     2 = No
Result GetResult(
    const void* dialog
);

} // namespace beerio::reminder_dialog
