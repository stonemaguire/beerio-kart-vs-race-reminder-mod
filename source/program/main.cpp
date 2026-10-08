#include "lib.hpp"

#include "base/pointers.hpp"

#include "beerio/hooks.hpp"

extern "C" void exl_main(
    void* x0,
    void* x1)
{
    EXL_UNUSED(x0, x1);

    // Required before installing exlaunch hooks.
    exl::hook::Initialize();

    // Preserve the base-plugin scaffold's pointer initialization.
    base::Pointers::create();

    beerio::hooks::Install();

    Logging.Log(
        "Beerio VS settings and race gate hooks installed."
    );
}
