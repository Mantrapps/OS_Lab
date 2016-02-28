//
//  CPU.cpp
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

#include "CPU.h"
#include "Memory.h"


namespace Project1 {
    
    

    //read from memory, write to CPU, interrupt timer setting
    CPU::CPU(const int& read_pipe, const int& write_pipe, int interrupt_time):
    Reg_AC(0),
    Reg_X(0),
    Reg_Y(0),
    Reg_SP(User_Stack),
    Reg_IR(0),
    Reg_PC(0),
    CPU_Mode(CPU_UserMode),
    CPU_Status(CPU_Running),
    c_read_msg(read_pipe),
    c_write_msg(write_pipe),
    Interrupt_Time(interrupt_time),
    Waited_Timer(0),
    Instruction_Counter(0)
    {}
    
    CPU::~CPU(){}
    
    void CPU::Reset()
    {
        CPU_Status=CPU_Running;
        Reg_PC=0;
    }
    void CPU::Fetch()
    {
        
        CPU_Send_Request_Read(CPU_Mode, Reg_PC);
        CPU_Get_Response(Reg_IR);
        Reg_PC++;
        //std::cout<<"Fetch Complete: PC "<<Reg_PC<<" IR: "<<Reg_IR<<" AC: "<<Reg_AC<<" sp: "<<Reg_SP<<" x: "<<Reg_X<<" y: "<<Reg_Y<<" "<<CPU_Mode<<std::endl;
        // need watch out if there is over bound to Sys mode
    }
    
    
    /*
    
    */
    // work as control unit in CPU
    void CPU::Decode()
    {
        if (CPU_Status==CPU_End) return;
        switch(Reg_IR)
        {
                //load value load the value into the AC
            case 1:
                Load_Value();
                break;
                //load addr load the value at the address into the AC
            case 2:
                Load_addr();
                break;
                //LoadInd addr
            case 3:
                LoadInd_addr();
                break;
                //loadIdxX addr
            case 4:
                LoadIdxX_addr();
                break;
                //LoadIdxY addr
            case 5:
                LoadIdxY_addr();
                break;
                //LoadSpX
            case 6:
                LoadSpX();
                break;
                //Store addr
            case 7:
                Store_addr();
                break;
                // get a random int from 1 to 100 into AC
            case 8:
                Get();
                break;
                //Put port
            case 9:
                Put_Port();
                break;
                //addX
            case 10:
                AddX();
                break;
                //add Y
            case 11:
                AddY();
                break;
                //Sub X
            case 12:
                SubX();
                break;
                //Sub y
            case 13:
                SubY();
                break;
                //copy the value in the AC to X
            case 14:
                CopyToX();
                break;
                //copy the value in X to the AC
            case 15:
                CopyFromX();
                break;
                //copy the value in the AC to Y
            case 16:
                CopyToY();
                break;
                //copy the value in Y to the AC
            case 17:
                CopyFromY();
                break;
                //copy the value in SP to the AC
            case 18:
                CopyToSp();
                break;
                // Copy From Sp
            case 19:
                CopyFromSp();
                break;
                //Jump addr
            case 20:
                Jump_addr();
                break;
                //Jump If equal addr
            case 21:
                JumpIfEqual_addr();
                break;
                //Jump If not equal addr
            case 22:
                JumpIfNotEqual_addr();
                break;
                //call addr
            case 23:
                Call_addr();
                break;
                //ret
            case 24:
                Ret();
                break;
                //inc X
            case 25:
                IncX();
                break;
                //Dec X
            case 26:
                DecX();
                break;
                //Push AC onto Stack
            case 27:
                Push();
                //push();
                break;
                //Pop from stack into AC
            case 28:
                Pop();
                //pop();
                break;
                //Int
            case 29:
                Int(Sys_Interrupt_Start, Reg_PC, CPU_KernelMode);
                break;
                //IRet
            case 30:
                IRet();
                break;
                //End Execution
            case 50:
                End();
                break;
        }
        if (CPU_Status==CPU_End) {
            return;
        }
        ++Instruction_Counter;
        //std::cout<<"Decode Complete: PC "<<Reg_PC<<" IR: "<<Reg_IR<<" AC: "<<Reg_AC<<" sp: "<<Reg_SP<<" x: "<<Reg_X<<" y: "<<Reg_Y<<std::endl;
        //std::cout<<std::endl;

    }
    // Interrupt Timer Monitor
    void CPU::Interrupt_Timer()
    {
        if (CPU_Status==CPU_End)
        {
            return;
        }
        if (Interrupt_Time==0) {
            return;
        }
        if (CPU_Mode==CPU_UserMode && Waited_Timer>0) {
            Int(Timer_Interrupt_Start,Reg_PC,CPU_KernelMode);
            --Waited_Timer;
        }
        if (!(Instruction_Counter % Interrupt_Time))
        {
            if (CPU_Mode==CPU_UserMode) {
                Int(Timer_Interrupt_Start,Reg_PC,CPU_KernelMode);
            }
            else
            {
                Waited_Timer++;
            }
        }
        
    }
    //CPU Sent Request to Read
    bool CPU::CPU_Send_Request_Read(C_Mode CPU_Mode,int R_PC)
    {
        if (CPU_Status==CPU_End) {
            return false;
        }
        else{
            C_Msg CPU_Read_Msg;
            CPU_Read_Msg.address=R_PC;
            //Request Type CPU_Request_Read
            CPU_Request CPU_Request_Msg={CPU_Request_Read,CPU_Mode,CPU_Read_Msg};
            ssize_t result=write(c_write_msg, &CPU_Request_Msg, sizeof(CPU_Request_Msg));
        if (result==-1) {
            perror("CPU Send Request to read memory failed!");
            
            return false;
        }
        return true;
        }
    }
    //CPU Sent Request to Write
    bool CPU::CPU_Send_Request_Write(C_Mode CPU_Mode,int R_PC,int value)
    {
        C_Msg CPU_Write_Msg;
        CPU_Write_Msg.address=R_PC;
        CPU_Write_Msg.data=value;
        //Request Type CPU_Request_Write
        CPU_Request CPU_Request_Msg={CPU_Request_Write,CPU_Mode,CPU_Write_Msg};
        ssize_t result=write(c_write_msg, &CPU_Request_Msg, sizeof(CPU_Request_Msg));
        if (result==-1) {
            perror("CPU Send Request to write memory failed");
            return false;
        }
        return true;
    }
    //CPU Get Response from Memory
    bool CPU::CPU_Get_Response(int& return_val)
    {
        Memory_Response Memory_Response_CPU;
        ssize_t result=read(c_read_msg, &Memory_Response_CPU, sizeof(Memory_Response_CPU));
        if (result==-1) {
            perror("CPU get response from memory failed");
            return false;
        }
        Memory_res=Memory_Response_CPU.execute_result;
        return_val=Memory_Response_CPU.data;
        return true;
    }
    
