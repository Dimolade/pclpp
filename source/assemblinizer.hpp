// https://armconverter.com
#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iomanip>

#define kynex_CTRL

#ifdef kynex_CTRL
#include "../CTRLF/CTRL/Memory.h"
#include "../CTRLF/CTRL/Hook.h"
#include "../CTRLF/CTRL/Exception.h"
#include "../CTRLF/CTRL/App.h"
#include "../CTRLF/CTRL/CodeGen.h"
#include "../CTRLF/CTRL/CodeAllocator.h"
#include "../CTRLF/CTRL/Arch.h"
#endif

#define assemblinizer_littleEndian

struct PCLPP_Assembly_Map_Result
{
    bool success;
    uint32_t result = 0;

    PCLPP_Assembly_Map_Result(bool s, uint32_t r = 0) :
    success(s), result(r) {}
};

class PCLPP_Assembly
{
public:
    std::vector<uint8_t> code;
    uint32_t startAddress = 0;
    uint32_t Instructs()
    {
        if (code.size() == 0) return 0;
        return code.size()/4;
    }
#ifdef kynex_CTRL
    CTRLCodeRegion codeRegion = nullptr;
    u8* codeBlockData = nullptr;
    inline bool allocStartAddress()
    {
        codeBlockData = ctrlAllocCodeBlock(&codeRegion, code.size());
        if (codeBlockData)
        {
            return true;
        }
        return false;
    }

    inline bool allocStartAddress_AfterCommit(int index = 0)
    {
        startAddress = ctrlGetCodeBlock(codeRegion, index);
        if (startAddress)
        {
            return true;
        }
        return false;
    }

    inline PCLPP_Assembly_Map_Result commitCodeRegion()
    {
        Result res = ctrlCommitCodeRegion(&codeRegion);
        if (R_FAILED(res))
        {
            return PCLPP_Assembly_Map_Result(false, res);
        }
        return PCLPP_Assembly_Map_Result(true);
    }

    inline bool setupInstructions()
    {
        if (codeBlockData) {
            memcpy(codeBlockData, code.data(), code.size());
            return true;
        }
        return false;
    }

    inline void unalloc()
    {
        if (codeRegion != nullptr && startAddress != 0)
            ctrlDestroyCodeRegion(&codeRegion);
    }
#else
    inline void allocStartAddress()
    {
        // make magic with byteSize
    }
#endif

    inline void emit32(uint32_t opcode, bool reverseWord = false)
    {
        code.reserve(code.size()+4);
        if (reverseWord) {
            // Swap byte order: 0xE12FFF1E <-> 0x1EFF2FE1
            opcode = ((opcode & 0x000000FF) << 24) |
                    ((opcode & 0x0000FF00) << 8)  |
                    ((opcode & 0x00FF0000) >> 8)  |
                    ((opcode & 0xFF000000) >> 24);
        }

#ifdef assemblinizer_littleEndian
        for (int i = 0; i < 4; i++)
            code.push_back((opcode >> (i * 8)) & 0xFF);
#else
        for (int i = 3; i >= 0; i--)
            code.push_back((opcode >> (i * 8)) & 0xFF);
#endif
    }

    inline void NOP()
    {
        emit32(0x00F020E3, true);
    }

    inline void CallFunction(uint32_t function)
    {
        MOVRImm(10,function);
        BLX(10);
    }

    inline void BX(uint8_t reg)
    {
        emit32(0xE12FFF10 | (reg & 0xF));
    }

    inline void BLX(uint8_t reg)
    {
        emit32(0xE12FFF30 | (reg & 0xF));
    }

    inline void BXLR() { emit32(0x1EFF2FE1, true); }

    inline void PUSH_LR()
    {
        emit32(0x04E02DE5, true);
    }

    inline void POP_LR()
    {
        emit32(0x04E09DE4, true);
    }

    inline void PUSHUNSAFE()
    {
        emit32(0x1F002DE9, true);
    }

    inline void POPUNSAFE()
    {
        emit32(0x1F00BDE8, true);
    }

    inline void PUSHEVERYTHING()
    {
        emit32(0xFF032DE9, true);
    }

    inline void POPEVERYTHING()
    {
        emit32(0xFF03BDE8, true);
    }

    inline void PUSH(uint32_t regMask)
    {
        emit32(0xE92D0000 | regMask);
    }

