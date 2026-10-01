#include "lib.hpp"
#include "base/memory/mem_utils.hpp"

#include "lib/log/logger_mgr.hpp"
#include <program/loggers.hpp>
#include <string>


namespace base::memory
{
    uintptr_t decodeADRPLDR(uintptr_t adrp_pc)
    {
        exl::util::ModuleInfo info = exl::util::GetMainModuleInfo();
        exl::util::Range text = info.m_Text;

        uintptr_t pc = adrp_pc + text.m_Start;

        uint32_t adrp = *reinterpret_cast<uint32_t*>(pc);
        uint32_t ldr = *reinterpret_cast<uint32_t*>(pc + 4);

        // Logging.Log("decodeADRPLDR()");
        // Logging.Log(std::to_string(reinterpret_cast<uint64_t>(adrp_pc)));
        // Logging.Log(std::to_string(reinterpret_cast<uint64_t>(pc)));

        if ((adrp & 0x9F000000) != 0x90000000) {
            Logging.Log("FATAL: decodeADRPLDR failed! Instruction is not an ADRP.");
            return 0; 
        }
        
        if ((ldr & 0xFFC00000) != 0xF9400000) {
            Logging.Log("FATAL: decodeADRPLDR failed! Instruction is not an LDR Xn.");
            return 0;
        }

        uint32_t immlo = (adrp >> 29) & 0x3;
        uint32_t immhi = (adrp >> 5) & 0x7FFFF;
        int32_t imm = (immhi << 2) | immlo;

        // Sign extend.
        if (imm & 0x100000) {
            imm |= 0xFFE00000;
        }

        uintptr_t base_page = pc & ~0xFFFULL;
        uintptr_t target_page = base_page + (static_cast<int64_t>(imm) << 12);

        uint32_t pimm = (ldr >> 10) & 0xFFF;
        uintptr_t offset = pimm << 3;

        return target_page + offset;
    }

    uintptr_t decodeADRPLDRRelative(uintptr_t adrp_pc)
    {
        uintptr_t addr = decodeADRPLDR(adrp_pc);
        if (addr == 0) {
            return 0;
        }

        exl::util::ModuleInfo info = exl::util::GetMainModuleInfo();
        exl::util::Range text = info.m_Text;

        return addr - text.m_Start;
    }

    uintptr_t relativeToGlobal(uintptr_t ptr)
    {
        exl::util::ModuleInfo info = exl::util::GetMainModuleInfo();
        exl::util::Range text = info.m_Text;

        return ptr + text.m_Start;
    }

    uintptr_t globalToRelative(uintptr_t ptr)
    {
        if (ptr == 0) {
            return 0;
        }

        exl::util::ModuleInfo info = exl::util::GetMainModuleInfo();
        exl::util::Range text = info.m_Text;

        return ptr - text.m_Start;
    }
}