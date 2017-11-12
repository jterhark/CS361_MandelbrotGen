#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <algorithm>

#define READ 0
#define WRITE 1
#define SHMAX 4000

//todo set signal handler for SIGCHLD

using namespace std;

int main() {
    int msgid1, msgid2, shmid, pipe1[2], pipe2[2];
    pid_t calcPid, displayPid;

    char msgid1Char[sizeof(int)],
            msgid2Char[sizeof(int)],
            shmidChar[sizeof(int)];

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


    write(pipe1[WRITE], "Hello from mandelbrot\n", 22);

    int status;

    wait4(calcPid, &status, WUNTRACED, nullptr);
    std::cerr << "Calc processes exited with code " << WEXITSTATUS(status) << std::endl;

    wait4(displayPid, &status, WUNTRACED, nullptr);
    cerr << "Display process exited with code " << WEXITSTATUS(status) << std::endl;

    cerr << "Press enter to continue: " << std::endl;
    std::cin.get();

    //close pipe write end
    close(pipe1[WRITE]);

    //free message queues
    if (msgctl(msgid1, IPC_RMID, nullptr) < 0) {
        perror("Cannot free first message queue");
    }

    if (msgctl(msgid2, IPC_RMID, nullptr) < 0) {
        perror("Cannot free second message queue");
    }

    //free shared memory
    if (shmctl(shmid, IPC_RMID, nullptr) < 0) {
        perror("Cannot free shared memory: ");
    }

//
//    pid_t calcPid, displayPid;
//
//    if((calcPid=fork())==-1){
//        perror("Cannot fork");
//
//    }else if(calcPid==0){//child
//        char pipeRead[sizeof(int)+1];
//        sprintf(pipeRead, "%d", pipes[READ]);
//        char* args[] = {const_cast<char *>("./calc.exe"), pipeRead ,nullptr};
//        if(execvp(args[0], args)<0){
//            perror("cannot exec calc");
//            exit(-2);
//        };
//    }
//
//    write(pipes[WRITE], "ASDF\n", 5);
//
//    if((displayPid=fork())==-1){
//        perror("Cannot fork display");
//        exit(-3);
//    }else if(displayPid==0){
//        char* args[] = {const_cast<char *>("./display.exe"), nullptr};
//        if(execvp(args[0], args)<0){
//            perror("cannot exec display");
//            exit(-2);
//        };
//    }
//
//    while(waitpid(calcPid, nullptr, WNOHANG)>0);
//    while(waitpid(displayPid, nullptr, WNOHANG)>0);

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


    return 0;
}