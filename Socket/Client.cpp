//
//  Client.cpp
//  OS_Project_3
//
//  Network Communication Using Sockets
//
//  Created by Kaiz on 4/18/16.
//  Copyright © 2016 CS5348. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <iostream>


/* Run as:  client host port
 *
 * where host is the machine to connect to and port is its port number */


using namespace std;

#define BUFSIZE     80       /* anything large enough for your messages */

//Get Menu
void Print_Menu()
{
    printf("1: Display the names of all known users.\n");
    printf("2: Display the names of all currently connected users.\n");
    printf("3: Send a text message to a particular user.\n");
    printf("4: Send a text message to all currently connected users.\n");
    printf("5: Send a text message to all known users.\n");
    printf("6: Get my messages.\n");
    printf("7: Exit.\n");
}

void message_print(int print_format, char* output)
{
    string formatted_string;
    int n=0;
    if (print_format==0) {
        formatted_string="\n";
    }
    switch (print_format) {
        case 1:
        {formatted_string="Known Users:\n";
            char* subchar_array = strtok(output, "\\");
            while (subchar_array!=NULL) {
                formatted_string.append("\t");
                formatted_string.append(to_string(++n));
                formatted_string.append(":");
                formatted_string.append(subchar_array);
                formatted_string.append("\n");
                subchar_array = strtok(NULL, "\\");
            }
            break;
        }
        case 2:
        {formatted_string="Currently Connected Users:\n";
            char* subchar_array = strtok(output, "\\");
            while (subchar_array!=NULL) {
                formatted_string.append("\t");
                formatted_string.append(to_string(++n));
                formatted_string.append(":");
                formatted_string.append(subchar_array);
                formatted_string.append("\n");
                subchar_array = strtok(NULL, "\\");
            }
            break;
        }
        case 6:
        {
            if (strcmp(output, "NULL")==0) {
                formatted_string="You don't have messages.\n";
            }
            else
            {
                formatted_string="Your Messages Users:\n";
                char* subchar_array = strtok(output, "\\");
                while (subchar_array!=NULL) {
                    formatted_string.append("\t");
                    formatted_string.append(to_string(++n));
                    formatted_string.append(":");
                    formatted_string.append(subchar_array);
                    formatted_string.append("\n");
                    subchar_array = strtok(NULL, "\\");
                }
            }
            break;
        }
    }
    cout<<formatted_string<<endl;
}



