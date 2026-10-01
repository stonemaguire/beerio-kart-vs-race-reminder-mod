#pragma once

#include "base.hpp"
#include "util/func_ptrs.hpp"
#include "util/type_traits.hpp"

#include "nn/os/os_ro.hpp"

#define HOOK_DEFINE_REPLACE(name)                        \
struct name : public ::exl::hook::impl::ReplaceHook<name>

namespace exl::hook::impl {

    template<typename Derived>
    struct ReplaceHook {

        template<typename T = Derived>
        using CallbackFuncPtr = decltype(&T::Callback);

        static ALWAYS_INLINE void InstallAtOffset(ptrdiff_t address) {
            _HOOK_STATIC_CALLBACK_ASSERT();

            hook::Hook(util::modules::GetTargetStart() + address, Derived::Callback);
        }

        template<typename T>
        static ALWAYS_INLINE void InstallAtFuncPtr(T ptr) {
            _HOOK_STATIC_CALLBACK_ASSERT();

            using Traits = util::FuncPtrTraits<T>;
            static_assert(std::is_same_v<typename Traits::CPtr, CallbackFuncPtr<>>, "Argument pointer type must match callback type!");

            hook::Hook(ptr, Derived::Callback);
        }

        static ALWAYS_INLINE void InstallAtPtr(uintptr_t ptr) {
            _HOOK_STATIC_CALLBACK_ASSERT();
            
            hook::Hook(ptr, Derived::Callback);
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

			hook::Hook(address, Derived::Callback);
		}
    };
}