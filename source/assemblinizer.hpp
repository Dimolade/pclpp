// https://armconverter.com

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

class Assembly
{
public:
    std::vector<uint8_t> code;
    uint32_t startAddress = 0;
    uint instructs = 0;
    inline uint32_t InstructionOffset()
    {
        return instructs > 0 ? (instructs-1)*4 : 0;
    }

#ifdef kynex_CTRL
    CTRLCodeRegion codeRegion = nullptr;
    u8* codeBlockData = nullptr;
    inline void allocStartAddress()
    {
        codeBlockData = ctrlAllocCodeBlock(&codeRegion, instructs*4);
    }

    inline void allocStartAddress_AfterCommit(int index = 0)
    {
        startAddress = ctrlGetCodeBlock(codeRegion, index);
    }

    inline void commitCodeRegion()
    {
        ctrlCommitCodeRegion(&codeRegion);
    }

    inline void setupInstructions()
    {
        if (codeBlockData) {
            memcpy(codeBlockData, code.data(), instructs*4);
        }
    }

    inline void unalloc()
    {
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
        instructs++;
    }

    inline void NOP()
    {
        emit32(0x00F020E3, true);
    }

    inline void CallFunction(uint32_t function)
    {
        MOVRImm(5,function);
        BLX(5);
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

    inline void ADDRR(uint8_t target, uint8_t reg, uint8_t with = 16)
    {
        uint8_t rn, rm;

        if (with <= 15) {
            rn = reg;
            rm = with;
        } else {
            rn = target;
            rm = reg;
        }

        emit32(0xE0800000 |
            ((rn & 0xF) << 16) |
            ((target & 0xF) << 12) |
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

        size_t words = code.size() / 4;
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
