#include <iostream>
#include <string>
#include <sys/shm.h>
#include <sys/msg.h>
#include <cstring>
#include <iomanip>
#include <signal.h>
#include <fstream>
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

        FilenameMessage fMessage;

        msgrcv(msg2, &fMessage, sizeof(FilenameMessage) - sizeof(long int), FILENAMEMESSAGETYPE, 0);

        ofstream file;
        bool write = false;

        if(strcmp(fMessage.filename, "")!=0){
            if(ifstream(fMessage.filename)){
                cerr<<"File already exists"<<endl;
            }else {
                file = ofstream(fMessage.filename);
                if (!file) {
                    cerr<<"Cannot create file"<<endl;
                }else{
                    write=true;
                }
            }
        }

        for (int r = 0; r < nRows; ++r) {
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

        cout<<setw(6)<<xMin<<setw(nCols)<<setfill(' ')<<xMax<<endl;

        if(write)
            file.close();

        ++total;

        DoneMessage done{};

        if (msgsnd(msg1, &done, sizeof(DoneMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send message queue message in calc");
            exit(-20);
        }
    }

}