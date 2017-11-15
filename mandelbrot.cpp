#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <algorithm>
#include <cstring>
#include "structs.h"

#define READ 0
#define WRITE 1
#define SHMAX 4000

//todo set signal handler for SIGCHLD

using namespace std;

int msgid1 = -1, msgid2 = -1, shmid = -1;
pid_t calcPid = -1, displayPid = -1;

void exitHandler(int signal){
    exit(1);
}

void cleanup() {
//free message queues
    if (msgid1 != -1 && msgctl(msgid1, IPC_RMID, nullptr) < 0) {
        perror("Cannot free first message queue");
    }

    if (msgid2 != -1 && msgctl(msgid2, IPC_RMID, nullptr) < 0) {
        perror("Cannot free second message queue");
    }

    //free shared memory
    if (shmid != -1 && shmctl(shmid, IPC_RMID, nullptr) < 0) {
        perror("Cannot free shared memory: ");
    }

    //kill processes
    if(calcPid!=-1)
        kill(calcPid, SIGUSR1);

    if(displayPid!=-1)
        kill(displayPid, SIGUSR1);

    int status;

    if(calcPid!=-1) {
        wait4(calcPid, &status, WUNTRACED, nullptr);
        cout << "Calc processes exited with code " << WEXITSTATUS(status) << std::endl;
    }

    if(displayPid!=-1) {
        wait4(displayPid, &status, WUNTRACED, nullptr);
        cout << "Display process exited with code " << WEXITSTATUS(status) << std::endl;
    }
}

int main() {
    int pipe1[2], pipe2[2];

    char msgid1Char[sizeof(int)],
            msgid2Char[sizeof(int)],
            shmidChar[sizeof(int)];

    signal(SIGINT, exitHandler);
    signal(SIGSEGV, exitHandler);
    signal(SIGTERM, exitHandler);
    signal(SIGKILL, exitHandler);
    signal(SIGSTOP, exitHandler);

    atexit(cleanup);

    //create pipes
    if (pipe(pipe1) == -1) {
        perror("Cannot create first pipe");
        exit(-3);
    }

    if (pipe(pipe2) == -1) {
        perror("Cannot create second pipe");
        exit(-4);
    }

    if ((msgid1 = msgget(IPC_PRIVATE, IPC_CREAT | 0660)) == -1) {
        perror("Cannot create first message queue");
        exit(-9);
    }

    if ((msgid2 = msgget(IPC_PRIVATE, IPC_CREAT | 0660)) == -1) {
        perror("Cannot create second message queue");
        exit(-10);
    }


    //create shared memory
    if ((shmid = shmget(IPC_PRIVATE, SHMAX * sizeof(int), IPC_CREAT | 0660)) == -1) {
        perror("Cannot create shared memory: ");
        exit(-1);
    }


    sprintf(msgid1Char, "%d", msgid1);
    sprintf(msgid2Char, "%d", msgid2);
    sprintf(shmidChar, "%d", shmid);


    if ((calcPid = fork()) == -1) {
        perror("Cannot fork calculation process");
        exit(-5);
    } else if (calcPid == 0) {//calc child
        close(pipe1[WRITE]);
        close(pipe2[READ]);

        dup2(pipe1[READ], STDIN_FILENO);
        dup2(pipe2[WRITE], STDOUT_FILENO);

        close(pipe1[READ]);
        close(pipe2[WRITE]);

        char *args[] = {const_cast<char *>("./calc"),
                        shmidChar,
                        msgid1Char, nullptr};
        if (execvp(args[0], args) < 0) {
            perror("Cannot exec calc process");
            exit(-6);
        }
    }

    if ((displayPid = fork()) == -1) {
        perror("Cannot fork display process");
        exit(-7);
    } else if (displayPid == 0) { //display child
        close(pipe1[READ]);
        close(pipe1[WRITE]);
        close(pipe2[WRITE]);

        dup2(pipe2[READ], STDIN_FILENO);

        close(pipe2[READ]);

        char *args[] = {const_cast<char *>("./display"),
                        shmidChar,
                        msgid1Char,
                        msgid2Char, nullptr};
        if (execvp(args[0], args) < 0) {
            perror("Cannot exec display process");
            exit(-8);
        }
    }

    close(pipe1[READ]);
    close(pipe2[READ]);
    close(pipe2[WRITE]);

    while (true) {
        double xMin, xMax, yMin, yMax;
        int nCols, nRows, maxIters = 100;
        string fName;
        FilenameMessage fMessage;

        cout << "xMin (less than -2 to exit): ";
        cin >> xMin;

        if (xMin < -2.0) {
            break;
        }

        cout << "xMax:";
        cin >> xMax;
        cout << "yMin:";
        cin >> yMin;
        cout << "yMax";
        cin >> yMax;
        cout << "nRows";
        cin >> nRows;
        cout << "nCols:";
        cin >> nCols;
        cout << "filename (enter to skip):";
        getline(cin, fName);//ignore newline from last answer
        getline(cin, fName);
        cout << endl;

        strncpy(fMessage.filename, fName.c_str(), 250);

        if (msgsnd(msgid2, &fMessage, sizeof(FilenameMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send file message to display");
        }

        dprintf(pipe1[WRITE], "%f\n%f\n%f\n%f\n%d\n%d\n%d\n", xMin, xMax, yMin, yMax, nRows, nCols, maxIters);

        DoneMessage done{};
        msgrcv(msgid1, &done, sizeof(DoneMessage) - sizeof(long int), DONEMESSAGETYPE, 0);
        msgrcv(msgid1, &done, sizeof(DoneMessage) - sizeof(long int), DONEMESSAGETYPE, 0);
        cout << "Both children are done" << endl;
    }

//    xMin = -2.0;
//    xMax = 2.0;
//    yMin = -1.5;
//    yMax = 1.5;
//    nRows = 50;
//    nCols = 80;
//    maxIters = 100;
//
//    xMin=-0.65;
//    xMax = -0.5;
//    yMin = 0.5;
//    yMax = 0.65;
//    nRows = 50;
//    nCols = 80;
//    maxIters = 100;


//
//    cout << "Press enter to continue: " << std::endl;
//    std::cin.get();

    //close pipe write end
    close(pipe1[WRITE]);


    //dup2(mypipe[READ], stdin)
    //data = shmat(shmid, NULL, shmflg)
    //if data==(int*)-1...
    //release shared memory shmdt(shmaddr) MUST BE DONE!!!
    //use atexit()

    // ipcs -a show all resources
    // ipcrm remove resource
    // -m shmid
    // -q msgid
    // -s semid


    //message queue key = IPCPRIVATE flag IPC_CREATE
    // int msgget(key, flag) check if < 0
    //returns id

    // int msgsnd(msgid, const void *msgp, size_t msgsz, int msgflg)
    // msgsend(id, msg struct, size(msg)-size(long int), 0)
    // struct{
    //      long int //type
    //      anything i want except pointers (payload)
    // }msg;


    //receive message
    //ssize_t msgrcv(id, void memory pointer (where to store in memory), size allocated for memory pointer, long msgType ( can be zero), 0)
    // msgType will force revieve to grab all structs with a long int matching it
    //make sure ssize_t is the right number of bytes

    //free msg queue
    //msgctl(int msqid, IPC_RMID, NULL);

    exit(0);
}