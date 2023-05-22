# carWashSimulation
Simulation of a car wash in c
with threads, semaphores, memory, singals, ipc and more

On initial run, the software receives the following arguments on the command line:
- Number of washing stations (at most 5)
- Average time between vehicle arrivals (reasonable time for use: around 1.5 seconds)
- Average time for washing a car (reasonable for use: around 3 seconds)
- Running time of the simulation (reasonable time to use: 30 seconds)

run on linux:
an example to run the file (the 4 values can be changed):
gcc -pthread carWashSimulation.c -o carWashSimulation -lm
./carWashSimulation  3 1.5 3 20
