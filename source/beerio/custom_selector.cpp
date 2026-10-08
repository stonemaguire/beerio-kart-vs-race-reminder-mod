#include "beerio/custom_selector.hpp"
#include "beerio/text_config.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace beerio::custom_selector {

namespace {

constexpr std::size_t kVtableEntries = 12;

constexpr std::size_t kIndexToOptionSlot =
    0x18 / sizeof(std::uintptr_t);

constexpr std::size_t kLabelTextSlot =
    0x28 / sizeof(std::uintptr_t);

constexpr std::size_t kOptionToTextSlot =
    0x30 / sizeof(std::uintptr_t);

constexpr std::size_t kShouldSkipSlot =
    0x48 / sizeof(std::uintptr_t);

std::array<std::uintptr_t, kVtableEntries> gFrequencyVtable{};
std::array<std::uintptr_t, kVtableEntries> gFrequencyStartVtable{};

CustomSelector gFrequency{};
CustomSelector gFrequencyStart{};

bool gInitialized = false;

std::uint32_t IndexToOption(
    game::OptionSelector* selector,
    std::int32_t index)
{
    (void)selector;

    if (index < 0) {
        return 1;
    }

    return static_cast<std::uint32_t>(index) + 1;
}

const char16_t* FrequencyLabel(
    game::OptionSelector* selector)
{
    (void)selector;

    return text_config::FrequencyLabel;
}

const char16_t* FrequencyStartLabel(
    game::OptionSelector* selector)
{
    (void)selector;

    return text_config::FrequencyStartLabel;
}

char16_t* AppendString(
    char16_t* output,
    const char16_t* text)
{
    while (*text != u'\0') {
        *output++ = *text++;
    }

    return output;
}

char16_t* AppendNumber(
    char16_t* output,
    std::uint32_t value)
{
    if (value >= 100) {
        *output++ =
            static_cast<char16_t>(
                u'0' + ((value / 100) % 10)
            );
    }

    if (value >= 10) {
        *output++ =
            static_cast<char16_t>(
                u'0' + ((value / 10) % 10)
            );
    }

    *output++ =
        static_cast<char16_t>(
            u'0' + (value % 10)
        );

    return output;
}

const char16_t* OrdinalSuffix(
    std::uint32_t value)
{
    // 11, 12, and 13 are always:
    //
    // 11th
    // 12th
    // 13th
    //
    // even though their final digits would otherwise suggest
    // 11st, 12nd, and 13rd.
    const std::uint32_t lastTwo =
        value % 100;

    if (lastTwo >= 11 &&
        lastTwo <= 13) {
        return u"th";
    }

    switch (value % 10) {
    case 1:
        return u"st";

    case 2:
        return u"nd";

    case 3:
        return u"rd";

    default:
        return u"th";
    }
}

const char16_t* IntegerOptionToText(
    game::OptionSelector* selector,
    std::uint32_t value)
{
    auto* custom =
        reinterpret_cast<CustomSelector*>(
            selector
        );

    // Current feature range is 1..96.
    // Keep this defensive in case the selector is ever expanded.
    if (value < 1) {
        value = 1;
    }

    if (value > 999) {
        value = 999;
    }

    char16_t* output =
        custom->valueText;

    // Frequency:
    //
    //     Every 1 Race
    //     Every 2 Races
    //
    // Frequency Start:
    //
    //     27th Race
    const bool isFrequency =
        selector ==
            &gFrequency.base;

    if (isFrequency) {
        output =
            AppendString(
                output,
                u"Every "
            );
    }

    output =
        AppendNumber(
            output,
            value
        );

    if (isFrequency) {
        output =
            AppendString(
                output,
                u" Race"
            );

        if (value != 1) {
            output =
                AppendString(
                    output,
                    u"s"
                );
        }
    }
    else {
        output =
            AppendString(
                output,
                OrdinalSuffix(value)
            );

        output =
            AppendString(
                output,
                u" Race"
            );
    }

    *output = u'\0';

    return custom->valueText;
}

bool NeverSkipOption(
    game::OptionSelector* selector,
    std::uint32_t option)
{
    (void)selector;
    (void)option;

    return false;
}

void InitializeOne(
    CustomSelector& selector,
    std::array<std::uintptr_t, kVtableEntries>& vtable)
{
    selector.base.vtable =
        vtable.data();

    // selectedIndex 0 corresponds to displayed value 1.
    selector.base.selectedIndex = 0;

    // Replaced immediately with the selected Race Count.
    selector.base.optionCount = 1;

    // OptionRow_refreshValueText displays "-----"
    // when this is zero.
    selector.base.enabled = 1;

    selector.base.dirty = 0;
    selector.base.variant = 0;

    std::memset(
        selector.valueText,
        0,
        sizeof(selector.valueText)
    );
}

void ClampOne(
    CustomSelector& selector,
    std::uint32_t maximum)
{
    if (maximum < 1) {
        maximum = 1;
    }

    // Current maximum Race Count is 96.
    if (maximum > 96) {
        maximum = 96;
    }

    selector.base.optionCount =
        static_cast<std::uint8_t>(
            maximum
        );

    if (selector.base.selectedIndex >=
        maximum) {

        selector.base.selectedIndex =
            static_cast<std::uint8_t>(
                maximum - 1
            );
    }
}

} // namespace

