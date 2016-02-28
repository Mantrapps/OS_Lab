//
//  Memory.cpp
//  OSProject1
//
//  Created by Kaiz on 2/20/16.
//  kxz160030@utdallas.edu
//

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include <fstream>
#include <unistd.h>
#include <regex>
#include "Memory.h"

namespace Project1 {
    
    //Pipe Write Msg
    struct M_Msg
    {
        int address;// address memory
        int data;   //
    };
    //CPU Request to Write to Memory
    struct Memory_Receive_Request
    {
        Request_Type CPU_Request;
        C_Mode CPU_Current_Mode;
        M_Msg Msg;
    };
    //Memory Response data
    struct Memory_Response_Msg
    {
        bool execute_result;
        int data;
    };
    //Host Overboard warning
    std::string override;
    //initiation memory with 2000 size
    Memory::Memory(const int& read_pipe, const int& write_pipe, std::string filepath):Memory_Max(2000),file_path(filepath), Memory_Space(new int[Memory_Max]), Memory_Status(Memory_New), m_read_msg(read_pipe), m_write_msg(write_pipe)
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
        for (int i=0; i< Memory_Max; i++)
        {
            Memory_Space[i]=0;
        }
    }
    void Memory::Memory_Loading()
    {
        
        std::ifstream ifs(file_path);
        std::string tmp;
        int Memory_address=User_Area_Start;
        if(ifs.is_open())
        {
            while(getline(ifs, tmp))
            {
                std::regex pattern("[0-9.]+");
                std::regex pattern2("[0-9]+");
                std::smatch result;
                std::smatch result2;
                bool match=regex_search(tmp, result, pattern);
                if (match) {
                    std::string tmp2;
                    tmp2=result.str();
                    if (tmp2[0]=='.') {
                        bool match2=regex_search(tmp2, result2, pattern2);
                        if(match2){
                        Memory_address=stoi(result2.str());
                        ///???
                        std::cout<<"change address to "<<result2.str()<<std::endl;
                        }
                    }
                    else{
                        Memory_Space[Memory_address]=stoi(result.str());
                        //???
                        std::cout<<result.str()<<" at: "<<Memory_address<<std::endl;
                        Memory_address++;
                    }
                }
            }
            Memory_Status=Memory_Running;
            ifs.close();
        }
        else
        {
            ///????
            ifs.close();
            std::cout<<"The file you input not exist or not supported!"<<std::endl;
            Memory_Status=Memory_End;
        }
        
    }
    // user mode cannot access system area
    const int& Memory::Read_Memory(const int& address)
    {
        return Memory_Space[address];
    }
    // user mode cannot access system area
    void Memory::Write_Memory(const int& address, const int& Value)
    {
        Memory_Space[address]=Value;
    }
    int Memory::Memory_current_status()
    {
        return Memory_Status;
    }
    bool Memory::Memory_Address_Validation(int address, Project1::C_Mode cpu_mode)
    {
        if ((cpu_mode!=CPU_UserMode)&&(address>=User_Area_Start&&address<=User_Area_End)) {
            override="Kernal Mode";
            return false;
        }
        else if (((cpu_mode!=CPU_KernelMode)&&(address>=Sys_Area_Start&&address<=Sys_Area_End)))
        {
            override="User Mode";
            return false;
        }
        else{
            return true;
        }
    }
    bool Memory::CPU_Read_Memory(int Address, C_Mode CPU_Mode, int& data)
    {

        if (Memory_Address_Validation(Address, CPU_Mode)) {
            
            data=Memory_Space[Address];
            return true;
        }
        else{
            std::cout<<"Memory(Read) Violation: "<<" accessing system address "<<" at "<<Address<< " in "<<override<<std::endl;
            return false;
        }
    }
    //???
    bool Memory::CPU_Write_Memory(int Address, C_Mode CPU_Mode, int data)
    {
        if (Memory_Address_Validation(Address, CPU_Mode)) {
            Memory_Space[Address]=data;
            return true;
        }
        else{
            std::cout<<"Memory(Write) Violation: "<<" accessing system address "<<" at "<<Address<< " in "<<override<<std::endl;
            return false;
        }
    }
    void Memory::m_response()
    {
        
        if (Memory_Status!=3) {
            Request_Type CPU_Request_T;
            int *data;
            Memory_Receive_Request Request_Msg;
            ssize_t result=read(m_read_msg, &Request_Msg, sizeof(Request_Msg));
            if (result==-1) {
                perror("Memory cannot get CPU's request!");
            }
            else{
                CPU_Request_T=Request_Msg.CPU_Request;
                data=&Request_Msg.Msg.data;
                if (CPU_Request_T==CPU_Request_Read)
                {
                    int val;
                    if(CPU_Read_Memory(Request_Msg.Msg.address, Request_Msg.CPU_Current_Mode, val))
                    {
                        Memory_Response_Msg MSG={true,val};
                        write(m_write_msg, &MSG, sizeof(MSG));
                    }
                    else
                    {
                        std::cout<<"CPU Read Memory failed!"<<std::endl;
                        Memory_Status=Memory_End;
                    }
                    
                }
                else if(CPU_Request_T==CPU_Request_Write)
                {
                    //std::cout<<"wola Write"<<std::endl;

                    if(CPU_Write_Memory(Request_Msg.Msg.address, Request_Msg.CPU_Current_Mode, Request_Msg.Msg.data))
                    {
                        //std::cout<<"Write "<<Request_Msg.Msg.address<<" What "<<Request_Msg.Msg.data<<std::endl;
                        Memory_Response_Msg MSG={true,Request_Msg.Msg.data};
                        write(m_write_msg, &MSG, sizeof(MSG));
                    }
                    else
                    {
                        std::cout<<"CPU Write to Memory failed!"<<std::endl;
                        Memory_Status=Memory_End;
                    }
                }
                else{
                    //std::cout<<"wola Nothing"<<std::endl;

                    Memory_Response_Msg MSG={false,Request_Msg.Msg.data};
                    write(m_write_msg, &MSG, sizeof(MSG));
                }
                
            }
        }
        else
        {
            
            std::cout<<"Memory is ending!!"<<std::endl;
        }
        ///??? User mode or system mode
    }
    void Memory::Run()
    {
        std::cout<<"Begin Memory Run recyle "<<Memory_Status<<std::endl;

        
        while (Memory_Status!=3) {
            m_response();
        }
        std::cout<<"Inside Memory Run recyle "<<Memory_Status<<std::endl;

        
    }

    
}
