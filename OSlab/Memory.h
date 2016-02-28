//
//  Memory.h
//  OSProject1
//
//  Created by Kai Zhu on 2/20/16.
//  kxz160030@utdallas.edu
//

#ifndef Memory_h
#define Memory_h

#include "CPU.h"


namespace Project1
{
    // Memory Status
    enum M_Status{
        Memory_New=0,
        Memory_Running=1,
        Memory_Exception=2,
        Memory_End=3
    };
    //Memory Address
    const int User_Area_Start=0;
    const int User_Area_End=999;
    const int Sys_Area_Start=1000;
    const int Sys_Area_End=1999;

    const int User_Stack=1000;
    const int Sys_Stack=2000;
    //Need?
    const int Timer_Interrupt_Start=1000;
    const int Sys_Interrupt_Start=1500;



    // Declare Memory Class
    // Read_File()
    // Read_Memory()
    // Write_Memory()
    class Memory{
    public:
        
        // Max Memory Size
        const int Memory_Max;
        //Construct Memory Instance
        Memory(const int& read_pipe, const int& write_pipe, std::string filepath);
        //Release Memory Space
        ~Memory();
        //Memory status
        int Memory_current_status();
        //Memory load instructions
        void Memory_Loading();
        //Memory Run
        void Run();
        //Clear Memory Value
        void clear();
        //memory pull request from CPU
        void m_response();
        //CPU Read_Memory
        bool CPU_Read_Memory(int Address,  C_Mode CPU_Mode, int& data);
        //CPU Write_Memory
        bool CPU_Write_Memory(int Address, C_Mode CPU_Mode, int data);
        
        
        
    private:
        //memory read pipe
        int m_read_msg;
        //memory write pipe
        int m_write_msg;
        
        //Memory Initiation
        int* Memory_Space;
        //instructions filename
        std::string file_path;
        
        //Memory Status
        M_Status Memory_Status;
        //Memory Area Check
        bool Memory_Address_Validation(int address, C_Mode CPU_Mode);
        //Load value from the address in memory
        const int& Read_Memory(const int& Address);
        //Store value to the address in memory
        void Write_Memory(const int& Address, const int& Value);
        
        
    };
    
}

#endif /* Memory_h */
