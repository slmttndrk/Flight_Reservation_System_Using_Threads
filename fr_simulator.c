#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h> 
#include "queue.h"
#include "linkedlist.h"


// tid for thread id of reservating client
Queue tid;
// res_seat for reservated seat
Queue res_seat;
// cli for naming client threads
Queue cli;
// cli_to_tid for converting thread ids to client number i
int cli_to_tid[101];

// clients' work
void *reserve();
// servers' work
void *request();

// semaphore creating
sem_t sem;
// for available seat number in a plane
int seat;
// for a copy available seat number in a plane that will be used in a linked list 
int copyseat;
// a linked list head pointer
node_t * head = NULL;


int main(int argc, char** argv){
	// initialize queues
	queueInit(&tid, sizeof(int));
	queueInit(&res_seat, sizeof(int));
	queueInit(&cli, sizeof(int));

	// if there is not any argument
	if (argc != 2) { 
		fprintf(stderr,"An integer argument must be inserted \n");
		return -1;
	}
	
	// if the value that user enters is not between [50-100]
	if (atoi(argv[1]) > 100 || atoi(argv[1]) < 50 ) { 
		fprintf(stderr,"Argument must be in between [50-100] \n");
		return -1;
	}
	
	// get the seat number
	seat=atoi(argv[1]);
	// copy the seat number
	copyseat=seat;
	// push the whole seats in a linked list
	for(int i=0; i<copyseat; i++){
		if(i==0)
			pushbegin(&head,i);		
		else		
		pushend(head,i);
	}

	// creating main thread
	pthread_t clients[seat];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	// creating child threads and sending tasks
	for (int i = 0; i < seat; i++){
		pthread_create(&clients[i], &attr, reserve, NULL);
	}

	// waiting for all child threads to be terminated
	for (int i = 0; i < seat; i++)
		pthread_join(clients[i],NULL);

	// converting long unsigned integer thread ids to integer ids between 1 and total client number
	int convert_lui;
	for(int k=0; k<seat; k++){
		dequeue(&cli, &convert_lui);
		cli_to_tid[k]=convert_lui;
	}
	
	// writing the output file
	FILE *file=fopen("output.txt", "w");
	fprintf(file,"Number of total seats: %d \n", seat);
	while(getQueueSize(&tid) > 0 && getQueueSize(&res_seat) > 0)
    {	
		int t, r_s, i; 
        
		dequeue(&tid, &t);
		dequeue(&res_seat, &r_s);

		for(i=0;i<seat;i++){
			if(cli_to_tid[i]==t)
				break;	
		}
        fprintf(file, "Client%d reserves Seat%d \n", i+1, r_s+1);
    }
	fprintf(file,"All seats  are reserved. \n");
	fclose(file);

	return 0;
}

// child threads' work
void *reserve()
{
	// counting semaphore initialize
	sem_init(&sem,0,seat);
	// semaphore for enqueueing the thread ids before reservation
	sem_wait(&sem);
	long unsigned int bef_res=pthread_self();
	enqueue(&cli,&bef_res);
	sem_post(&sem);
	
	// random sleep time in ms
	srand ( time ( NULL));
	int r = (rand()%151)+50;
	usleep(r);
	
	// create a pair server thread immediately after wake up
	pthread_t server;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	
	// send request task to the server thread
	pthread_create(&server, &attr, request, NULL);	
	// wait for the server thread to terminate
	pthread_join(server,NULL);	
	
	// semaphore for enqueueing thread ids after reservation
	sem_wait(&sem);
	long unsigned int aft_res=pthread_self();
	enqueue(&tid, &aft_res);
	sem_post(&sem); 
	
	// terminating after reservation is completed
	pthread_exit(0);

}

// server thread's work
void *request()
{
	// semaphore to choose a seat that is not hold by another client
	sem_wait(&sem);
	int r = (rand()%copyseat);
	// remove available seat from linked list in order to prevent another client to see his seat is available
	r=remove_by_index(&head,r);
	// decrease the random range because of removed available seat in the linked list	
	copyseat--;
	sem_post(&sem);
	
	// enqueue the reserved seat			
	enqueue(&res_seat, &r);

	// server thread terminates
	pthread_exit(0);
	
}


