/*
 * Jake TerHark - jterha2
 * CS361 - mandelbrot
 */

#include <iostream>
#include <string>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstring>
#include <iomanip>
#include <csignal>
#include <fstream>
#include "structs.h"

using namespace std;

int *memory;
int total = 0;

//handle signal from mandelbrot
void sigUser1Handler(int sig){
    exit(total);
}

//release shared memory
void cleanUp(){
    if (shmdt(memory) < 0) {
        perror("Cannot release shared memory in calc");
    }
}

int main(int argc, char* args[]){
    //setup handler functions
    signal(SIGUSR1, sigUser1Handler);
    atexit(cleanUp);

    char colors[16] = ".-~:+*%O8&?$@#X"; //16->null terminator

    //get shared memory and message queue id's (mandelbrot)
    int shmid = atoi(args[1]);
    int msg1 = atoi(args[2]);
    int msg2 = atoi(args[3]);

    //attach shared memory
    memory = (int*) shmat(shmid, nullptr, 0);

    double xMin, xMax, yMin, yMax;
    int nRows, nCols, maxIters;

    //main loop
    while(true) {

        //get input from stdin (mandelbrot-calc)
        cin >> xMin >> xMax >> yMin >> yMax >> nRows >> nCols >> maxIters;

        //get filename from queue 1 (mandelbrot)
        FilenameMessage fMessage;
        msgrcv(msg2, &fMessage, sizeof(FilenameMessage) - sizeof(long int), FILENAMEMESSAGETYPE, 0);

        ofstream file;
        bool write = false;

        //check if file can be created and written to
        if(strcmp(fMessage.filename, "")!=0){
            if(ifstream(fMessage.filename)){
                cerr<<"File already exists"<<endl;
            }else {
                file.open(fMessage.filename);
                if (!file) {
                    cerr<<"Cannot create file"<<endl;
                }else{
                    write=true;
                }
            }
        }

        //output
        for (int r = 0; r < nRows; ++r) {

            //x-axis labels
            if(r==0){
                cout<<setw(5)<<yMax;
            }else if(r==nRows-1){
                cout<<setw(5)<<yMin;
            }else{
                cout<<setw(6)<<setfill(' ');
            }

            for (int c = 0; c < nCols; ++c) {
                int n = *(memory + r * nCols + c);
                if (n < 0)
                    cout << " ";
                else
                    cout << colors[n % 15];
                if(write){
                    file<<n<<" ";
                }
            }
            cout << endl;
            if(write)
                file<<endl;
        }

        //y-axis labels
        cout<<setw(6)<<xMin<<setw(nCols)<<setfill(' ')<<xMax<<endl;

        if(write)
            file.close();

        ++total;

        //send done message to queue 1 (mandelbrot)
        DoneMessage done{};
        if (msgsnd(msg1, &done, sizeof(DoneMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send message queue message in calc");
            exit(-20);
        }
    }

    exit(total);

}