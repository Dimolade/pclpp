#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <vector>
#include <stack>
#include <iostream>

//#define pclpp_std_debug

class pclpp_varpool
{
public:
    pclpp_varpool(uint16_t capacity)
        : data(capacity), used(capacity, false)
    {
        for (int i = capacity - 1; i >= 0; i--)
        {
            freeList.push(i);
        }
    }

    uint16_t allocate(uint32_t value)
    {
        if (freeList.empty())
            return -1;

        uint16_t index = freeList.top();
        freeList.pop();

        data[index] = value;
        used[index] = true;

        return index;
    }

    void free(uint16_t index)
    {
        if (!used[index])
            return;

        used[index] = false;
        freeList.push(index);
    }

    uint32_t& operator[](uint16_t index)
    {
        if (!used[index])
            return;

        return data[index];
    }

private:
    std::vector<uint32_t> data;
    std::vector<bool> used;
    std::stack<uint16_t> freeList;
};

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

    static inline pclpp_varpool localvariablemanager{65535};

    static uint16_t AllocateLocal(uint32_t value)
    {
        uint16_t index = localvariablemanager.allocate(value);
        #ifdef pclpp_std_debug
        std::cout << "Allocate Local: " << std::to_string(value) << std::endl;
        std::cout << "Local Index: " << std::to_string(index) << std::endl;
        #endif
        return index;
    }

    static uint32_t GetLocal(uint16_t index)
    {
        #ifdef pclpp_std_debug
        std::cout << "Getting Local: " << std::to_string(index) << std::endl;
        std::cout << "Local Value: " << std::to_string(localvariablemanager[index]) << std::endl;
        #endif
        return localvariablemanager[index];
    }

    static void UnallocateLocal(uint16_t index)
    {
        #ifdef pclpp_std_debug
        std::cout << "Unallocate Local: " << std::to_string(index) << std::endl;
        #endif
        localvariablemanager.free(index);
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
        #ifdef pclpp_std_debug
        std::cout << "Reading word: " << std::to_string(*mem) << std::endl;
        std::cout << "Address: " << std::to_string((uint32_t)mem) << std::endl;
        #endif
        return *mem;
    }
};
