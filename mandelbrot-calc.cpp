#include <iostream>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <sys/shm.h>
#include <sys/msg.h>
#include <csignal>
#include "structs.h"

using namespace std;

int* memory;
int total = 0;

//handle signal from mandelbrot
void sigUser1Handler(int sig){
    cerr<<total<<flush;
    exit(total);
}

//release shared memory
void cleanUp(){
    if (shmdt(memory) < 0) {
        perror("Cannot release shared memory in calc");
    }
}

int main(int argc, char *args[]) {
    //register signal handler and at exit functions
    signal(SIGUSR1, sigUser1Handler);
    atexit(cleanUp);

    total = 0;

    //get message queue and shared memory id's
    int shmid = atoi(args[1]);
    int msgid = atoi(args[2]);

    //attached shared memory
    memory = (int *) shmat(shmid, NULL, 0);

    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;

    //main loop
    while(true) {

        //read values from stdin (pipe from mandelbrot)
        cin >> xMin >> xMax >> yMin >> yMax >> nRows >> nCols >> maxIters;

        //mandelbrot calculations
        double deltaX = (xMax - xMin) / (nCols - 1);
        double deltaY = (yMax - yMin) / (nRows - 1);

        for (int r = 0; r < nRows; ++r) {
            double Cy = yMin + r * deltaY;
            for (int c = 0; c < nCols; ++c) {
                double Cx = xMin + c * deltaX,
                        Zx = 0.0,
                        Zy = 0.0;
                int n = 0;
                for (n = 0; n < maxIters; ++n) {
                    if (Zx * Zx + Zy * Zy >= 4.0)
                        break;
                    double Zx_next = Zx * Zx - Zy * Zy + Cx;
                    double Zy_next = 2.0 * Zx * Zy + Cy;
                    Zx = Zx_next;
                    Zy = Zy_next;
                }
                if (n >= maxIters) {
                    *(memory + r * nCols + c) = -1;
                } else {
                    *(memory + r * nCols + c) = n;
                }
            }

        }

        //send values to stdout (mandelbrot-display)
        printf("%f\n%f\n%f\n%f\n%d\n%d\n%d\n", xMin, xMax, yMin, yMax, nRows, nCols, maxIters);

        ++total;

        //send done message to queue 1 (mandelbrot)
        DoneMessage msg{};
        if (msgsnd(msgid, &msg, sizeof(DoneMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send message queue message in calc");
            exit(-20);
        }
    }

    exit(total);
}

