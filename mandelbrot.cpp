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
#define SHMAX 16000

//todo set signal handler for SIGCHLD

using namespace std;

int   msgid1     = -1,
      msgid2     = -1,
      shmid      = -1;
pid_t calcPid    = -1,
      displayPid = -1;

void exitHandler(int signal) {
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
    if (calcPid != -1)
        kill(calcPid, SIGUSR1);

    if (displayPid != -1)
        kill(displayPid, SIGUSR1);

    int status = 0;

    //reap processes
    if (calcPid != -1) {
        if(waitpid(calcPid, &status, WUNTRACED)!=-1){
            if(WIFEXITED(status)){
                cout << "Calc process exited normally with code " << WEXITSTATUS(status) << std::endl;
            }
        }else{
            perror("waitpid calc error");
        }

    }

    status=0;
    if (displayPid != -1) {
        if(waitpid(displayPid, &status, WUNTRACED)!=-1){
            if(WIFEXITED(status)){
                cout << "Display process exited with code " << WEXITSTATUS(status) << std::endl;
            }
        }else{
            perror("waitpid display error");
        }
    }
}

int main() {
    int pipe1[2], pipe2[2];

    char msgid1Char[sizeof(int)],
         msgid2Char[sizeof(int)],
         shmidChar[sizeof(int)];

    //handle signals to make sure memory is freed
    signal(SIGINT, exitHandler);
    signal(SIGSEGV, exitHandler);
    signal(SIGTERM, exitHandler);
    signal(SIGKILL, exitHandler);
    signal(SIGCHLD, exitHandler);

    //free shared memory and message queues
    atexit(cleanup);

    cout << "Jake TerHark - jterha2" << endl << "CS361 - Mandelbrot" << endl << endl;

    //create pipes
    if (pipe(pipe1) == -1) {
        perror("Cannot create first pipe");
        exit(-3);
    }

    if (pipe(pipe2) == -1) {
        perror("Cannot create second pipe");
        exit(-4);
    }

    //create message queues
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


    //convert to ids to strings to pass as parameters
    sprintf(msgid1Char, "%d", msgid1);
    sprintf(msgid2Char, "%d", msgid2);
    sprintf(shmidChar, "%d", shmid);

    //fork children and exec
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

        char *args[] = {const_cast<char *>("./mandelCalc"),
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

        char *args[] = {const_cast<char *>("./mandelDisplay"),
                        shmidChar,
                        msgid1Char,
                        msgid2Char, nullptr};
        if (execvp(args[0], args) < 0) {
            perror("Cannot exec display process");
            exit(-8);
        }
    }

    //close unused pipe ends
    close(pipe1[READ]);
    close(pipe2[READ]);
    close(pipe2[WRITE]);

    //main loop
    while (true) {
        double xMin, xMax, yMin, yMax;
        int    nCols, nRows, maxIters;

        string          fName;
        FilenameMessage fMessage;

        //get user input
        cout << "xMin (less than -2 to exit): ";
        cin >> xMin;

        if (xMin < -2.0) {
            break;
        }

        cout << "xMax: ";
        cin >> xMax;
        cout << "yMin: ";
        cin >> yMin;
        cout << "yMax: ";
        cin >> yMax;
        cout << "nRows: ";
        cin >> nRows;
        cout << "nCols: ";
        cin >> nCols;
        cout << "Max iterations: ";
        cin >> maxIters;
        cout << "filename (enter to skip):";
        getline(cin, fName);//ignore newline from last answer
        getline(cin, fName);
        cout << endl;

        //make sure result will fit in shared memory
        if (nRows * nCols >= SHMAX) {
            cerr << "Please choose smaller values for nRows and nCols" << endl;
            continue;
        }

        //copy filename to message queue msg
        strncpy(fMessage.filename, fName.c_str(), 250);

        //send filename on message queue 2
        if (msgsnd(msgid2, &fMessage, sizeof(FilenameMessage) - sizeof(long int), 0) == -1) {
            perror("Cannot send file message to display");
        }

        //send info to calc process
        dprintf(pipe1[WRITE], "%f\n%f\n%f\n%f\n%d\n%d\n%d\n", xMin, xMax, yMin, yMax, nRows, nCols, maxIters);

        //wait for two done messages on queue 1
        DoneMessage done{};
        msgrcv(msgid1, &done, sizeof(DoneMessage) - sizeof(long int), DONEMESSAGETYPE, 0);
        msgrcv(msgid1, &done, sizeof(DoneMessage) - sizeof(long int), DONEMESSAGETYPE, 0);
        cout << endl;
    }

    //close pipe write end
    close(pipe1[WRITE]);

    // children will be killed and reaped no matter what in cleanup (at exit)
    signal(SIGCHLD, SIG_DFL);

    exit(0);
}