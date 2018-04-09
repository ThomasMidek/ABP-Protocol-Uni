#include <stdio.h>;
#include <stdlib.h>;
#include <string.h>;
#include <sys/wait.h>;
#include <sys/types.h>;
#include <signal.h>;
#include <unistd.h>;

#define MSG_LENG 1

/*pipe-creation*/
int pipe1[2], pipe2[2];

/*sigaction struct creation*/
struct sigaction actioninitiator = {0};
struct sigaction actionresponder = {0};

void initiator(void){
    char msg_buf[2];
    pid_t r_pid=fork();
    switch (r_pid) {
        case -1:    //error-handling
            printf("Error!");
            break;

        case 0:     //child-process
            close(pipe1[1]);
            close(pipe2[0]);
            actionresponder.sa_handler = abp_responder;
            sigaction(SIGUSR1,&actionresponder,0);
            pid_t ppid= getppid();
            break;

        default:    //parent-process
            close(pipe1[0]);
            close(pipe2[1]);
            actioninitiator.sa_handler = abp_initiator;
            sigaction(SIGUSR2,&actioninitiator,0);
            sleep(2);
            write(pipe1[1],msg_buf,MSG_LENG);
            int n = kill(r_pid,SIGUSR1);
            while(n==0){
                pause();
                write(pipe1[1],msg_buf,MSG_LENG);
                n = kill(r_pid,SIGUSR1);
            }
            sleep(4);
            close(pipe1[1]);
            close(pipe2[0]);
    }
}

void abp_initiator(){
    char msg_buf[2];
    if(read(pipe2[0],msg_buf,MSG_LENG)==-1){
        printf("agb_initiator error!");
    }
    else{
            printf("%s",msg_buf);
    }
}

void abp_responder(){
    char msg_buf[2];
    if(read(pipe1[0],msg_buf,MSG_LENG)==-1){
        printf("agb_responder error!");
    }
    else{
        printf("%s",msg_buf);
    }
}

int main(void) {

    char parentMessage[] = "TheQuickBrownFoxOle";
    char childMessage[] = "JumpsOverTheLazyDog";

    if(pipe(pipe1)==-1||pipe(pipe2)==-1){
        printf("Pipe-Error!");
        return -1;
    }
    initiator()
}