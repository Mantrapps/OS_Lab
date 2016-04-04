//
//  main.cpp
//  OS Project2
//
//  Copyright © 2016 CS5348. All rights reserved.
//
#include <iostream>
#include <sys/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>
#include <array>
#include <queue>
#include <fstream>
#include <string>

using namespace std;

#define NUM_BO_Agent     2
#define NUM_MAX_CUS      50
#define NUM_Ticket_Taker 1
#define NUM_Concession_Stand_Worker 1
//Theater ready to open
static sem_t Theater_status[3];




//movies database
pair<string, int>* movies;

//semaphore Box_Office_Queue_Mutex, 1
static sem_t BO_Queue_Mutex;
//semaphore Ticket Taker_Queue_Mutex, 1
static sem_t TT_Queue_Mutex;
//semaphore oncession Worker_Queue_Mutex, 1
static sem_t CW_Queue_Mutex;


//semaphore Box Office Windows Idle, 0
//static sem_t BO_Agent_Windows;
//semaphore Box Office Agents processed the ticket request, 0
static sem_t Buy_Ticket;
//semaphore Customer BO_Response, 0
static sem_t BO_Response[NUM_MAX_CUS];
//semaphore Check ticket remain, 1
static sem_t Sell_Ticket_Mutex;
//semaphore Leave Ticket Windows, 0
static sem_t Leave_BOX_Office[NUM_MAX_CUS];
//semaphore Request to Ticket Taker, 0
static sem_t Check_Ticket;
//semaphore Ticket Taker approve to entry, 0
static sem_t Ticket_Approve;
//semaphore leave_Ticket_Taker
static sem_t Leave_Ticket_Taker;
//semaphore Concession Worker ready for getting order, 0
static sem_t Concession_Order;
//semaphore Concession Worker delivered order, 0
static sem_t Fullfil_Order;
//semaphore Leave Concession Worker
static sem_t Leave_Concession_Worker;




struct Customer_S
{
    int customerid;
    string movie;
    int movie_id;
    //what snack?
    string snack;
};


//Queue at Box office
static queue<Customer_S> BO_Queue;
//Queue at Ticket Taker
static queue<Customer_S> TT_Queue;
//Queue at Concession Worker
static queue<Customer_S> CW_Queue;
//Ticket Array
static array<bool, NUM_MAX_CUS> Get_Ticket_Status;


void Tearing_Ticket()
{
    sleep(15/60);
}
void Filling_Order()
{
    sleep(180/60);
}
bool Selling_Ticket(int i)
{
    sleep(90/60);
    sem_wait (&Sell_Ticket_Mutex) ;
    if (movies[i].second<=0) {
        sem_post(&Sell_Ticket_Mutex);
        return false;
    }
    else{
        
        movies[i].second=movies[i].second-1;
        sem_post(&Sell_Ticket_Mutex);
        return true;
    }
}
class Customer
{
public:
    Customer(const int i)
    {
        Customer_S ctemp;
        ctemp.customerid=i;
        ctemp.movie_id=rand()%5;
        ctemp.movie=movies[ctemp.movie_id].first;
        cus=ctemp;
    }
    std::string getMovie()
    {
        return cus.movie;
    }
    std::string getSnack()
    {
        return cus.snack;
    }
    static void* work(void *arg)
    {
        int *pnum = (int *)arg;
        int num = *pnum;
        free(arg);
        Customer cus(num);
        cus.Execute();
        return NULL;
    }
    void Execute()
    {
        printf("Customer %d created, buying ticket to %s\n", cus.customerid, cus.movie.c_str());

        sem_wait(&BO_Queue_Mutex);
        
        //put customer in Box Office Queue
        BO_Queue.push(cus);
        //signal Buy_Ticket
        sem_post(&Buy_Ticket);
        //printf("Customer %d in queue", cs.customerid);
        sem_post(&BO_Queue_Mutex);
        //wait Get_Ticket of cus id
        sem_wait(&BO_Response[cus.customerid]);

        if (Get_Ticket_Status[cus.customerid]) {
            sem_wait(&TT_Queue_Mutex);
            //put Customer(have ticket) in Ticket Taker Queue
            TT_Queue.push(cus);
            printf("Customer %d in line to see ticket taker\n", cus.customerid);
            //signal see ticket to check_ticket
            sem_post(&Check_Ticket);
            //???
            //Signal Leave_Box_Office with Ticket
            sem_post(&Leave_BOX_Office[cus.customerid]);
            
            sem_post(&TT_Queue_Mutex);
            
            //wait Ticket Approve
            sem_wait(&Ticket_Approve);
            
            //50%
            if (rand()%2==1) {
                //see Concession
                switch (rand()%2) {
                    case 0:
                        cus.snack="popcorn";
                        break;
                    case 1:
                        cus.snack="soda";
                        break;
                    case 2:
                        cus.snack="soda and popcorn";
                        break;
                    default:
                        break;
                }
                
                sem_wait(&CW_Queue_Mutex);
                CW_Queue.push(cus);
                printf("Customer %d in line to buy %s \n", cus.customerid,cus.snack.c_str());
                //signal Concession Order
                sem_post(&Concession_Order);
                //Signal Leave_Ticket_Taker ???
                sem_post(&Leave_Ticket_Taker);
                sem_post(&CW_Queue_Mutex);
                sem_wait(&Fullfil_Order);
                printf("Customer %d receives %s \n", cus.customerid,cus.snack.c_str());
                
                printf("Customer %d enters theater to see %s \n", cus.customerid,cus.movie.c_str());
                sem_post(&Leave_Concession_Worker);
            }
            else{
                //Not see Concession
                printf("Customer %d enters theater to see %s \n", cus.customerid,cus.movie.c_str());
                //???
                sem_post(&Leave_Ticket_Taker);
            }
        }
        //Signal Leave_Box_Office without Ticket
        else{
            sem_post(&Leave_BOX_Office[cus.customerid]);
        }
        
    }
private:
    //struct customer
    Customer_S cus;
};


