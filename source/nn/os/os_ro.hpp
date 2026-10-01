#include <cstdint>
#include <nx/types.h>

namespace nn::ro {
	::Result LookupSymbol(uintptr_t* out, const char* name);
}