    //CPU Status
    int CPU::CPU_Current_Status()
    {
        return CPU_Status;
    }
    /*void CPU::Run()
    {
        while (CPU_Status==CPU_Running) {
            //store the instruction in IR
            Fetch();
            Decode();
            Interrupt_Timer();
        }
    }*/

    /*-----------------Instruction Set-----------------*/
    
    //1
    void CPU::Load_Value()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(Reg_AC);
    }
    //2
    void CPU::Load_addr()
    {
        int address;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(address);
        CPU_Send_Request_Read(CPU_Mode, address);
        CPU_Get_Response(Reg_AC);
    }
    //3
    void CPU::LoadInd_addr()
    {
        int address;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(address);
        CPU_Send_Request_Read(CPU_Mode, address);
        CPU_Get_Response(address);
        CPU_Send_Request_Read(CPU_Mode, address);
        CPU_Get_Response(Reg_AC);

    }
    //4
    void CPU::LoadIdxX_addr()
    {
        int value;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(value);
        CPU_Send_Request_Read(CPU_Mode, value+Reg_X);
        CPU_Get_Response(Reg_AC);
    }
    //5
    void CPU::LoadIdxY_addr()
    {
        int value;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(value);
        CPU_Send_Request_Read(CPU_Mode, value+Reg_Y);
        CPU_Get_Response(Reg_AC);
        
    }
    //6
    void CPU::LoadSpX()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_SP+Reg_X);
        CPU_Get_Response(Reg_AC);
    }
    //7
    void CPU::Store_addr()
    {
        int address;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(address);
        int tmpval;
        CPU_Send_Request_Write(CPU_Mode, address, Reg_AC);
        CPU_Get_Response(tmpval);
    }
    //8
    void CPU::Get()
    {
        Reg_AC=random();
    }
    //9
    void CPU::Put_Port()
    {
        int port;
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(port);
        if (port==1) {
            std::cout<<Reg_AC;
        }
        else if (port==2)
        {
            std::cout<<static_cast<char>(Reg_AC);
        }
    }
    //10
    void CPU::AddX()
    {
        Reg_AC=Reg_AC+Reg_X;
        
    }
    //11
    void CPU::AddY()
    {
        Reg_AC=Reg_AC+Reg_Y;
        
    }
    //12
    void CPU::SubX()
    {
        Reg_AC=Reg_AC-Reg_X;
    }
    //13
    void CPU::SubY()
    {
        Reg_AC=Reg_AC-Reg_Y;
    }
    //14
    void CPU::CopyToX()
    {
        Reg_X=Reg_AC;
    }
    //15
    void CPU::CopyFromX()
    {
        Reg_AC=Reg_X;
    }
    //16
    void CPU::CopyToY()
    {
        Reg_Y=Reg_AC;
    }
    //17
    void CPU::CopyFromY()
    {
        Reg_AC=Reg_Y;
    }
    //18
    void CPU::CopyToSp()
    {
        Reg_SP=Reg_AC;
    }
    //19
    void CPU::CopyFromSp()
    {
        Reg_AC=Reg_SP;
    }
    //20
    void CPU::Jump_addr()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
        CPU_Get_Response(Reg_PC);
    }
    //21
    void CPU::JumpIfEqual_addr()
    {
        if (Reg_AC==0) {
            CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
            CPU_Get_Response(Reg_PC);
        }
        else{
            Reg_PC++;
        }
        
    }
    //22
    void CPU::JumpIfNotEqual_addr()
    {
        if (Reg_AC!=0) {
            CPU_Send_Request_Read(CPU_Mode, Reg_PC++);
            CPU_Get_Response(Reg_PC);
        }
        else
        {
            Reg_PC++;
        }
    }
    //23 
    void CPU::Call_addr()
    {
        int tmpval;
        CPU_Send_Request_Write(CPU_Mode, --Reg_SP, Reg_PC+1);
        CPU_Get_Response(tmpval);
        Jump_addr();
    }
    //24
    void CPU::Ret()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_SP++);
        CPU_Get_Response(Reg_PC);
    }
    //25
    void CPU::IncX()
    {
        ++Reg_X;
    }
    //26
    void CPU::DecX()
    {
        --Reg_X;
    }
    //27
    void CPU::Push()
    {
        int tmpval;
        CPU_Send_Request_Write(CPU_Mode, --Reg_SP, Reg_AC);
        CPU_Get_Response(tmpval);
    }
    //28
    void CPU::Pop()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_SP++);
        CPU_Get_Response(Reg_AC);
        
    }
    //29
    void CPU::Int(const int& interrupt_Address, const int& return_address, const C_Mode& cpu_m)
    {
        CPU_Mode=cpu_m;
        int user_mode_sp=Reg_SP;
        Reg_SP=Sys_Stack;
        int tmpval;
        CPU_Send_Request_Write(CPU_Mode, --Reg_SP, user_mode_sp);
        CPU_Get_Response(tmpval);
        CPU_Send_Request_Write(CPU_Mode, --Reg_SP, return_address);
        CPU_Get_Response(tmpval);
        Reg_PC=interrupt_Address;

    }
    //30
    void CPU::IRet()
    {
        CPU_Send_Request_Read(CPU_Mode, Reg_SP++);
        CPU_Get_Response(Reg_PC);
        CPU_Send_Request_Read(CPU_Mode, Reg_SP++);
        CPU_Get_Response(Reg_SP);
        CPU_Mode=CPU_UserMode;
    }
    //50
    void CPU::End()
    {
        CPU_Status=CPU_End;
        //CPU_Send_Request_Read(CPU_Mode, 0);
        //???
        std::cout<<std::endl;
        std::cout<<"The program execution is end."<<std::endl;

    }
    
    int CPU::random()
    {
        int i=rand()%100 +1;
        return i;
    }
    
    
}
