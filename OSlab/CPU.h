//
//  CPU.h
//  OSProject1
//
//  Created by Kaiz on 2/20/16.
//  kxz160030@utdallas.edu
//

#ifndef CPU_h
#define CPU_h



namespace Project1 {
    // CPU Status, 0 = Running, 1 = End
    enum C_Status{
        CPU_Running=0,
        CPU_End=1
    };
    // CPU Mode, 0 = User Mode, 1 = Kernel Mode
    enum C_Mode{
        CPU_UserMode=0,
        CPU_KernelMode=1
    };
    // CPU Request Type
    enum Request_Type{
        CPU_Request_Read=0,
        CPU_Request_Write=1,
        CPU_Request_End=3
    };
    /*//Pipe Read Msg
    struct C_ReadMsg
    {
        int address;
    };*/
    //Pipe Write Msg
    struct C_Msg
    {
        int address;// address memory
        int data;  // Write Value
    };
    //CPU Request to Write or Read Request to Memory
    struct CPU_Request
    {
        Request_Type CPU_Request_Type;
        C_Mode CPU_Current_Mode;
        /*union
        {
            C_ReadMsg  CPU_Request_Read;
            C_WriteMsg CPU_Request_Write;
        };*/
        C_Msg CPU_Pass_data;
    };
    //Memory Response data
    struct Memory_Response
    {
        bool execute_result;
        int data;
    };
    
    class CPU
    {
    public:

        
        //read from memory, write to CPU, interrupt timer setting
        CPU(const int& read_pipe, const int& write_pipe, int interrupt_time);
        
        ~CPU();
        
        void Reset();
        
        int CPU_Current_Status();
        
        void Run();
        //Fetch Code
        void Fetch();
        //Execute Code
        void Decode();
        //Interrupt Timer
        void Interrupt_Timer();
        
        
    private:
        //cpu read pipe
        int c_read_msg;
        //cpu write pipe
        int c_write_msg;
        //CPU sent Request to Read
        bool CPU_Send_Request_Read(C_Mode CPU_Mode,int R_PC);
        //CPU sent Request to Write
        bool CPU_Send_Request_Write(C_Mode CPU_Mode,int R_PC, int value);
        //CPU get response
        bool CPU_Get_Response(int& return_val);
        // Memory reponse result message
        bool Memory_res;
        // Register Program Counter
        int Reg_PC;
        // Register Stack Pointer
        int Reg_SP;
        // Register Instruction Register
        int Reg_IR;
        // Register Accumlator
        int Reg_AC;
        // Register x
        int Reg_X;
        // Register Y
        int Reg_Y;
        //CPU Status
        C_Status CPU_Status;
        //CPU Mode
        C_Mode CPU_Mode;
        
        
        //Count execute instruction amount
        int Instruction_Counter;
        //Delayed Counter
        int Waited_Timer;
        //User input interrupt time
        int Interrupt_Time;
        
        
    /*-----------------Instruction Set-----------------*/
        //1
        void Load_Value(void);
        //2
        void Load_addr(void);
        //3
        void LoadInd_addr(void);
        //4
        void LoadIdxX_addr(void);
        //5
        void LoadIdxY_addr(void);
        //6
        void LoadSpX(void);
        //7
        void Store_addr(void);
        //8
        void Get(void);
        //9
        void Put_Port(void);
        //10
        void AddX(void);
        //11
        void AddY(void);
        //12
        void SubX(void);
        //13
        void SubY(void);
        //14
        void CopyToX(void);
        //15
        void CopyFromX(void);
        //16
        void CopyToY(void);
        //17
        void CopyFromY(void);
        //18
        void CopyToSp(void);
        //19
        void CopyFromSp(void);
        //20
        void Jump_addr(void);
        //21
        void JumpIfEqual_addr(void);
        //22
        void JumpIfNotEqual_addr(void);
        //23
        void Call_addr(void);
        //24
        void Ret(void);
        //25
        void IncX(void);
        //26
        void DecX(void);
        //27 Push value into stack
        void Push(void);
        //28 Pop value from the stack
        void Pop(void);
        //29
        void Int(const int&C_Modeupt_address, const int& return_address, const C_Mode& cpu_m);
        //30
        void IRet(void);
        // get a random number
        int random();

        //50
        void End();
    };
}


#endif /* CPU_h */
