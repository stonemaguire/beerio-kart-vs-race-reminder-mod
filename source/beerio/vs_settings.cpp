#include "beerio/vs_settings.hpp"

#include "beerio/addresses.hpp"
#include "beerio/custom_selector.hpp"

#include "lib.hpp"

#include <cstddef>
#include <cstdint>

namespace beerio::vs_settings {

namespace {

void* gActiveMenu = nullptr;

void* gRaceCountRow = nullptr;
void* gFrequencyRow = nullptr;
void* gFrequencyStartRow = nullptr;

game::OptionSelector* gRaceSelector = nullptr;

constexpr std::uint32_t kRaceCounts[] = {
    4,
    6,
    8,
    12,
    16,
    24,
    32,
    48,
    96
};

constexpr std::size_t kRaceCountChoiceCount =
    sizeof(kRaceCounts) / sizeof(kRaceCounts[0]);

game::MenuRowDescriptor MakeNormalDescriptor(
    void* rowObject,
    std::uint8_t visualIndex,
    std::uint64_t logicalIndex)
{
    game::MenuRowDescriptor descriptor{};

    descriptor.rowObject = rowObject;
    descriptor.visualIndex = visualIndex;
    descriptor.unknown09 = 0;
    descriptor.rowType = 1;
    descriptor.skipNavigation = 0;
    descriptor.enabled = 1;
    descriptor.logicalIndex = logicalIndex;

    return descriptor;
}

std::uint32_t GetRaceMaximum()
{
    if (gRaceSelector == nullptr) {
        return 1;
    }

    const std::uint32_t index =
        gRaceSelector->selectedIndex;

    if (index >= kRaceCountChoiceCount) {
        return 1;
    }

    return kRaceCounts[index];
}

} // namespace

bool ShouldInject(game::VsSettingsMenuView& menu)
{
    if (menu.Raw() == nullptr) {
        return false;
    }

    // Local VS configuration.
    if (menu.Mode() != 0) {
        return false;
    }

    // Only modify the exact native local-VS configuration that
    // we have reverse-engineered.
    if (menu.GridColumns() != 1) {
        return false;
    }

    if (menu.GridRows() < 9) {
        return false;
    }

    if (menu.SelectorCount() != 7) {
        return false;
    }

    if (menu.SelectorCapacity() < 9) {
        return false;
    }

    if (menu.SelectableCount() != 7) {
        return false;
    }

    if (menu.Type0Count() != 0) {
        return false;
    }

    if (menu.Type1Count() != 6) {
        return false;
    }

    if (menu.Type2Count() != 1) {
        return false;
    }

    if (menu.TotalRowCount() != 7) {
        return false;
    }

    game::MenuRowDescriptor* descriptors =
        menu.Descriptors();

    if (descriptors == nullptr) {
        return false;
    }

    game::OptionSelector** selectors =
        menu.Selectors();

    if (selectors == nullptr) {
        return false;
    }

    if (menu.GroupCount() < 2) {
        return false;
    }

    game::UiObjectGroup* groups =
        menu.Groups();

    if (groups == nullptr) {
        return false;
    }

    game::UiObjectGroup& listGroup =
        groups[1];

    // buildUi() creates fourteen L_List row objects.
    if (listGroup.count < 8 ||
        listGroup.capacity < 8 ||
        listGroup.objects == nullptr) {
        return false;
    }

    // Local VS uses:
    //
    // L_List_00
    // L_List_01
    // L_List_02
    // L_List_03
    // L_List_04
    // L_List_05 = Race Count
    //
    // The next two prebuilt rows are:
    //
    // L_List_06 = Frequency
    // L_List_07 = Frequency Start
    //
    // IMPORTANT:
    //
    // On the first-ever entry into the VS settings screen,
    // descriptor[6].rowObject and Race Count row +0x198 are not
    // necessarily populated yet.
    //
    // The group-1 object array and selector array ARE already
    // populated, so those are the authoritative sources at this
    // point in the menu lifecycle.

    void* raceRow =
        listGroup.objects[5];

    void* frequencyRow =
        listGroup.objects[6];

    void* frequencyStartRow =
        listGroup.objects[7];

    if (raceRow == nullptr ||
        frequencyRow == nullptr ||
        frequencyStartRow == nullptr) {
        return false;
    }

    // Local VS selector order:
    //
    // 0 Engine Class
    // 1 Teams
    // 2 Items
    // 3 COM
    // 4 COM Vehicles
    // 5 Courses
    // 6 Race Count
    //
    // We proved in GDB that selectorArray[6] is already fully
    // initialized on the first menu entry even while:
    //
    // raceRow + 0x198 == nullptr
    //
    // Therefore do NOT discover Race Count through the row.
    game::OptionSelector* raceSelector =
        selectors[6];

    if (raceSelector == nullptr ||
        raceSelector->vtable == nullptr) {
        return false;
    }

    // Sanity-check that selectorArray[6] really is the local VS
    // Race Count selector that we reverse-engineered.
    if (raceSelector->optionCount != 9) {
        return false;
    }

    if (raceSelector->variant != 1) {
        return false;
    }

    if (raceSelector->selectedIndex >=
        raceSelector->optionCount) {
        return false;
    }

    // First entry:
    //
    //     spare row +0x198 == nullptr
    //
    // Later entries:
    //
    //     Nintendo may reuse the same row object and our selector
    //     pointer may still be present.
    //
    // Allow either nullptr or our own selector. Reject anything else
    // so we don't overwrite a row being used by another mode.

    game::OptionSelector* currentFrequencySelector =
        game::RowSelector(frequencyRow);

    game::OptionSelector* currentFrequencyStartSelector =
        game::RowSelector(frequencyStartRow);

    if (currentFrequencySelector != nullptr &&
        currentFrequencySelector !=
            custom_selector::FrequencySelector()) {
        return false;
    }

    if (currentFrequencyStartSelector != nullptr &&
        currentFrequencyStartSelector !=
            custom_selector::FrequencyStartSelector()) {
        return false;
    }

    return true;
}

bool InjectCustomRows(game::VsSettingsMenuView& menu)
{
    if (!ShouldInject(menu)) {
        return false;
    }

    game::MenuRowDescriptor* descriptors =
        menu.Descriptors();

    game::UiObjectGroup* groups =
        menu.Groups();

    game::UiObjectGroup& listGroup =
        groups[1];

    // Use the preconstructed group-1 row objects directly.
    //
    // descriptor[6].rowObject can still be null on the first
    // menu entry.
    void* raceRow =
        listGroup.objects[5];

    void* frequencyRow =
        listGroup.objects[6];

    void* frequencyStartRow =
        listGroup.objects[7];

    game::OptionSelector** selectors =
        menu.Selectors();

    if (descriptors == nullptr ||
        raceRow == nullptr ||
        frequencyRow == nullptr ||
        frequencyStartRow == nullptr ||
        selectors == nullptr) {
        return false;
    }

    // selectorArray[6] is already initialized on the first entry,
    // even though raceRow +0x198 is still null.
    game::OptionSelector* raceSelector =
        selectors[6];

    if (raceSelector == nullptr ||
        raceSelector->vtable == nullptr) {
        return false;
    }

    if (raceSelector->optionCount != 9 ||
        raceSelector->variant != 1) {
        return false;
    }

    if (!custom_selector::Initialize(
            raceSelector)) {
        return false;
    }

    gActiveMenu = menu.Raw();

    gRaceCountRow = raceRow;
    gFrequencyRow = frequencyRow;
    gFrequencyStartRow = frequencyStartRow;

    gRaceSelector = raceSelector;

    // Set the maximum for Frequency and Frequency Start based on
    // the currently selected Race Count.
    custom_selector::SetMaximum(
        GetRaceMaximum()
    );

    // Attach our custom selectors to the two spare native row
    // objects.
    game::RowSelector(gFrequencyRow) =
        custom_selector::FrequencySelector();

    game::RowSelector(gFrequencyStartRow) =
        custom_selector::FrequencyStartSelector();

    // Append the custom selectors to Nintendo's selector array.
    selectors[7] =
        custom_selector::FrequencySelector();

    selectors[8] =
        custom_selector::FrequencyStartSelector();

    menu.SelectorCount() = 9;

    // Descriptor 7:
    //
    // +0x00 Frequency row pointer
    // +0x08 metadata 0x0000000100010006
    // +0x10 logical index 7
    descriptors[7] =
        MakeNormalDescriptor(
            gFrequencyRow,
            6,
            7
        );

    // Descriptor 8:
    //
    // +0x00 Frequency Start row pointer
    // +0x08 metadata 0x0000000100010007
    // +0x10 logical index 8
    descriptors[8] =
        MakeNormalDescriptor(
            gFrequencyStartRow,
            7,
            8
        );

    // Native local VS:
    //
    // selectorCount    = 7
    // selectableCount  = 7
    // type1Count       = 6
    // type2Count       = 1
    // totalRowCount    = 7
    //
    // Beerio local VS:
    //
    // selectorCount    = 9
    // selectableCount  = 9
    // type1Count       = 8
    // type2Count       = 1
    // totalRowCount    = 9

    menu.SelectableCount() = 9;
    menu.Type1Count() = 8;
    menu.TotalRowCount() = 9;

    return true;
}

void UpdateLimitsFromRaceCount()
{
    if (gRaceSelector == nullptr) {
        return;
    }

    custom_selector::SetMaximum(
        GetRaceMaximum()
    );
}

void RefreshCustomValues()
{
    if (gFrequencyRow == nullptr ||
        gFrequencyStartRow == nullptr) {
        return;
    }

    using RefreshValueTextFn =
        void (*)(void* row);

    const std::uintptr_t targetStart =
        exl::util::modules::GetTargetStart();

    auto refreshValueText =
        reinterpret_cast<RefreshValueTextFn>(
            targetStart +
            addr::OptionRow_RefreshValueText
        );

    refreshValueText(
        gFrequencyRow
    );

    refreshValueText(
        gFrequencyStartRow
    );
}

void OnRaceCountRowEvent(void* row)
{
    if (row == nullptr) {
        return;
    }

    if (gActiveMenu == nullptr ||
        gRaceCountRow == nullptr) {
        return;
    }

    if (row != gRaceCountRow) {
        return;
    }

    UpdateLimitsFromRaceCount();
    RefreshCustomValues();
}

void* ActiveMenu()
{
    return gActiveMenu;
}

void* RaceCountRow()
{
    return gRaceCountRow;
}

void* FrequencyRow()
{
    return gFrequencyRow;
}

void* FrequencyStartRow()
{
    return gFrequencyStartRow;
}

} // namespace beerio::vs_settings
