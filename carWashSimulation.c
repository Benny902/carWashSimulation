#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <signal.h>
#include <errno.h>

float nextTime(float rateParameter);
void killThread();

struct Car {
    pid_t pid;
    int in, out, count;
    clock_t begin, finish, sum;
    bool done;
};

int tCount = 0;
clock_t tSum = 0;
sem_t semaphore;
time_t start;

int main(int argc, char* argv[]) {
    // need to have 4 arguments: './FileName' 'N(numOfStations)' 'avgArriveTime' 'avgWashTime' 'simTime'
    if ((argc != 5) || ((atoi(argv[1])) <= 0))
	{
		fprintf(stderr, "Usage: %s number_of_processes\n", argv[0]);
		exit(1);
	}

    int N = atoi(argv[1]); // num of stations
    float avgArriveTime = atof(argv[2]); // average time between car arrivale
    float avgWashTime = atof(argv[3]); // average car wash time 
    int simTime = atof(argv[4]);  // simulation time
    
    
    start = time(0);
    bool *queue;
    struct Car *car;
    
    pid_t main_pid = getpid();
    
    // key : IPC_PRIVATE
    // int shmget(key_t key, int size, int flag);
    int shmid = shmget(IPC_PRIVATE, sizeof(bool) * N, IPC_CREAT|IPC_EXCL|0666);
    //void *shmat(int shmid, void* addr, int flag);
    queue = (bool*)shmat(shmid, NULL, 0);
    
    shmid = shmget(IPC_PRIVATE, sizeof(struct Car) * 50, IPC_CREAT|IPC_EXCL|0666);
    car = (struct Car*)shmat(shmid, NULL, 0);
    

    for(int i=0; i<N; ++i)
        queue[i] = true;
        
    //int sem_init(sem_t *sem, int pshared, unsigned int value);  // Link with -pthread.
    
    sem_init(&semaphore, 1, 1);

    pid_t pid = fork();
    if(pid == 0){ // child proccess
	signal(SIGINT,killThread);
	while(!car->done) {
	    if((time(0) - start) >= simTime) {
		sem_wait(&semaphore);
		car->done = true;
		sem_post(&semaphore);
		break;
	    }
	    //srand(time(0));
	    sleep(nextTime(1.0/avgArriveTime));
            if(fork() == 0) {  
		struct Car newCar;
		newCar.pid = getpid();
		sem_wait(&semaphore);
		newCar.begin = time(0) - start;
		car[car->in++] = newCar;
		printf("Car #%d entered the queue at Time:%ld\n", newCar.pid, newCar.begin);
		sem_post(&semaphore);
		sleep(1);
		tCount++;
		tSum += newCar.finish - newCar.begin; 
		printf("Car #%d wash start time: %ld\n", getpid(), time(0) - start);
		//srand(time(0));
		sleep(nextTime(1.0/avgWashTime) + 1.5);
		printf("car #%d has entered cleaning station\n", car->pid);
		printf("Car #%d wash finish time: %ld\n", getpid(), time(0) - start);
		kill(getpid(),SIGINT);	
            }
        }       
    }else { // parent proccess
	signal(SIGINT,SIG_IGN);
	kill(getpid(), SIGSTOP); 
	//wait for child processes to finish
	sleep(1);
        while (!(-1 == wait(NULL) && errno == ECHILD));
        sleep(3);
	printf("Total washed cars: %d\n", tCount);
	float avg;
	//avg = (tSum / tCount); // divide by zero error 
	printf("Average wash time per car : %lf\n", avg);
    }
    //wait for child processes to finish
    //while (!(-1 == wait(NULL) && errno == ECHILD));
    //printf("Total washed cars: %d\n", tCount);
    return 0;
}

float nextTime(float rateParameter){
  return -logf(1.0f-(float)rand() / (RAND_MAX + 1.0)) / rateParameter;
}

void killThread() {
	kill(getpid(), SIGKILL);
}