/*      Author: Willard Laxton  */
/*      Project: CS 4760 P2     */
/*      Date: 02-25-2019        */

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char *argv[])
{
//Shared Clock Structure
	struct clock_Structure
        {
                int seconds;
                int nanoseconds;
        };

//Local Vars
	key_t key = 100;
        int shm_Clock_Ptr = shmget(key, 2 * sizeof(struct clock_Structure), 0777| IPC_CREAT);
        struct clock_Structure *clock = (struct clock_Structure*) shmat(shm_Clock_Ptr,(void*)0,0);
 	int duration_Seconds, duration_Nanoseconds;	


	duration_Nanoseconds = clock->nanoseconds + atoi(argv[0]);
	
//Converts Nanoseconds To Seconds When Nanoseconds Exceds 1000000000
	if( (duration_Nanoseconds - 1000000000) > 0)
     	{
       		duration_Seconds += 1;
         	duration_Nanoseconds -=  1000000000;
    	}

//Increments Clock Until It Runs For The Time Assigned By The Parent
	while(1)
	{
		if(clock->seconds > duration_Seconds || ((duration_Seconds == clock->seconds) && (clock->nanoseconds >= duration_Nanoseconds)))
       		{
			break;
		}
	}
}

