//
//  Memory.h
//  OSProject1
//
//  Created by Kai Zhu on 2/20/16.
//  kxz160030@utdallas.edu
//

#ifndef Memory_h
#define Memory_h

namespace Project1
{
    typedef unsigned char byte;
    class Memory{
    public:
        // Max Memory Size
        const int Memory_Max;
        
        //Construct Memory Instance
        Memory();
        
        //Release Memory Space
        ~Memory();
        
        //Clear Memory Value
        void clear();
        
        //Load value from the address in memory
        const byte& Read_Memory(const byte& Address);
        
        //Store value to the address in memory
        void Write_Memory(const byte& Address, const byte& Value);
    private:
        byte* Memory_Space;
        
    };
    
}

#endif /* Memory_h */
