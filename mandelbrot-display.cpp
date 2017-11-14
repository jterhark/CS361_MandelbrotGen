#include <iostream>
#include <string>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstring>
#include <iomanip>
#include <signal.h>
#include "structs.h"

using namespace std;

int *memory;
int total = 0;

void sigUser1Handler(int sig){
    exit(total); //todo number of images processed
}

void cleanUp(){
    if (shmdt(memory) < 0) {
        perror("Cannot release shared memory in calc");
    }
}

int main(int argc, char* args[]){
    signal(SIGUSR1, sigUser1Handler);
    atexit(cleanUp);

    char colors[16] = ".-~:+*%O8&?$@#X"; //16->null terminator


    int shmid = atoi(args[1]);
    int msg1 = atoi(args[2]);
    int msg2 = atoi(args[3]);

    memory = (int*) shmat(shmid, nullptr, 0);

    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;

    while(true) {

        cin >> xMin >> xMax >> yMin >> yMax >> nRows >> nCols >> maxIters;

        printf("Display: \n%f\n%f\n%f\n%f\n%d\n%d\n%d\n", xMin, xMax, yMin, yMax, nRows, nCols, maxIters);
        printf("Display: %d", memory[0]);
        printf("Display: %d\n", memory[20]);

        for (int r = 0; r < nRows; ++r) {
            for (int c = 0; c < nCols; ++c) {
                int n = *(memory + r * nCols + c);
                if (n < 0)
                    cout << " ";
                else
                    cout << colors[n % 15];
            }
            cout << endl;
        }

        ++total;

        DoneMessage done{};

        if (msgsnd(msg1, &done, sizeof(DoneMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send message queue message in calc");
            exit(-20);
        }
    }

}