    inline void POP(uint32_t regMask)
    {
        emit32(0xE8BD0000 | regMask);
    }

    inline bool EncodeImm(uint32_t val, uint8_t reg, uint32_t opcode)
    {
        for (int rot = 0; rot < 16; rot++) {

            uint32_t r = (val << (rot * 2)) | (val >> (32 - rot * 2));

            if ((r & ~0xFF) == 0) {
                emit32(opcode |
                    ((reg & 0xF) << 12) |
                    (rot << 8) |
                    (r & 0xFF));
                return true;
            }
        }
        return false;
    }

    inline void CMPRR(uint8_t rn, uint8_t rm)
    {
        emit32(0xE1500000 |
            ((rn & 0xF) << 16) |
            (rm & 0xF));
    }

    inline void CMNRR(uint8_t rn, uint8_t rm)
    {
        emit32(0xE1700000 |
            ((rn & 0xF) << 16) |
            (rm & 0xF));
    }

    inline void MOVW(uint8_t reg, uint16_t imm)
    {
        uint32_t imm4  = (imm >> 12) & 0xF;
        uint32_t imm12 = imm & 0xFFF;

        emit32(0xE3000000 |
            (imm4 << 16) |
            ((reg & 0xF) << 12) |
            imm12);
    }

    inline void MOVT(uint8_t reg, uint16_t imm)
    {
        uint32_t imm4  = (imm >> 12) & 0xF;
        uint32_t imm12 = imm & 0xFFF;

        emit32(0xE3400000 |
            (imm4 << 16) |
            ((reg & 0xF) << 12) |
            imm12);
    }

    inline void MOVRImm(uint8_t reg, uint32_t val)
    {
        if (EncodeImm(val, reg, 0xE3A00000))
            return;

        if (EncodeImm(~val, reg, 0xE3E00000))
            return;

        MOVW(reg, val & 0xFFFF);
        MOVT(reg, (val >> 16) & 0xFFFF);
    }

    inline void MOVRR(uint8_t target, uint8_t reg)
    {
        emit32(0xE1A00000 |
            ((target & 0xF) << 12) |
            (reg & 0xF));
    }

    inline void ADDRImm(uint8_t reg, uint32_t val)
    {
        if (EncodeImm(val, reg, 0xE2800000))
            return;

        if (EncodeImm(~val, reg, 0xE2400000))
            return;

    }

    inline void ADDRR(uint8_t target, uint8_t reg, uint8_t with = 16, bool eq = false)
    {
        uint8_t rn, rm;

        if (with <= 15) {
            rn = reg;
            rm = with;
        } else {
            rn = target;
            rm = reg;
        }

        uint32_t base = eq ? 0x00800000 : 0xE0800000;

        emit32(0xE0800000 |
            ((rn & 0xF) << 16) |
            ((target & 0xF) << 12) |
            (rm & 0xF));
    }

    inline void MULRR(uint8_t target, uint8_t reg, uint8_t with = 16)
    {
        uint8_t rm, rs;

        if (with <= 15) {
            rm = reg;
            rs = with;
        } else {
            rm = target;
            rs = reg;
        }

        emit32(0xE0000090 |
            ((target & 0xF) << 16) |
            ((rs & 0xF) << 8) |
            (rm & 0xF));
    }

    inline void SUBRImm(uint8_t reg, uint32_t val)
    {
        if (EncodeImm(val, reg, 0xE2400000))
            return;

        if (EncodeImm(~val, reg, 0xE2800000))
            return;

    }

    inline void SUBRR(uint8_t target, uint8_t reg, uint8_t with = 16)
    {
        uint8_t rn, rm;

        if (with <= 15) {
            rn = reg;
            rm = with;
        } else {
            rn = target;
            rm = reg;
        }

        emit32(0xE0400000 |
            ((rn & 0xF) << 16) |
            ((target & 0xF) << 12) |
            (rm & 0xF));
    }

    inline std::string AsString()
    {
        std::ostringstream oss;

        size_t words = Instructs();
        for (size_t i = 0; i < words; ++i)
        {
            uint32_t word = code[i*4] |
                            (code[i*4 + 1] << 8) |
                            (code[i*4 + 2] << 16) |
                            (code[i*4 + 3] << 24);

            oss << "0x" << std::hex << std::uppercase
                << std::setw(8) << std::setfill('0')
                << word;

            if (i + 1 < words)
                oss << " ";
        }

        return oss.str();
    }
};
