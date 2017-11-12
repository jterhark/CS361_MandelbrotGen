#include <iostream>
#include <string>
#include <sys/shm.h>

using namespace std;

int main(int argc, char* args[]){
    cout<<"in display"<<endl<<flush;


    int shmid = atoi(args[1]);
    int msg1 = atoi(args[2]);
    int msg2 = atoi(args[3]);

    cerr<<"From Display: " << "shmid: " << shmid << " msg1: " << msg1 << " msg2: " <<msg2 << endl<<flush;

    string s;
    getline(cin, s);
    cerr<<"From display: " << s <<endl<<flush;

    auto memory = (int*) shmat(shmid, NULL, 0);
    int i = memory[0];
    cerr<<"Display number test: " <<i<<endl<<flush;
    if(shmdt(memory) < 0){
        perror("Cannot release shared memory in display");
    }

}