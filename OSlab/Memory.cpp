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
    
    Memory::Memory():Memory_Max(2000), Memory_Space(new byte[Memory_Max])
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
        for (byte i=0; i< Memory_Max; i++)
        {
            Memory_Space[i]=0;
        }
    }
    const byte& Memory::Read_Memory(const byte& address)
    {
        return Memory_Space[address];
    }
    
    void Memory::Write_Memory(const byte& address, const byte& Value)
    {
        Memory_Space[address]=Value;
    }
    
    
}
