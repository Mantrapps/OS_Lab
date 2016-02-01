//
//  main.cpp
//  OSlab
//
//  Created by Kaiz on 2/1/16.
//  Copyright © 2016 Mantrapps. All rights reserved.
//

#include <iostream>
#include <unistd.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("--beginning of program\n");
    
    int counter = 0;
    pid_t pid = fork();
    
    if (pid == 0)
    {
        // child process
        int i = 0;
        for (; i < 5; ++i)
        {
            printf("child process: counter=%d\n", ++counter);
        }
    }
    else if (pid > 0)
    {
        // parent process
        int j = 0;
        for (; j < 5; ++j)
        {
            printf("parent process: counter=%d\n", ++counter);
        }
    }
    else
    {
        // fork failed
        printf("fork() failed!\n");
        return 1;
    }
    
    printf("--end of program--\n");
    
    return 0;
}
