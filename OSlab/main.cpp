//
//  main.cpp
//  OSProject-1
//  Multi-Process
//
//  Created by Kai Zhu on 2/1/16.
//  Email:kxz160030@utdallas.edu
//

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstdio>
//temply used for test
#include <fstream>
#include <regex>

#include "Memory.h"
#include "CPU.h"

using namespace std;
using namespace Project1;




int main(int argc, char* argv[])
{
    // Child process id, result
    pid_t pid, pid_result;
    // Child status
    int status;
    // File Path to load test case
    string file_path;
    // Timer parameter
    int interrupt_time=0;
    
    if (argc==2) {
        file_path=argv[1];
        cout<<argv[1]<<" prepare to load into memory!"<<endl;
        cout<<"No time interrupt parameter!"<<endl;
        cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
        
    }
    else if (argc==3) {
        file_path=argv[1];
        interrupt_time=stoi(argv[2]);
        cout<<argv[1]<<" prepare to load into memory!"<<endl;
        cout<<"Set time interrupt parameter="<<interrupt_time<<endl;
        cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<endl;
    }
    else
    {
        cout<<"Please indicate the sample file name."<<endl;
        return 1;
    }
    // Pipe for CPU, 0 for read, 1 for write
    int cpu_pipe[2];
    // Pipe for Memory, 0 for read, 1 for write
    int memory_pipe[2];
    // Open pipe
    if (pipe(cpu_pipe)==-1||pipe(memory_pipe)==-1)
    {
        perror("pipe call error");
        exit(EXIT_FAILURE);
    }
    
    pid=fork();
    if (pid==-1)
    {
        perror("error: fork call");
        exit(EXIT_FAILURE);
    }
    else if (pid==0)
    {
        //close cpu write
        close(cpu_pipe[1]);
        //close memory read
        close(memory_pipe[0]);
        //Memory Initiation
        printf("Now Memory Initiation!\n");
        //Memory Initiation
        Memory new_Memory(cpu_pipe[0],memory_pipe[1], file_path);
        new_Memory.Memory_Loading();
        while (new_Memory.Memory_current_status()!=Memory_End) {
            new_Memory.m_response();
        }
        //close cpu read
        close(cpu_pipe[0]);
        //close memory write
        close(memory_pipe[1]);
        _exit(0);
    }
    else
    {
        //close cpu read
        close(cpu_pipe[0]);
        //close memory write
        close(memory_pipe[1]);
        //CPU Print Pid  Need delete
        printf("Parent: child's pid is %d\n",pid);
        //??
        int count=200;
        //CPU Initiation
        CPU new_CPU(memory_pipe[0],cpu_pipe[1], interrupt_time);
        while (new_CPU.CPU_Current_Status()!=CPU_End) {
            
            new_CPU.Fetch();
            new_CPU.Decode();
            new_CPU.Interrupt_Timer();
            ///??
            //new_CPU.Fetch();
            //new_CPU.Decode();
            //new_CPU.Interrupt_Timer();
        }
        cout<<"CPU RUNNING "<<count<<" times"<<endl;
        //???
        //exit(1);
        //close cpu write
        close(cpu_pipe[1]);
        //close memory read
        close(memory_pipe[0]);
        // wait for a child process to terminate or stop
        pid_result=waitpid(pid, &status, 0);
            
        if (pid_result==-1)
        {
            if(errno==ECHILD)
            {
                cout<< "Child does not exist"<<endl;
            }
            else
            {
                cout<<"Bad argument passed to waitpid"<<endl;
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            //perror("waitpid");
            //exit(EXIT_FAILURE);
            }
            else
            {
            //capture process completion status
            if (WIFEXITED(status))
            {
                cout << "Child exited with status " << WEXITSTATUS(status) << endl;
            }
            else if(WIFSIGNALED(status))
            {
                cout <<"Child killed via signal "<<WTERMSIG(status)<<endl;
            }
            else if(WIFSTOPPED(status))
            {
                cout<<"Child stopped via signal"<< WSTOPSIG(status)<<endl;
            }//Need continue??
            else {
                cout<<"Unexpected status " <<status<<endl;
            }
            
            }
    }

    
    
    return 0;
    
    
   /*
    cout<< "Start Initiate Memory..."<<endl;
    Memory* new_Memory=new Memory();
    cout<<"Memory is Ready"<<endl;
    cout<< "Memory size is:" << (int)new_Memory->Memory_Max<<endl;
    new_Memory->Write_Memory(0,1);
    new_Memory->Write_Memory(1,51);
    new_Memory->Write_Memory(2,3);
    new_Memory->Write_Memory(3,50);
    
    cout << "Creating the CPU instance...";
    CPU* new_CPU = new CPU(new_Memory);
    cout << "Ready" << endl;
    
    cout << "Starting..." << endl;
    new_CPU->Run();
    cout << "Complete" << endl;
    
    // We must delete our CPU before the memory, because
    // the CPU uses the memory, and the CPU constructor
    // breaks the link to the memory when its "~CPU" function
    // sets "m-TheMemory = nullptr;"
    for (int current=0; current<new_Memory->Memory_Max; current++) {
        cout<<"Address ["<<(int)current<<"]= "<<(int)new_Memory->Read_Memory(current)<<endl;
    }
    
    delete new_CPU;
    
    delete new_Memory;
    */
}