//
//  Memory.cpp
//  OSProject1
//
//  Created by Kaiz on 2/20/16.
//  kxz160030@utdallas.edu
//

#include <stdio.h>
#include "Memory.h"

namespace Project1 {
    
    Memory::Memory():Memory_Max(20), Memory_Space(new char[Memory_Max])
    {
        clear();
    }
    Memory::~Memory()
    {
        if (Memory_Space!=nullptr)
        {
            delete [] Memory_Space;
            Memory_Space=nullptr;
        }
    }
    void Memory::clear()
    {
        for (char i=0; i< Memory_Max; i++)
        {
            Memory_Space[i]=0;
        }
    }
    const char& Memory::Read_Memory(const char& address)
    {
        return Memory_Space[address];
    }
    
    void Memory::Write_Memory(const char& address, const char& Value)
    {
        Memory_Space[address]=Value;
    }
    
    
}