bool Initialize(
    game::OptionSelector* raceSelector)
{
    if (raceSelector == nullptr ||
        raceSelector->vtable == nullptr) {
        return false;
    }

    if (gInitialized) {
        return true;
    }

    std::memcpy(
        gFrequencyVtable.data(),
        raceSelector->vtable,
        sizeof(gFrequencyVtable)
    );

    std::memcpy(
        gFrequencyStartVtable.data(),
        raceSelector->vtable,
        sizeof(gFrequencyStartVtable)
    );

    // Keep Nintendo's native implementations for:
    //
    // +0x20 getCurrentOption
    // +0x38 getCurrentOptionText
    // +0x40 changeSelection
    //
    // Replace only the behavior specific to our selectors.

    gFrequencyVtable[kIndexToOptionSlot] =
        reinterpret_cast<std::uintptr_t>(
            &IndexToOption
        );

    gFrequencyVtable[kLabelTextSlot] =
        reinterpret_cast<std::uintptr_t>(
            &FrequencyLabel
        );

    gFrequencyVtable[kOptionToTextSlot] =
        reinterpret_cast<std::uintptr_t>(
            &IntegerOptionToText
        );

    gFrequencyVtable[kShouldSkipSlot] =
        reinterpret_cast<std::uintptr_t>(
            &NeverSkipOption
        );

    gFrequencyStartVtable[kIndexToOptionSlot] =
        reinterpret_cast<std::uintptr_t>(
            &IndexToOption
        );

    gFrequencyStartVtable[kLabelTextSlot] =
        reinterpret_cast<std::uintptr_t>(
            &FrequencyStartLabel
        );

    gFrequencyStartVtable[kOptionToTextSlot] =
        reinterpret_cast<std::uintptr_t>(
            &IntegerOptionToText
        );

    gFrequencyStartVtable[kShouldSkipSlot] =
        reinterpret_cast<std::uintptr_t>(
            &NeverSkipOption
        );

    InitializeOne(
        gFrequency,
        gFrequencyVtable
    );

    InitializeOne(
        gFrequencyStart,
        gFrequencyStartVtable
    );

    gInitialized = true;

    return true;
}

game::OptionSelector* FrequencySelector()
{
    return &gFrequency.base;
}

game::OptionSelector* FrequencyStartSelector()
{
    return &gFrequencyStart.base;
}

void SetMaximum(
    std::uint32_t maximum)
{
    ClampOne(
        gFrequency,
        maximum
    );

    ClampOne(
        gFrequencyStart,
        maximum
    );
}

std::uint32_t Frequency()
{
    return
        static_cast<std::uint32_t>(
            gFrequency.base.selectedIndex
        ) + 1;
}

std::uint32_t FrequencyStart()
{
    return
        static_cast<std::uint32_t>(
            gFrequencyStart.base.selectedIndex
        ) + 1;
}

} // namespace beerio::custom_selector