int main(int argc, char *argv[])
{
    // Store Host Name
    char hostname[100];
    // Store Port Number
    int port;
    // User Name
    char name[100];
    // socket
    int sd;
    //server address
    struct sockaddr_in serv_i_addr;
    //Host name in the internet
    struct hostent *hp;
    
    // receive message
    char buf[BUFSIZE];
    // Send menu instruction
    char ins[BUFSIZE];
    // Send message
    char msg[BUFSIZE];

    
    int count;
    
    /* check for command line arguments */
    if (argc != 3)
    {
        printf("Missings argument!\n");
        exit(1);
    }
    
    //get Hostname
    strncpy(hostname, argv[1], sizeof(hostname));
    hostname[99] = '\0';
    //get Port
    port = atoi(argv[2]);
    
    /* create an Internet domain stream socket for client */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }
    
    /* lookup host machine information */
    if ((hp = gethostbyname(hostname)) == 0) {
        perror("Error on gethostbyname call");
        exit(1);
    }
    
    //initiate the server address with O
    memset(&serv_i_addr, 0, sizeof(serv_i_addr));
    //set each element of the server address
    serv_i_addr.sin_family = AF_INET;
    serv_i_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
    serv_i_addr.sin_port = htons(port); /* convert to network byte order */
    
    
    printf("Connecting to %s:%d\n\n", hostname, port);
    
    /* connect to port on host */
    if (connect(sd,(struct sockaddr *)  &serv_i_addr, sizeof(serv_i_addr)) == -1) {
        perror("Error on connect call");
        exit(1);
    }
    
    /* ask user for name */
    printf("Enter your name: ");
    
    std::cin.getline(name,sizeof(name));
    //fgets(name, sizeof(name), stdin);
    
    /* send the name to the server */
    if ((count = write(sd, name, strlen(name)+1)) == -1) {
        perror("Error on write call");
        exit(1);
    }

    
    /* wait for a message to come back from the server */
    if ( (count = read(sd, buf, BUFSIZE)) == -1) {
        perror("Error on read call");
        exit(1);
    }

    //Approve or deny
    //printf("\n\n%s\n\n", buf);
    if (strcmp(buf, "deny")==0) {
        close(sd);
        exit(1);
    }
    
    Print_Menu();

    while (1) {
        
        //c_output for output
        char c_output[BUFSIZE];
        memset(c_output, 0, sizeof(c_output));
        memset(ins, 0, sizeof(ins));
        memset(msg, 0, sizeof(msg));
        //Format message
        int print_format=0;
        //User Char Ins to transfer data
        printf("Enter your choice: ");
        std::cin.getline(ins,sizeof(ins));
        
        
        switch (ins[0]) {
            case '1':
                strncpy(msg, "1", 1);
                print_format=1;
                break;
            case '2':
                strncpy(msg, "2", 1);
                print_format=2;
                break;
            case '3'://Message to someone
                
                strncpy(msg, "3", sizeof(msg));
                if ( (count = write(sd, msg, strlen(msg)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }

                
                printf("Enter Recipient's name:");
                std::cin.getline(msg,sizeof(msg)); // get recipient
    
                if ( (count = write(sd, msg, strlen(msg)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }
                
                printf("Enter a message:");
                std::cin.getline(msg,sizeof(msg)); // get message

                break;
            case '4'://Message to every current connected person
                strncpy(msg, "4", sizeof(msg));
                if ( (count = write(sd, msg, strlen(msg)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }

                
                printf("Enter a message:");
                std::cin.getline(msg,sizeof(msg)); // get message
                break;
            case '5'://message to every known person
                strncpy(msg, "5", sizeof(msg));
                if ( (count = write(sd, msg, strlen(msg)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }

                
                printf("Enter a message:");
                std::cin.getline(msg,sizeof(msg)); // get message
                break;
            case '6':
                strncpy(msg, "6", 1);
                print_format=6;
                break;
            case '7':
                strncpy(msg, "7", 1);
                break;
            default:
                printf("Invalid Input!!!\n");
                continue;
                break;
        }
        //ins[BUFSIZE-1]='\0';//bind string end
        // send the instruction to server
        if ( (count = write(sd, msg, strlen(msg)+1)) == -1) {
            perror("Error on write call");
            exit(1);
        }

        //if the instruction is to exit
        if (strcmp(ins, "7")==0) {
            break;
        }
        memset(buf, 0, sizeof(buf));//clean buf
        //Read the message from server
        if ( (count = read(sd, buf, BUFSIZE-1)) == -1) {
            perror("Error on read call");
            exit(1);
        }

        buf[BUFSIZE-1]='\0'; //bind string end
        //store to c_output
        strcat(c_output, buf);
        //if last time =80?
        while (count==(BUFSIZE-1)) {
            memset(buf, 0, sizeof(buf));//clean buf
            if ( (count = read(sd, buf, BUFSIZE-1)) == -1) {
                perror("Error on read call");
                exit(1);
            }

            buf[BUFSIZE-1]='\0';  //bind string end
            strcat(c_output, buf);
        }
        /* print the received message */
        if (print_format!=0) {
            //Build output format
            printf("\n");
            message_print(print_format, c_output);
        }
        else
        {
            printf("\n%s\n",c_output);
        }
        printf("\n");
        Print_Menu();
    }
    
    /* close the socket */
    close(sd);
}



