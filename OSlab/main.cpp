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
#include <stdlib.h>
#include "Memory.h"

using namespace std;
using namespace Project1;

int main()
{
    cout<< "Start Initiate Memory..."<<endl;
    Memory* new_Memory=new Memory();
    cout<<"Memory is Ready"<<endl;
    cout<< "Memory size is:" << new_Memory->Memory_Max<<endl;
    delete new_Memory;
}