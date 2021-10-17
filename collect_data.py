from os import system, wait
import re
from time import sleep
# from multiprocessing import Process, Queue
from threading import Thread

try:
    from parse import parse
except ModuleNotFoundError:
    print("ERROR! Parse needs to be installed.\n Try pip3 install parse.")
    exit(-1)


def thread_func(i, j):
    # print("HELLo")
    # system(f"gcc main_v{j}.c -o main_v{j}.out -lpthread")
    system(
        f"./main_v{j}.out 3 10 2.0 {i+1} 8 10 12 > results/v{j}/heur{i+1}.txt")


Procc = []
for j in range(2):
    system(f"gcc main_v{j}.c -o main_v{j}.out -lpthread")
    sleep(1)
    for i in range(3):
        # thread_func(i, j)
        Procc.append(Thread(target=thread_func, args=[i, j]))
        Procc[-1].start()

# for p_id in Procc:
#     p_id.start()

for p_id in Procc:
    print("Waiting for Thread Conn")
    p_id.join()

# sleep(310)

for j in range(2):
    print(f"VERSION {j}:")

    deadlock_times = [[], [], []]
    complete_cnt = [0, 0, 0]
    closed_cnt = [0, 0, 0]

    for i in range(3):
        with open(f"results/v{j}/heur{i+1}.txt") as file:
            for line in file.readlines():
                line = line.rstrip()

                obj = re.search("COMPLETED", line)

                if obj:
                    complete_cnt[i] += 1
                else:
                    obj = re.search("Num Iterations", line)
                    if obj:
                        res = parse(
                            "Time Since Previous Deadlock: {}, Num Iterations: {}.", line)
                        deadlock_times[i].append(float(res[0]))
                    else:
                        obj = re.search("Count Closed Threads", line)
                        if obj:
                            res = parse("Count Closed Threads: {}", line)
                            closed_cnt[i] += int(res[0])

    for i in range(3):
        print(f"\tHEURISTIC ID: {i+1}")
        print(f"\tCompleted Threads: {complete_cnt[i]}")
        print(
            f"\tAverage Time Between Deadlocks: {sum(deadlock_times[i])/len(deadlock_times[i])}\n")


# VERSION 0:
# 	HEURISTIC ID: 1
# 	Completed Threads: 172
# 	Average Time Between Deadlocks: 6.622161066666665

# 	HEURISTIC ID: 2
# 	Completed Threads: 150
# 	Average Time Between Deadlocks: 5.632672632653059

# 	HEURISTIC ID: 3
# 	Completed Threads: 168
# 	Average Time Between Deadlocks: 6.29783995744681

# VERSION 1:
# 	HEURISTIC ID: 1
# 	Completed Threads: 201
# 	Average Time Between Deadlocks: 8.116767916666667

# 	HEURISTIC ID: 2
# 	Completed Threads: 148
# 	Average Time Between Deadlocks: 5.484082740740741

# 	HEURISTIC ID: 3
# 	Completed Threads: 130
# 	Average Time Between Deadlocks: 5.020254966101693


# VERSION 0:
# 	HEURISTIC ID: 1
# 	Completed Threads: 181
# 	Average Time Between Deadlocks: 5.7657342156862725

# 	HEURISTIC ID: 2
# 	Completed Threads: 146
# 	Average Time Between Deadlocks: 5.397218056603775

# 	HEURISTIC ID: 3
# 	Completed Threads: 130
# 	Average Time Between Deadlocks: 3.838500594594595

# VERSION 1:
# 	HEURISTIC ID: 1
# 	Completed Threads: 202
# 	Average Time Between Deadlocks: 7.488461461538461

# 	HEURISTIC ID: 2
# 	Completed Threads: 136
# 	Average Time Between Deadlocks: 5.878656816326532

# 	HEURISTIC ID: 3
# 	Completed Threads: 119
# 	Average Time Between Deadlocks: 3.8211780128205115
