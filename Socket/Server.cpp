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


//semaphore Connect_id, 0
static sem_t Connect_id;
//semaphore Visit_Server_DB, 0
static sem_t Visit_Server_DB;
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
    //???
    int  message_amount;
    // Message history
    std::string message_history[Max_User_Msg_History];
};

void* handleClient(void *);

class Server_DB{
    
private:
    //str
    str_client stc_clients[Max_Known_Users];
    // how to make sure char 80
    std::string message;
public:
    Server_DB()
    {
        for (int i=0; i<Max_Known_Users; i++) {
            stc_clients[i].name="";
            stc_clients[i].known=false;
            stc_clients[i].connected=false;
            //??? message_amount
        }
    }
    // Connecting
    void connect(std::string name)
    {
        //known User
        if (get_id(name)!=-1) {
            stc_clients[get_id(name)].connected=true;
            printf("%ld, Connection by known user %s\n",time(0),name.c_str());
        }
        else
        {
            //new array id
            connection_number++;
            //Client Name
            stc_clients[connection_number].name=name;
            if (!stc_clients[connection_number].known)
            {
                stc_clients[connection_number].known=true;
            }
            stc_clients[connection_number].connected=true;
            printf("%ld, Connection by unknown user %s\n",time(0),name.c_str());
        }
    }
    //Menu-1 Display the names of all known users
    std::string display_all_known_users()
    {
        message="Knoown users:\n";
        printf("I'm here1");
        for (int i=0; i<Max_Known_Users; i++) {
            if (stc_clients[i].known) {
                message.append(stc_clients[i].name.c_str());
                message.append("\n");
                printf("I'm here2");
            }
        }
        return message;
    }
    //Menu-2 Display the names of all currently connected users
    void display_all_connecting_users(char user_name)
    {
        printf("wo shi 2");
    }
    //Menu-3 Send a text message to a particular user
    void message_to ()
    {
        printf("wo shi 3");
    }
    //Menu-4 Send a text message to all currently connected users
    void message_to_connecting_users()
    {
        printf("wo shi 4");
    }
    //Menu-5 Send a text message to all known users
    void message_to_known_users()
    {
        printf("wo shi 5");
    }
    //Menu-6 Get My Messages
    void Get_my_messages()
    {
        printf("wo shi 6");
    }
    //check if server known this customer
    bool client_known(int i)
    {
        return stc_clients[i].known;
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

    void disconnect(int i)
    {
        stc_clients[i].connected=false;
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
    if(sem_init(&Visit_Server_DB, 0, 1)==-1)
    {
        perror("sem_init Visit_Server_DB");
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
    close(sd);
}


// Thread Function
void* handleClient(void *arg)
{
    int count=0;        // Used For Record How many bytes read and write
    char buf[BUFSIZE];  // Used for incoming string, and outgoing data
    char temp[BUFSIZE]; //
    std::string client_name; //Used for store the client Name
    
    int sd = *((int*)arg);  /* get sd from arg */
    free(arg);              /* free malloced arg */
    
    // read client name
    if ( (count = read(sd, buf, sizeof(buf)) ) == -1) {
        perror("read");
        exit(1);
    }
    printf("Server read %d bytes \n", count);//???
    
    client_name=std::string(buf);
    printf("Name %s\n", client_name.c_str());
    //Make a connection
    sem_wait(&Connect_id);
    //IF exist user and connected, deny access
    if(s_db.access_check(client_name)&&connection_number<(Max_Known_Users-1))
    {
        s_db.connect(client_name);
        strcpy(temp,"approve");
    }
    else
    {
        //??? If deny what next
        strcpy(temp,"deny");
        //snprintf(temp, BUFSIZE, "E%s Existed or Connection Full! Access Denied!!!", buf);
    }
    sem_post(&Connect_id);
    //Store User Name; Mark as known; Mark as connecting
    
    //strcat(connected_user,buf);

    //snprintf(temp, BUFSIZE, "Hi %s", buf);
    
    //need semaphore
    
    //send message back to Client (approve or deny)
    if ( (count = write(sd, temp, strlen(temp)+1) ) == -1) {
        perror("write");
        exit(1);
    }
    printf("Server sent %d bytes\n", count);
    
    //??? if client control+c to stop the socket, then server will keep while loop
    while (strcmp(temp, "deny")!=0)
    {
        //read a message from the client
        if ( (count = read(sd, buf, sizeof(buf)) ) == -1) {
            perror("read");
            exit(1);
        }
        
        
        printf("Server read %d bytes\n", count);//???
        printf("Testing3:(%s)",buf);
        if (strcmp(buf, "7")==0) {
            break;
        }
        switch (buf[0]) {
            case '1':
                printf("1. Client %s sent \n",client_name.c_str());
                strcpy(temp,s_db.display_all_known_users().c_str());
                printf("I'm here3");
                break;
            case '2':
                printf("2. Client sent %c \n", buf[0]);
                break;
            case '3'://Message to someone
                printf("3. Client sent %c \n", buf[0]);
                break;
            case '4'://Message to every current connected person
                printf("4. Client sent %c \n", buf[0]);
                break;
            case '5'://message to every known person
                printf("5. Client sent %c \n", buf[0]);
                break;
            case '6':
                printf("6. Client sent %c \n", buf[0]);
                break;
            default:
                continue;
                break;
        }
        printf("I'm here4");

        strcpy(temp,"shoudao");
        if ( (count = write(sd, temp, strlen(temp)+1) ) == -1) {
            perror("write");
            exit(1);
        }
        printf("Server sent %d bytes\n", count);
    }
    
    /* close socket */
    close(sd);
}



