//
//  Server.cpp
//  OS_Project_3
//
//  Network Communication Using Sockets
//
//  Created by Kaiz on 4/18/16.
//  Copyright © 2016 CS5348. All rights reserved.
//
// Max 100 users
// Each user max 10 message
// Each message 80 characters

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <iostream>

#define BUFSIZE     80       /* anything large enough for your messages */
#define Max_Known_Users 100   // Maximum of 100 known users
#define Max_User_Msg_History 10 //Maximum of 10 messages each users


//semaphore Connect_id, 1
static sem_t Connect_id;
//semaphore Visit_Server_Data, 1
static sem_t Visit_Server_Data;
// connection amount
int connection_number=0;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

struct str_client
{
    // Connection Name
    std::string name;
    // Known Before
    bool known;
    // Currently Connected
    bool connected;
    //
    int message_amount;
    // Message history  ???
    std::string message_history[Max_User_Msg_History];
};

void* handleClient(void *);

std::string get_time_now()
{
    time_t time_basis;
    struct tm * timeinfo;
    char now_time [80];
    time (&time_basis);
    timeinfo = localtime (&time_basis);
    strftime (now_time, 80,"%D %I:%M %p",timeinfo);
    return now_time;
};

class Server_DB{
    
private:
    //str
    str_client stc_clients[Max_Known_Users];
    // how to make sure char 80
    
public:
    Server_DB()
    {
        for (int i=0; i<Max_Known_Users; i++) {
            stc_clients[i].name="";
            stc_clients[i].known=false;
            stc_clients[i].connected=false;
            stc_clients[i].message_amount=0;
        }
    }
    // Connecting
    void connect(std::string name)
    {
        //known User
        if (get_id(name)!=-1) {
            stc_clients[get_id(name)].connected=true;
            printf("%s, Connection by known user %s\n",get_time_now().c_str(),name.c_str());
        }
        else
        {
            //new array id
            //Client Name
            stc_clients[connection_number].name=name;
            if (!stc_clients[connection_number].known)
            {
                stc_clients[connection_number].known=true;
            }
            stc_clients[connection_number].connected=true;
            printf("%s, Connection by unknown user %s\n",get_time_now().c_str(),name.c_str());
            connection_number++;
        }
    }

    //Menu-1 Display the names of all known users
    std::string display_all_known_users()
    {
        std::string message;
        message="Known users:\n";
        //??? serial no
        int serial=1;
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].known) {
                message.append(std::to_string(serial)+":");
                message.append(stc_clients[i].name.c_str());
                message.append("\n");
                serial++;
            }
        }
        return message;
    }
    //Menu-2 Display the names of all currently connected users
    std::string display_all_connecting_users()
    {
        std::string message;
        message="Currently Connected users:\n";
        //??? serial no
        int serial=1;
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].connected) {
                message.append(std::to_string(serial)+":");
                message.append(stc_clients[i].name.c_str());
                message.append("\n");
                serial++;
            }
        }
        return message;
    }
    //Menu-3 Send a text message to a particular user
    std::string message_to (std::string from, std::string to, std::string msg)
    {
        std::string message;
        //??? check no exist user
        int To_id=get_id(to);
        if(To_id==-1)
        {
            //make unknown to known
            stc_clients[connection_number].name=to;
            stc_clients[connection_number].known=true;
            message="Message posted to "+to;
            std::string time_now=get_time_now();
            //Add amount
            stc_clients[connection_number].message_amount++;
            msg="From "+from+", "+time_now+", "+msg;
            //Add message
            stc_clients[connection_number].message_history[0]=msg;
            //message="Recipient Not Exist!";
            connection_number++;
            return message;
        }
        else
        {
            
            int index=stc_clients[To_id].message_amount+1;
            //Check if over Max_User_Msg_History
            if (index>Max_User_Msg_History) {
                message="Recipient: "+to+" Can not accept Message!";
                return message;
            }
            else{
                message="Message posted to "+to;
                std::string time_now=get_time_now();
                //Add amount
                stc_clients[To_id].message_amount=index;
                msg="From "+from+", "+time_now+", "+msg;
                //Add message
                stc_clients[To_id].message_history[--index]=msg;
                return message;
            }
        }
    }
    //Menu-4 Send a text message to all currently connected users ??? (Ignore all full message users)
    std::string message_to_connecting_users(std::string from, std::string msg)
    {
        
        std::string message;
        message="Message posted to all currently connected users";
        std::string time_now=get_time_now();
        msg="From "+from+", "+time_now+", "+msg;
        //printf("MSG:(%s)\n",msg.c_str());
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].connected)
            {
                int index=stc_clients[i].message_amount+1;
                if (index<Max_User_Msg_History)
                {
                    //Add amount
                    stc_clients[i].message_amount=index;
                    //Add message
                    stc_clients[i].message_history[--index]=msg;
                }
            }
        }
        return message;
    }
    //Menu-5 Send a text message to all known users ???(Ignore all full message users)
    std::string message_to_known_users(std::string from, std::string msg)
    {
        std::string message;
        message="Message posted to all known users";
        std::string time_now=get_time_now();
        msg="From "+from+", "+time_now+", "+msg;
       
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].known)
            {
                int index=stc_clients[i].message_amount+1;
                if (index<Max_User_Msg_History) {
                    //Add amount
                    stc_clients[i].message_amount=index;
                    //Add message
                    stc_clients[i].message_history[--index]=msg;
                }
            }
        }
        return message;
    }
    //Menu-6 Get My Messages
    std::string Get_my_messages(std::string name)
    {
        std::string message;
        int serial=1;
        //who
        int id=get_id(name);
        if (stc_clients[id].message_amount==0) {
            message="You don't have messages.\n";
        }
        else{
            message="Your messages:\n";
            for (int i=0; i<stc_clients[id].message_amount; i++)
            {
                message.append(std::to_string(serial)+":");
                message.append(stc_clients[id].message_history[i]);
                message.append("\n");
                serial++;
            }
            //after retrivel,clean the message
            for (int i=0; i<stc_clients[id].message_amount; i++)
            {
                stc_clients[id].message_history[i].clear();
            }
            stc_clients[id].message_amount=0;
        }
        
        return  message;
    }
    //duplicate check
    bool access_check(std::string name)
    {
        int array_id;
        array_id=get_id(name);
        //if not exist this user name
        if (array_id==-1) {
            return true;
        }
        //if exist this user name and connected
        else if(stc_clients[array_id].connected)
        {
            return false;
        }
        else return true;
    }
    //disconnect

    void disconnect(std::string name)
    {
        stc_clients[get_id(name)].connected=false;
    }
    
    //get id by name
    int get_id(std::string name)
    {
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].name==name) {
                return i;
            }
        }
        return -1;
    }
};