class Box_Office{
    
public:
    Box_Office(const int id)
    {
        bid=id;
    }
    static void* work(void *arg)
    {
        int *pnum = (int *)arg;
        int num = *pnum;
        free(arg);
        Box_Office boxoffice(num);
        boxoffice.Execute();
        return NULL;
    }
    void Execute()
    {
    
        printf("Box office agent %d created\n", bid);

        sem_post(&Theater_status[0]);
        while (true)
        {

            //wait customer Buy Ticket
            sem_wait (&Buy_Ticket);
   
            sem_wait(&BO_Queue_Mutex);
            //get customer in Box Office Queue
            Customer_S cus=BO_Queue.front();
            BO_Queue.pop();
            printf("Box office agent %d serving customer %d \n", bid, cus.customerid);
            sem_post(&BO_Queue_Mutex);
            
            //sell ticket
            if(Selling_Ticket(cus.movie_id)) {Get_Ticket_Status[cus.customerid]=true;}
            
            //Signal BO_Response[]
            sem_post (&BO_Response[cus.customerid]);
        
            if (Get_Ticket_Status[cus.customerid]) {
                printf("Box office Agent %d sold ticket for %s to customer %d \n", bid, cus.movie.c_str(), cus.customerid);
            }

            sem_wait (&Leave_BOX_Office[cus.customerid]);

        }
    }
private:
    int bid;
};


class Ticket_Taker{
    
public:
    Ticket_Taker(const int id)
    {
        Tid=id;
    }
    int getID()
    {
        return Tid;
    }
    static void* work(void *arg)
    {
        int *pnum = (int *)arg;
        int num = *pnum;
        free(arg);
        Ticket_Taker ticket_taker(num);
        ticket_taker.Execute();
        return NULL;
    }
    void Execute()
    {
        
        printf("Ticket Taker created\n");
        sem_post(&Theater_status[1]);
        while (true)
        {
            //wait customer to check ticket
            sem_wait (&Check_Ticket);

            
            sem_wait(&TT_Queue_Mutex);

            //get customer in Ticket Taker Queue
            Customer_S cus=TT_Queue.front();
            TT_Queue.pop();
            sem_post(&TT_Queue_Mutex);
            
            Tearing_Ticket();
            
            printf("Ticket taken from customer %d \n", cus.customerid);
            
            //Signal Ticket_Approve
            sem_post (&Ticket_Approve);
            
            //Wait leave
            sem_wait (&Leave_Ticket_Taker);
            
        }
    }
private:
    //Ticket ID
    int Tid;
};


