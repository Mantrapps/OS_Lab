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
    
    //send message and receive message
    char buf[BUFSIZE];
    // User Instruction and message
    char ins[BUFSIZE];
    //???
    int count;
    
    /* check for command line arguments */
    if (argc != 3)
    {
        ///???
        printf("Usage: client host port\n");
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
    printf("Client sent %d bytes\n", count);
    
    /* wait for a message to come back from the server */
    if ( (count = read(sd, buf, BUFSIZE)) == -1) {
        perror("Error on read call");
        exit(1);
    }
    //Read IF connected
    printf("Client read %d bytes\n", count);
    /* print the received message */
    //??? Approve or Deny
    printf("\n\n%s\n\n", buf);
    
    Print_Menu();

    while (1) {
        
        //User Char Ins to transfer data
        printf("Enter your choice: ");
        std::cin.getline(ins,sizeof(ins));
        
        switch (ins[0]) {
            case '1':
                strncpy(ins, "1", 1);
                printf("\n(%s)\n", ins);
                break;
            case '2':
                strncpy(ins, "2", 1);
                break;
            case '3'://Message to someone
                
                strncpy(ins, "3", sizeof(ins));
                if ( (count = write(sd, ins, strlen(ins)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }
                printf("Client sent %d bytes\n", count);
                
                printf("Enter Recipient's name:");
                std::cin.getline(ins,sizeof(ins)); // get recipient
    
                if ( (count = write(sd, ins, strlen(ins)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }
                printf("Client sent %d bytes\n", count);
                
                printf("Enter a message:");
                std::cin.getline(ins,sizeof(ins)); // get message

                break;
            case '4'://Message to every current connected person
                strncpy(ins, "4", sizeof(ins));
                if ( (count = write(sd, ins, strlen(ins)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }
                printf("Client sent %d bytes\n", count);
                
                printf("Enter a message:");
                std::cin.getline(ins,sizeof(ins)); // get message
                break;
            case '5'://message to every known person
                strncpy(ins, "5", sizeof(ins));
                if ( (count = write(sd, ins, strlen(ins)+1)) == -1) {
                    perror("Error on write call");
                    exit(1);
                }
                printf("Client sent %d bytes\n", count);
                
                printf("Enter a message:");
                std::cin.getline(ins,sizeof(ins)); // get message
                break;
            case '6':
                strncpy(ins, "6", 1);
                break;
            case '7':
                strncpy(ins, "7", 1);
                break;
            default:
                printf("Invalid Input!!!");
                continue;
                break;
        }
        // send the instruction to server
        if ( (count = write(sd, ins, strlen(ins)+1)) == -1) {
            perror("Error on write call");
            exit(1);
        }
        printf("Client sent %d bytes\n", count);
        
        if (strcmp(ins, "7")==0) {
            break;
        }
        //Read the message from server
        if ( (count = read(sd, buf, BUFSIZE)) == -1) {
            perror("Error on read call");
            exit(1);
        }
        printf("Client read %d bytes\n", count);
        printf("\n%s\n", buf);
        
        //???
        while (count==BUFSIZE) {
            if ( (count = read(sd, buf, BUFSIZE)) == -1) {
                perror("Error on read call");
                exit(1);
            }
            printf("Client read %d bytes\n", count);
            printf("\n%s\n", buf);
        }
        /* print the received message */
        
        
        Print_Menu();
    }
    
    /* close the socket */
    close(sd);
}
//Check input


