#pragma once

#include <cstring>
#include <cstdint>
#include <memory>

namespace base::memory
{
    struct ItaniumPtmf {
        uintptr_t ptr;
        uintptr_t adj;
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    inline uintptr_t ExtractVTableOffset(ReturnType (ClassType::*memFunc)(Args...)) 
    {
        ItaniumPtmf ptmf;
        std::memcpy(&ptmf, &memFunc, sizeof(ptmf));
        
        if (ptmf.adj & 1) {
            return ptmf.ptr; 
        }
        
        return 0; 
    }
    
    uintptr_t decodeADRPLDR(uintptr_t adrp_pc);
    uintptr_t decodeADRPLDRRelative(uintptr_t adrp_pc);

    uintptr_t relativeToGlobal(uintptr_t ptr);
    uintptr_t globalToRelative(uintptr_t ptr);
}