class Concession_Stand_Work
{
public:
    Concession_Stand_Work(const int id)
    {
        CW_id=id;
    }
    static void* work(void *arg)
    {
        int *pnum = (int *)arg;
        int num = *pnum;
        free(arg);
        Concession_Stand_Work cw_worker(num);
        cw_worker.Execute();
        return NULL;
    }
    void Execute()
    {
        
        printf("Concession stand worker created\n");
        sem_post(&Theater_status[2]);
        
        while (true)
        {
            //wait customer to order Concession
            sem_wait (&Concession_Order);
            
            
            sem_wait(&CW_Queue_Mutex);
            
            //get customer in Concession Worker Queue
            Customer_S cus=CW_Queue.front();
            CW_Queue.pop();
            printf("Order for %s taken from customer %d \n", cus.snack.c_str(),cus.customerid);
            sem_post(&CW_Queue_Mutex);
            
            Filling_Order();
            printf("%s given to customer %d \n", cus.snack.c_str(),cus.customerid);
            //Signal Fullfil order
            sem_post (&Fullfil_Order);
            //Wait leave Concession Worker
            sem_wait (&Leave_Concession_Worker);

        }
    }
private:
    //cw ID
    int CW_id;
};



// Box Office Agent Thread
void Box_Office_Agent_Working()
{
    pthread_t Box_Agent_Threads[NUM_BO_Agent];
    //Box_Office *Box_Agents;
    //Box_Agents=new Box_Office[NUM_BO_Agent];

    int index;
    int status;
    
    for (index = 0; index < NUM_BO_Agent; index++) {
        int *pnum = (int*)malloc(sizeof(int));
        *pnum = index;
        //Box_Agents[index]=Box_Office(index);
        status = pthread_create(&Box_Agent_Threads[NUM_BO_Agent], NULL, Box_Office::work, (void*)pnum);
        assert(0 ==status);
    }
}
// Ticket Taker Thread
void Ticket_Taker_Working()
{
    pthread_t Ticket_Taker_Thread[NUM_Ticket_Taker];
    int index;
    int status;
    for (index = 0; index < NUM_Ticket_Taker; index++) {
        int *pnum = (int*)malloc(sizeof(int));
        *pnum = index;
        status = pthread_create (&Ticket_Taker_Thread[NUM_Ticket_Taker], NULL, Ticket_Taker::work, (void*)pnum);
        assert(0 ==status);
    }

}
// Concession Worker Thread
void Concession_Stand_Worker_Working()
{
    pthread_t Concession_Stand_Worker[NUM_Concession_Stand_Worker];
    int index;
    int status;
    for (index = 0; index < NUM_Concession_Stand_Worker; index++) {
        int *pnum = (int*)malloc(sizeof(int));
        *pnum = index;
        status = pthread_create (&Concession_Stand_Worker[NUM_Concession_Stand_Worker], NULL, Concession_Stand_Work::work, (void*)pnum);
        assert(0 ==status);
    }
}

void Load_Movie()
{
    //Movies list and ticket
    movies = new pair<string, int> [5];
    string textline;
    string film_name;
    int remain_ticket;
    size_t i;
    int j=0;
    int num=0;
    ifstream LoadFile("movies.txt");
    if (LoadFile.is_open()) {
        while (!LoadFile.eof()) {
            getline(LoadFile, textline);
            i=textline.find_last_of(" ");
            film_name=textline.substr(0,i);
            remain_ticket=atoi(textline.substr(i+1).c_str());
            movies[j++]=make_pair(film_name, remain_ticket);
        }
        LoadFile.close();
    }

}
// Prepare for Theater_Open ???
void Theater_Open()
{
    //load Movies now
    Load_Movie();
    //Box Office in Position
    Box_Office_Agent_Working();
    //Ticket Taker in Position
    Ticket_Taker_Working();
    //Concession Worker in Position
    Concession_Stand_Worker_Working();
    int count;
    //make sure every work position in Theater working.
    for (count=0; count<NUM_BO_Agent; count++) {
        sem_wait(&Theater_status[0]);
    }
    for (count=0; count<<NUM_Ticket_Taker; count++) {
        sem_wait(&Theater_status[1]);
    }
    for (count=0; count<<NUM_Concession_Stand_Worker; count++) {
        sem_wait(&Theater_status[2]);
    }
    printf("Theater is open\n");
}