//Initiate DB in server
Server_DB s_db;

int main(int argc, char *argv[])
{
    // Local Host Name
    char  host[80];
    // Socket for Server
    int sd;
    // Current connected socket (new)
    int sd_current;
    // Port
    int port;
    // Client
    int *sd_client;
    //length of client internet address
    int addrlen;
    //server internet address
    struct sockaddr_in serv_i_addr;
    //client internet address
    struct sockaddr_in cli_i_addr;
    
    // ???
    pthread_t tid;
    pthread_attr_t attr;
    
    //Initialize semaphore assign Connect_id
    if(sem_init(&Connect_id, 0, 1)==-1)
    {
        perror("sem_init Connect_id");
        exit(EXIT_FAILURE);
    }
    //Initialize semaphore assign Connect_id
    if(sem_init(&Visit_Server_Data, 0, 1)==-1)
    {
        perror("sem_init Visit_Server_Data");
        exit(EXIT_FAILURE);
    }
    
    
    /* check for command line arguments */
    if (argc != 2)
    {
        printf("Usage: server port\n");
        exit(1);
    }
    /* get port from argv */
    port = atoi(argv[1]);
    
    //Step-1
    /* create an internet domain stream socket for server */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(1);
    }
    
    //initialize value in serv_i_addr to 0.
    memset(&serv_i_addr, 0, sizeof(serv_i_addr));
    //set elements in serv_i_addr
    serv_i_addr.sin_family = AF_INET;          // the code for address family
    serv_i_addr.sin_addr.s_addr = INADDR_ANY;  // the IP address of this machine
    serv_i_addr.sin_port = htons(port);        // convert to network byte order
    
    //Step-2
    /* bind the socket to the address and port number */
    if (bind(sd, (struct sockaddr *) &serv_i_addr, sizeof(serv_i_addr)) == -1) {
        perror("Error on bind call");
        exit(1);
    }
    
    //Step-3 Listen
    /* set queue size of pending connections */
    if (listen(sd, 5) == -1) {
        perror("Error on listen call");
        exit(1);
    }
    
    /* announce server is running */
    gethostname(host, 80);
    printf("Server is running on %s:%d\n", host, port);
    
    
    /* wait for a client to connect */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); /* use detached threads */
    
    addrlen = sizeof(cli_i_addr);
    
    while (1)
    {
        if ((sd_current = accept(sd, (struct sockaddr *)  &cli_i_addr, (socklen_t*)&addrlen)) == -1)
        {
            perror("Error on accept call");
            exit(1);
        }
        
        sd_client = (int*)(malloc(sizeof(sd_current)));
        *sd_client = sd_current;
        
        //create pthread for the connected client
        pthread_create(&tid, &attr, handleClient, sd_client);
    }
    
    /* close socket */
    //???
    printf("OOOPS!!!\n");
    close(sd);
}


