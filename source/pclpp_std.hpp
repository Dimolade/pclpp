#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

class pclpp_std
{
public:
    static uint32_t Malloc(uint16_t size)
    {
        return (uint32_t)malloc(size);
    }

    static uint32_t Calloc(uint16_t elements, uint16_t size)
    {
        return (uint32_t)calloc(elements, size);
    }

    static void Free(uint32_t address)
    {
        free((int*)address);
    }

    static void Printf(uint32_t address)
    {
        printf("%s", (char*)address);
    }

    static void Write8(uint32_t address, uint8_t value)
    {
        uint8_t* mem = (uint8_t*)address;
        *mem = value;
    }

    static void Write16(uint32_t address, uint16_t value)
    {
        uint16_t* mem = (uint16_t*)address;
        *mem = value;
    }

    static void Write32(uint32_t address, uint32_t value)
    {
        uint32_t* mem = (uint32_t*)address;
        *mem = value;
    }

    static uint8_t Read8(uint32_t address)
    {
        uint8_t* mem = (uint8_t*)address;
        return *mem;
    }

    static uint16_t Read16(uint32_t address)
    {
        uint16_t* mem = (uint16_t*)address;
        return *mem;
    }

    static uint32_t Read32(uint32_t address)
    {
        uint32_t* mem = (uint32_t*)address;
        return *mem;
    }
};
