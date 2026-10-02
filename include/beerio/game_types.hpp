#pragma once

#include <cstddef>
#include <cstdint>

namespace beerio::game {

template <typename T>
inline T& Field(void* object, std::ptrdiff_t offset)
{
    return *reinterpret_cast<T*>(
        reinterpret_cast<std::uint8_t*>(object) + offset
    );
}

struct OptionSelector
{
    // +0x00
    std::uintptr_t* vtable;

    // +0x08
    std::uint8_t selectedIndex;

    // +0x09
    std::uint8_t optionCount;

    // +0x0A
    std::uint8_t enabled;

    // +0x0B
    std::uint8_t dirty;

    // +0x0C
    std::uint32_t variant;
};

static_assert(offsetof(OptionSelector, vtable) == 0x00);
static_assert(offsetof(OptionSelector, selectedIndex) == 0x08);
static_assert(offsetof(OptionSelector, optionCount) == 0x09);
static_assert(offsetof(OptionSelector, enabled) == 0x0A);
static_assert(offsetof(OptionSelector, dirty) == 0x0B);
static_assert(offsetof(OptionSelector, variant) == 0x0C);
static_assert(sizeof(OptionSelector) == 0x10);

struct MenuRowDescriptor
{
    // +0x00
    void* rowObject;

    // +0x08
    std::uint8_t visualIndex;

    // +0x09
    std::uint8_t unknown09;

    // +0x0A
    std::uint8_t rowType;

    // +0x0B
    std::uint8_t skipNavigation;

    // +0x0C
    std::uint8_t enabled;

    // +0x0D..+0x0F
    std::uint8_t padding[3];

    // +0x10
    std::uint64_t logicalIndex;
};

static_assert(offsetof(MenuRowDescriptor, rowObject) == 0x00);
static_assert(offsetof(MenuRowDescriptor, visualIndex) == 0x08);
static_assert(offsetof(MenuRowDescriptor, rowType) == 0x0A);
static_assert(offsetof(MenuRowDescriptor, skipNavigation) == 0x0B);
static_assert(offsetof(MenuRowDescriptor, enabled) == 0x0C);
static_assert(offsetof(MenuRowDescriptor, logicalIndex) == 0x10);
static_assert(sizeof(MenuRowDescriptor) == 0x18);

struct UiObjectGroup
{
    // +0x00
    std::uint32_t count;

    // +0x04
    std::uint32_t capacity;

    // +0x08
    void** objects;
};

static_assert(offsetof(UiObjectGroup, count) == 0x00);
static_assert(offsetof(UiObjectGroup, capacity) == 0x04);
static_assert(offsetof(UiObjectGroup, objects) == 0x08);
static_assert(sizeof(UiObjectGroup) == 0x10);

class VsSettingsMenuView
{
public:
    explicit VsSettingsMenuView(void* object)
        : mObject(object)
    {
    }

    void* Raw() const
    {
        return mObject;
    }

    // Descriptor-grid width.
    // Local VS: 1.
    std::uint32_t& GridColumns()
    {
        return Field<std::uint32_t>(mObject, 0xB8);
    }

    // Descriptor-grid allocated row count.
    // Observed local VS value: 23.
    std::uint32_t& GridRows()
    {
        return Field<std::uint32_t>(mObject, 0xBC);
    }

    MenuRowDescriptor*& Descriptors()
    {
        return Field<MenuRowDescriptor*>(mObject, 0xC0);
    }

    // Active grid width.
    // Local VS: 1.
    std::uint32_t& ActiveColumns()
    {
        return Field<std::uint32_t>(mObject, 0xC8);
    }

    // Active grid height.
    // Native local VS: 7.
    // Beerio local VS: 9.
    std::uint32_t& ActiveRows()
    {
        return Field<std::uint32_t>(mObject, 0xCC);
    }

    // Number of UI object groups.
    std::uint32_t& GroupCount()
    {
        return Field<std::uint32_t>(mObject, 0xE0);
    }

    UiObjectGroup*& Groups()
    {
        return Field<UiObjectGroup*>(mObject, 0xE8);
    }

    std::uint32_t& SelectedColumn()
    {
        return Field<std::uint32_t>(mObject, 0x168);
    }

    std::uint32_t& SelectedRow()
    {
        return Field<std::uint32_t>(mObject, 0x16C);
    }

    // Number of entries currently stored in the selector pointer array.
    std::uint32_t& SelectorCount()
    {
        return Field<std::uint32_t>(mObject, 0x2E0);
    }

    // Observed capacity: 20.
    std::uint32_t& SelectorCapacity()
    {
        return Field<std::uint32_t>(mObject, 0x2E4);
    }

    OptionSelector**& Selectors()
    {
        return Field<OptionSelector**>(mObject, 0x2E8);
    }

    std::uint32_t& Type0Count()
    {
        return Field<std::uint32_t>(mObject, 0x2F0);
    }

    // Native local VS: 7.
    std::uint32_t& SelectableCount()
    {
        return Field<std::uint32_t>(mObject, 0x2F4);
    }

    // Native local VS: 6.
    std::uint32_t& Type1Count()
    {
        return Field<std::uint32_t>(mObject, 0x2F8);
    }

    // Native local VS: 1.
    std::uint32_t& Type2Count()
    {
        return Field<std::uint32_t>(mObject, 0x2FC);
    }

    // Native local VS: 7.
    std::uint32_t& TotalRowCount()
    {
        return Field<std::uint32_t>(mObject, 0x300);
    }

    // Local VS mode/configuration: 0.
    std::uint32_t& Mode()
    {
        return Field<std::uint32_t>(mObject, 0x304);
    }

private:
    void* mObject;
};

inline OptionSelector*& RowSelector(void* row)
{
    return Field<OptionSelector*>(row, 0x198);
}

} // namespace beerio::game