// Thread Function
void* handleClient(void *arg)
{
    int count=0;        // Used For Record How many bytes read and write
    char buf[BUFSIZE];  //Used for outgoing data
    char Ins[BUFSIZE]; // Used for incoming data
    
    std::string client_name; //Used for store the client Name
    
    int sd = *((int*)arg);  /* get sd from arg */
    free(arg);              /* free malloced arg */
    
    // read client name
    if ( (count = read(sd, Ins, sizeof(Ins)) ) == -1) {
        perror("read");
        exit(1);
    }
    printf("Server read %d bytes \n", count);//
    /*If Client exit at enter name;
    if (count==0) {
        printf("I'm here read %d bytes \n", count);//
        close(sd);
    }
    */
    
    client_name=std::string(Ins);
    printf("Name %s\n", client_name.c_str());
    //Make a connection
    sem_wait(&Visit_Server_Data);
    //IF exist user and connected, deny access
    if(s_db.access_check(client_name)&&connection_number<Max_Known_Users)
    {
        s_db.connect(client_name);
        strcpy(buf,"approve");
    }
    else
    {
        strcpy(buf,"deny");
    }
    sem_post(&Visit_Server_Data);
    
    //send message back to Client (approve or deny)
    if ( (count = write(sd, buf, strlen(buf)+1) ) == -1) {
        perror("write");
        exit(1);
    }
    printf("Server sent %d bytes\n", count);
    
    //??? if client control+c to stop the socket, then server will keep while loop
    // if deny
    while (strcmp(buf, "deny")!=0)
    {
        printf("While\n");
        char Recipient[BUFSIZE];  //store recipient
        char Message[BUFSIZE];    //store message
        memset(Recipient, 0, sizeof(Recipient));
        memset(Message, 0, sizeof(Recipient));
        memset(buf, 0, sizeof(buf));
        //read ins from the client
        if ((count = read(sd, Ins, sizeof(Ins)) ) == -1) {
            perror("read");
            exit(1);
        }
        
        printf("Server read %d bytes\n", count);//???
        //Known Client want to exit out
        if (strcmp(Ins, "7")==0) {
            sem_wait(&Visit_Server_Data);
            s_db.disconnect(client_name);
            printf("%s, %s exits.\n",get_time_now().c_str(),client_name.c_str());
            sem_post(&Visit_Server_Data);
            break;
        }
        switch (Ins[0]) {
            case '1':
                sem_wait(&Visit_Server_Data);
                strcpy(buf,s_db.display_all_known_users().c_str());
                printf("%s, %s displays all known usrs.\n",get_time_now().c_str(),client_name.c_str());
                sem_post(&Visit_Server_Data);
                break;
            case '2':
                sem_wait(&Visit_Server_Data);
                strcpy(buf,s_db.display_all_connecting_users().c_str());
                printf("%s, %s displays all connected usrs.\n",get_time_now().c_str(),client_name.c_str());
                sem_post(&Visit_Server_Data);
                break;
            case '3'://Message to someone
                //read recipient's name
                if ((count = read(sd, Recipient, sizeof(Recipient)) ) == -1) {
                    perror("read");
                    exit(1);
                }
                printf("Inside function-3-Recipient, Server read %d bytes\n", count);//???
                //read message
                if ((count = read(sd, Message, sizeof(Message)) ) == -1) {
                    perror("read");
                    exit(1);
                }
                printf("Inside function-3-Message, Server read %d bytes\n", count);//???
                sem_wait(&Visit_Server_Data);
                strcpy(buf,s_db.message_to(client_name, Recipient, Message).c_str());
                sem_post(&Visit_Server_Data);
                printf("%s, %s post a message for %s.\n",get_time_now().c_str(),client_name.c_str(),Recipient);
                break;
            case '4'://Message to every current connected person ???
                
                if ((count = read(sd, Message, sizeof(Message)) ) == -1) {
                    perror("read");
                    exit(1);
                }
                printf("Inside function-4-Message, Server read %d bytes\n", count);//???
                sem_wait(&Visit_Server_Data);
                strcpy(buf,s_db.message_to_connecting_users(client_name, Message).c_str());
                sem_post(&Visit_Server_Data);
                
                printf("%s, %s post a message for all connected users.\n",get_time_now().c_str(),client_name.c_str());
                break;
            case '5'://message to every known person ???
                if ((count = read(sd, Message, sizeof(Message)) ) == -1) {
                    perror("read");
                    exit(1);
                }
                printf("Inside function-5-Message, Server read %d bytes\n", count);//???
                sem_wait(&Visit_Server_Data);
                strcpy(buf,s_db.message_to_known_users(client_name, Message).c_str());
                sem_post(&Visit_Server_Data);
                
                printf("%s, %s post a message for all known users.\n",get_time_now().c_str(),client_name.c_str());
                break;
            case '6':
                sem_wait(&Visit_Server_Data);
                //?? if get my message too long
                strcpy(buf,s_db.Get_my_messages(client_name).c_str());
                sem_post(&Visit_Server_Data);
                printf("%s, %s gets messages.\n",get_time_now().c_str(),client_name.c_str());
                break;
            default:
                continue;
                break;
        }

        //send BUF to clients
        if ( (count = write(sd, buf, strlen(buf)+1) ) == -1) {
            perror("write");
            exit(1);
        }
        printf("Server sent %d bytes\n", count);
    }
    
    /* close socket */
    while (strcmp(buf, "deny")!=0) {
        printf("%s Close\n",client_name.c_str());
    }
    close(sd);
}



