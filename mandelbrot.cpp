#include <iostream>
#include <unistd.h>
#include <wait.h>

#define READ 0
#define WRITE 1


int main() {


    int pipes[2];
    if(pipe(pipes)==-1){
        perror("Cannot create pipe");
        exit(-1);
    }

    pid_t calcPid, displayPid;

    if((calcPid=fork())==-1){
        perror("Cannot fork");

    }else if(calcPid==0){//child
        char pipeRead[sizeof(int)+1];
        sprintf(pipeRead, "%d", pipes[READ]);
        char* args[] = {const_cast<char *>("./calc.exe"), pipeRead,nullptr};
        if(execvp(args[0], args)<0){
            perror("cannot exec calc");
            exit(-2);
        };
    }

    write(pipes[WRITE], "ASDF\n", 5);

    if((displayPid=fork())==-1){
        perror("Cannot fork display");
        exit(-3);
    }else if(displayPid==0){
        char* args[] = {const_cast<char *>("./display.exe"), nullptr};
        if(execvp(args[0], args)<0){
            perror("cannot exec display");
            exit(-2);
        };
    }

    while(waitpid(calcPid, nullptr, WNOHANG)>0);
    while(waitpid(displayPid, nullptr, WNOHANG)>0);

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