int main(void)
{
    // initialize semaphore Theater Status
    for (int i=0; i<3; i++) {
        if(sem_init(&Theater_status[i], 0, 0)==-1)
        {
            perror("sem_init Theater_status");
            exit(EXIT_FAILURE);
        }
    }

    
    // initialize semaphore BO_Queue_Mutux
    if(sem_init(&BO_Queue_Mutex, 0, 1)==-1)
    {
        perror("sem_init BO_Queue_Mutux");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore TT_Queue_Mutex
    if(sem_init(&TT_Queue_Mutex, 0, 1)==-1)
    {
        perror("sem_init TT_Queue_Mutex");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore CW_Queue_Mutex
    if(sem_init(&CW_Queue_Mutex, 0, 1)==-1)
    {
        perror("sem_init CW_Queue_Mutex");
        exit(EXIT_FAILURE);
    }
    
    // initialize semaphore BO_Agent_Windows
    //if(sem_init(&BO_Agent_Windows, 0, 0)==-1)
    /*{
        perror("sem_init BO_Agent_Windows");
        exit(EXIT_FAILURE);
    }*/
    // initialize semaphore Buy_Ticket
    if(sem_init(&Buy_Ticket, 0, 0)==-1)
    {
        perror("sem_init Buy_Ticket");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore BO_Response
    for (int i=0; i<NUM_MAX_CUS; i++) {
        if(sem_init(&BO_Response[i], 0, 0)==-1)
        {
            perror("sem_init BO_Response");
            exit(EXIT_FAILURE);
        }
    }
    // initialize semaphore Get_Ticket
    if(sem_init(&Sell_Ticket_Mutex, 0, 1)==-1)
    {
        perror("sem_init Sell_Ticket");
        exit(EXIT_FAILURE);
    }
    // nitialize semaphore Leave_Box_Office
    for (int i=0; i<NUM_MAX_CUS; i++) {
        if(sem_init(&Leave_BOX_Office[i], 0, 0)==-1)
        {
            perror("sem_init leave_BOX_Office");
            exit(EXIT_FAILURE);
        }
    }
    // initialize semaphore Leave_Ticket_Taker
    if(sem_init(&Leave_Ticket_Taker, 0, 0)==-1)
    {
        perror("sem_init Leave_Ticket_Taker");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore check_ticket, 0
    if(sem_init(&Check_Ticket, 0, 0)==-1)
    {
        perror("sem_init Check_Ticket");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore Ticket_Approve, 0
    if(sem_init(&Ticket_Approve, 0, 0)==-1)
    {
        perror("sem_init Check_Ticket");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore Concession_Order, 0
    if(sem_init(&Concession_Order, 0, 0)==-1)
    {
        perror("sem_init Concession_Order");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore Fullfil_Order, 0
    if(sem_init(&Fullfil_Order, 0, 0)==-1)
    {
        perror("sem_init Fullfil_Order");
        exit(EXIT_FAILURE);
    }
    // initialize semaphore Leave_Concession_Worker, 0
    if(sem_init(&Leave_Concession_Worker, 0, 0)==-1)
    {
        perror("sem_init Leave_Concession_Worker");
        exit(EXIT_FAILURE);
    }

    
    //open theater
    Theater_Open();
    
    int index;
    int status;
    
    //Create Customer Threads
    pthread_t Customer_Threads[NUM_MAX_CUS];
    for (index = 0; index < NUM_MAX_CUS; index++) {
        
        int *pnum = (int*)malloc(sizeof(int));
        *pnum = index;
        //Customers[index]=*new Customer(index);
        status = pthread_create (&Customer_Threads[index], NULL, Customer::work, (void*)pnum);
        assert(0 ==status);
    }
    
    // wait for 50 Customer threads to complete
    for (index = 0; index < NUM_MAX_CUS; index++) {
        // block until thread 'index' completes
        status = pthread_join(Customer_Threads[index], NULL);
        printf("Joined customer %d\n", index);
        assert(0 == status);
    }

    printf("All customers thread completed successfully\n");
    exit(EXIT_SUCCESS);


}