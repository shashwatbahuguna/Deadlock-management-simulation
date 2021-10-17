# Multiple Threads Deadlock Handling Simulation

Thread to simulate deadlock and handle accoring to different heuristics.

## Compilation and Running:

### Main File
Includes main function, all implementations.
Compilation
gcc main.c -o main.out -lpthread


Execution
USAGE: main.out [Number of Resources] [Number of Threads] [Interval for Execution] [Heuristic ID] Resource Limits [A_lim B_lim ... N_lim]. Heuristic IDs: 0, 1.

./main.out 3 10 2.0 1 8 10 12

### Data Collection
python3 collect_data.py
Executes both versions of the implementation and compiles the results for comparison of different heuristics.

### Working
The main function generates threads which run in an infinite loop, where, random resources required are generated int the range of 0, total resource/2 which are requested with intermittent gaps.
The deadlock checker thread is then generated which begins check in duration as specified by the ommand line argument.

Half of max is used for upper limit to ensure threads are able to decently execute, with decent thread count. 
(Example: 10 threads for 10 as resource limit)

A mutex is used to lock any accesses to the shared data.

After generation of resource count, mutex lock is initiated and the resource is then checked. It is accordingly then allocated or skipped.
The worker thread goes on in a cycle requesting resources untill the required count is reached.

The deadlock checker goes in an infinite loop, sleeps for the interval in command line.
The checker thread then locks the mutex and uses banker's algorithm to check for a deadlock by counting the number of finished threads.
If the count of finished threads according to the algorithm is lesser than the max count, threads are free according to one of the following heuritics:

#### Heursitic 1:
All threads in deadlock are closed and the corresponding resources are then freed.

#### Heursitic 2:
The deadlocked threads are arranged in the descending order of their (total) resource usage and are then closed one by one. After closing each thread and freeing its resources, we again check if the deadlock is removed. 
If deadlocked is removed, we break the loop. We continue to the next thread otherwise.

#### Heursitic 3:
This case is similar to heuristic 2 exept here, the count of those resources involved in a deadlock is used for sorting the threads. In the banker's algorithm, the third boolean array marks the resources which are involved in a deadlock.

NOTE: 
Since pthread_mutex_lock is a no cancellation point, a pthread blocked by pthread_mutex_lock cannot directly be closed. 
Two different strategies are used to deal with this situation:
Version 1:
Here, the pthread close is kept outside mutex lock so that the target thread passes over the protected region and can be easily closed.

Version 2:
Test Cancel calls are made in the locked regions as well to make sure that a thread exits before changing any variable in case of a pthread cancel. Further, to ensure unlocking of the mutex, cleanup functions are used, which are executed when a thread exits.


### Comparison
The outputs from the three heuristics (for 5 mins) are stored in the results directory. 
These are then compiled by the collect_data.py file using the printed statements at various sections in the code. The results are as follows:

VERSION 0:
	HEURISTIC ID: 1
	Completed Threads: 172
	Average Time Between Deadlocks: 6.622161066666665

	HEURISTIC ID: 2
	Completed Threads: 150
	Average Time Between Deadlocks: 5.632672632653059

	HEURISTIC ID: 3
	Completed Threads: 168
	Average Time Between Deadlocks: 6.29783995744681

VERSION 1:
	HEURISTIC ID: 1
	Completed Threads: 201
	Average Time Between Deadlocks: 8.116767916666667

	HEURISTIC ID: 2
	Completed Threads: 148
	Average Time Between Deadlocks: 5.484082740740741

	HEURISTIC ID: 3
	Completed Threads: 130
	Average Time Between Deadlocks: 5.020254966101693

Hence, the first heuristic gives the most time between two consecutive deadlocks.
Further, most threads are able to complete.