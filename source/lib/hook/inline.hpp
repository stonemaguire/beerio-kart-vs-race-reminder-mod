#pragma once

#include <common.hpp>

#include "base.hpp"
#include "nn/os/os_ro.hpp"

#define HOOK_DEFINE_INLINE(name)                        \
struct name : public ::exl::hook::impl::InlineHook<name>

namespace exl::hook::impl {

    template<typename Derived>
    struct InlineHook {
        
        template<typename T = Derived>
        using CallbackFuncPtr = decltype(&T::Callback);

        static ALWAYS_INLINE void InstallAtOffset(ptrdiff_t address) {
            _HOOK_STATIC_CALLBACK_ASSERT();

            hook::HookInline(util::modules::GetTargetStart() + address, Derived::Callback);
        }

        static ALWAYS_INLINE void InstallAtPtr(uintptr_t ptr) {
            _HOOK_STATIC_CALLBACK_ASSERT();
            
            hook::HookInline(ptr, Derived::Callback);
        }

        static ALWAYS_INLINE void InstallAtSymbol(const char* symbol) {
			_HOOK_STATIC_CALLBACK_ASSERT();

			char error_message[256] = { 0 };
			snprintf(error_message, sizeof(error_message), "Symbol %s not found!", symbol);

			uintptr_t address = 0;
			if (!R_SUCCEEDED(nn::ro::LookupSymbol(&address, symbol))) {
				svcOutputDebugString(error_message, sizeof(error_message));
				return;
			}

			hook::HookInline(address, Derived::Callback);
		}
    };
}