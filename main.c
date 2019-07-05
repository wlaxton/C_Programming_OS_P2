/*	Author: Willard Laxton	*/
/*	Project: CS 4760 P2	*/
/*	Date: 02-25-2019	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

/****************************************
 * 	Structure For Shared Clock	*
 * 	Made Global For Sig Handling	*
 ****************************************/
struct clock_Structure
{
	int seconds;
	int nanoseconds;
};

//Shared Clock Vars
key_t key; 
int shm_Clock_Ptr;
struct clock_Structure *clock;

//Sig Handler Prototypes
void catch_Ctrl_C(int);
void timed_Exit(int);

int main(int argc, char* argv[])
{	
//Timed Exit After Ten Seconds
	signal(SIGALRM, timed_Exit);
	alarm(10);

//Local Vars
	const int DURATION_SIZE = 20;
	char duration[DURATION_SIZE];	

	int number_Of_Arguments = argc, option, o_Flag = 0, i_Flag = 0, argv_Counter = 1, clock_Increment = 0, i = 0;
	int  number_Of_Children = 4, max_Children_Alive = 2, children_Alive = 0, child_Process_Seconds = 0;
	int child_Process_Nanoseconds, child_Process_Duration,	active_Child_Process_Counter = 0;
	int o_Used_Flag = 0, i_Used_Flag = 0, n_Used_Flag = 0, s_Used_Flag = 0, size = 0, status, terminating_Child_Pid = 0;
	
	FILE *input_Fp, *output_Fp;
	pid_t forked_Process; 

//Shared Memory Initialization
	key = 100;
	shm_Clock_Ptr = shmget(key, 2 * sizeof(struct clock_Structure), 0777| IPC_CREAT);
	clock = (struct clock_Structure*) shmat(shm_Clock_Ptr,(void*)0,0);

//Control C Signal Catch
	signal(SIGINT, catch_Ctrl_C);

//Shared Memory Error Check
	if(shm_Clock_Ptr == -1)
        {
                fprintf(stderr, "%s: ", argv[0] );
       		perror("Error in shmget");
           	exit(EXIT_FAILURE);
        }

	if(number_Of_Arguments > 10)
        {       
       		fprintf(stderr, "%s: %s\n", argv[0], "Error: Too many arguments were given");
  		exit(EXIT_FAILURE);
        } 

//GetOpt Loop That Continues Until There Are No More Matching Options (h, i, o)
	while( (option = getopt(number_Of_Arguments, argv, "hnsio")) != -1)
	{
		switch(option)
		{
			case 'h':
				printf("%s\n", "Options:");
				printf("\n%s\n", "-h");
				printf("%s\n\n", "List all of the legal command line options");
				printf("%s\n", "-i");
				printf("%s\n\n", "Reads from default input file named input.dat");
				printf("%s\n", "-i inputfilename");
				printf("%s\n\n", "Reads from given input file");
				printf("%s\n", "-o");
				printf("%s\n\n", "Writes to a default output file called output.dat");
				printf("%s\n", "-o outputfile name");
				printf("%s\n\n", "Writes to a specified output file");
				printf("%s\n", "-n number of children");
                                printf("%s\n\n", "Sets a maximum option of children. Set to 4 as a default");
				printf("%s\n", "-s max number of children active");
                                printf("%s\n\n", "Sets the maximum number of children that are allowed to be active at one time. Set to 2 as default");

				if(number_Of_Arguments == 2)
				{
					exit(EXIT_SUCCESS);
				}

				argv_Counter++;
			break;

//Option n
			case 'n':
				if(n_Used_Flag > 0)
                                {
                                        fprintf(stderr, "%s: %s\n", argv[0], "Error: You can't use the n option more than once");
                                        exit(EXIT_FAILURE);
                                }


				if( (argv[argv_Counter + 1] != NULL) && (strcmp(argv[argv_Counter + 1], "-h") != 0) && (strcmp(argv[argv_Counter + 1], "-n") != 0) && (strcmp(argv[argv_Counter + 1], "-s") != 0) && 
				(strcmp(argv[argv_Counter + 1], "-i") != 0) && (strcmp(argv[argv_Counter + 1], "-o") != 0))
                     		{
                        		number_Of_Children = atoi( argv[argv_Counter + 1]);
                      		}
				
				argv_Counter++;
				n_Used_Flag++;

			break;

//Option s
			case 's':
				if(s_Used_Flag > 0)
                                {
                                        fprintf(stderr, "%s: %s\n", argv[0], "Error: You can't use the s option more than once");
                                        exit(EXIT_FAILURE);
                                }


                                if( (argv[argv_Counter + 1] != NULL) && (strcmp(argv[argv_Counter + 1], "-h") != 0) && (strcmp(argv[argv_Counter + 1], "-n") != 0) && (strcmp(argv[argv_Counter + 1], "-s") != 0) &&
                                (strcmp(argv[argv_Counter + 1], "-i") != 0) && (strcmp(argv[argv_Counter + 1], "-o") != 0))
                                {
                                        max_Children_Alive = atoi( argv[argv_Counter + 1]);
                                }
                                
                                argv_Counter++;
                                s_Used_Flag++;
			break;


//Option i	
			case 'i':
//Prevents The i Option From Being Used More Than Once
				if(i_Used_Flag > 0)
                                {
                                        fprintf(stderr, "%s: %s\n", argv[0], "Error: You can't use the i option more than once");
                                        exit(EXIT_FAILURE);
                                }
  
				if( (argv[argv_Counter + 1] != NULL) && (strcmp(argv[argv_Counter + 1], "-h") != 0) && (strcmp(argv[argv_Counter + 1], "-n") != 0) && (strcmp(argv[argv_Counter + 1], "-s") != 0) &&
				(strcmp(argv[argv_Counter + 1], "-i") != 0) && (strcmp(argv[argv_Counter + 1], "-o") != 0) )
				{
					input_Fp = fopen(argv[argv_Counter + 1], "r");

					if(input_Fp == NULL)
					{
						fprintf(stderr, "%s: ", argv[0] );		
						perror("Error");
						exit(EXIT_FAILURE);
					}
			
//Checks To See If The Input File Is Empty And Exits If It Is
					fseek (input_Fp, 0, SEEK_END);
    					size = ftell(input_Fp);

    					if(size == 0) 
					{
        					fprintf(stderr, "%s: %s\n", argv[0], "Error: Input file empty");
                				exit(EXIT_FAILURE);
  					}
	
					fseek(input_Fp, 0, SEEK_SET);

					i_Flag++;	
					argv_Counter++;
				}

				argv_Counter++;
				i_Used_Flag++;
			break;

//Option o
			case 'o':
//Prevents The o Option From Being Used More Than Once
				if(o_Used_Flag > 0)
                                {
                                        fprintf(stderr, "%s: %s\n", argv[0], "Error: You can't use the o option more than once");
                                        exit(EXIT_FAILURE);
                                }

                                if( (argv[argv_Counter + 1] != NULL) && (strcmp(argv[argv_Counter + 1], "-h") != 0) && (strcmp(argv[argv_Counter + 1], "-n") != 0) && (strcmp(argv[argv_Counter + 1], "-s") != 0) &&
				(strcmp(argv[argv_Counter + 1], "-i") != 0) && (strcmp(argv[argv_Counter + 1], "-o") != 0) )
                                {
                                        output_Fp = fopen(argv[argv_Counter + 1], "w+");
                                        o_Flag++;
                                        argv_Counter++;
                                }

                                argv_Counter++;
				o_Used_Flag++;
			break;
		}
	}	

//Sets Default File For Input
	if(i_Flag == 0)
	{
		input_Fp = fopen("input", "r");

		if(input_Fp == NULL)
          	{
             		fprintf(stderr, "%s: %s\n", argv[0], "Error: File could not be opened");
                    	exit(EXIT_FAILURE);
            	}

//Checks To See If The Input File Is Empty And Exits If It Is
		fseek (input_Fp, 0, SEEK_END);
               	size = ftell(input_Fp);

            	if(size == 0)
             	{
                   	fprintf(stderr, "%s: %s\n", argv[0], "Error: Input file empty");
                 	exit(EXIT_FAILURE);
             	}

        	fseek(input_Fp, 0, SEEK_SET);
	}

//Sets Default File For Output
	if(o_Flag == 0)
        {
                output_Fp = fopen("output", "w+");

		if(output_Fp == NULL)
                {
                        fprintf(stderr, "%s: %s\n", argv[0], "Error: File could not be opened");
                        exit(EXIT_FAILURE);
                }
        }


//Formatting For File
	fprintf(output_Fp, "%-7s %-11s %-10s %-15s\n", "Seconds", " Nanoseconds", "PID", "Duration");

//First Scan Of the File For Clock Increment, Time, And Duration
	fscanf(input_Fp, "%d%d%d%d", &clock_Increment, &child_Process_Seconds, &child_Process_Nanoseconds, &child_Process_Duration);	

	while(i < number_Of_Children)
	{
//If Statement To Checks To See If A New Child Can Be Launched
		if( (children_Alive < max_Children_Alive) && (children_Alive < 20) && (clock->seconds > child_Process_Seconds || 
		  ((child_Process_Seconds == clock->seconds) && (clock->nanoseconds >= child_Process_Nanoseconds))) )
		{		
			sprintf(duration, "%d", child_Process_Duration);				

			children_Alive++;

			if((forked_Process = fork()) == 0)
			{	
				execl("./child", duration, NULL);
			}
	
			fprintf(output_Fp, "%-7d  %-11d %-10d %-15d Created\n", clock->seconds, clock->nanoseconds, forked_Process, child_Process_Duration);

			fscanf(input_Fp, "%d%d%d", &child_Process_Seconds, &child_Process_Nanoseconds, &child_Process_Duration);

			i++;
		}

//Wait Call for Children That Have Finished Executing
		if((terminating_Child_Pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			fprintf(output_Fp, "%-7d  %-11d %-26d Terminated\n", clock->seconds, clock->nanoseconds, terminating_Child_Pid);
			children_Alive--;
		}

//Adds To Clock
		clock->nanoseconds += clock_Increment;

		if( (clock->nanoseconds - 1000000000) > 0)
		{
			clock->seconds += 1;
			clock->nanoseconds -=  1000000000;
		}

//Keeps The Clock Going Until The Last Process Can Terminate
		if(i == number_Of_Children)
                {
			while(children_Alive != 0)
			{
                        	clock->nanoseconds += clock_Increment;

                		if( (clock->nanoseconds - 1000000000) > 0)
                		{
                        		clock->seconds += 1;
                        		clock->nanoseconds -=  1000000000;
                		}
				if((terminating_Child_Pid = waitpid(-1, &status, WNOHANG)) > 0)
                		{
                        		fprintf(output_Fp, "%-7d  %-11d %-26d Terminated\n", clock->seconds, clock->nanoseconds, terminating_Child_Pid);
        	               		children_Alive--;
                		}
			}
                }


//Checks If The End Of File Has Been Reached
		if(feof(input_Fp))
                {
                        fseek(input_Fp, -1, SEEK_CUR);
                        break;
                }
	}


//File Closing
	fclose(input_Fp);
	fclose(output_Fp);

//Destroy Shared Memory
	shmdt(clock);
	shmctl(shm_Clock_Ptr, IPC_RMID, NULL);
	
}

//Catches Ctrl C
void catch_Ctrl_C(int sig)
{
	printf("\nControl C Exit\n");
	
	shmdt(clock);
        shmctl(shm_Clock_Ptr, IPC_RMID, NULL);

	abort();

	exit(EXIT_FAILURE);
}

//Exit After Ten Seconds
void timed_Exit(int sig)
{
	printf("\nTime Exit\n");

	shmdt(clock);
        shmctl(shm_Clock_Ptr, IPC_RMID, NULL);

	abort();

	exit(EXIT_FAILURE);
}
