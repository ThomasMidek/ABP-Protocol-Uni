#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#define MSG_LENG 1

/*pipe-creation*/
int pipe1[2], pipe2[2];

/*sigaction struct creation*/
struct sigaction actioninitiator = {0};
struct sigaction actionresponder = {0};

void abp_initiator(){
    char msg_buf[1];
    if(read(pipe2[0],msg_buf,MSG_LENG)==-1){
        printf("agb_initiator error!");
    }
    else{
        printf("%s",msg_buf);
    }
}

void abp_responder(){
    char msg_buf[1];
    if(read(pipe1[0],msg_buf,MSG_LENG)==-1){
        printf("agb_responder error!");
    }
    else{
        printf("%c",msg_buf);
    }
}

void initiator(pid_t r_pid){
    char msg_buf[1]= "a";
    switch (r_pid) {

        case -1:    //error-handling
            printf("Error!");
            break;

        case 0:     //child-process
            printf("child");
            close(pipe1[1]);
            close(pipe2[0]);
            actionresponder.sa_handler = abp_responder;
            sigaction(SIGUSR1,&actionresponder,0);
            msg_buf[0]='x';
            pid_t ppid= getppid();
            while(1==1){
                pause();
                write(pipe2[1],msg_buf,MSG_LENG);
                kill(ppid,SIGUSR2);
            }
            sleep(4);
            close(pipe1[0]);
            close(pipe2[1]);
            exit(0);

        default:    //parent-process
            printf("parent");
            close(pipe1[0]);
            close(pipe2[1]);
            actioninitiator.sa_handler = abp_initiator;
            sigaction(SIGUSR2,&actioninitiator,0);
            msg_buf[0]='y';
            sleep(2);
            write(pipe1[1],msg_buf,MSG_LENG);
            kill(r_pid,SIGUSR1);
            while(1==1){
                printf("test");
                pause();
                write(pipe1[1],msg_buf,MSG_LENG);
                kill(r_pid,SIGUSR1);
            }
            sleep(4);
            close(pipe1[1]);
            close(pipe2[0]);
            exit(0);
    }
}

int main(void) {

    char parentMessage[] = "TheQuickBrownFoxOle";
    char childMessage[] = "JumpsOverTheLazyDog";

    if(pipe(pipe1)==-1||pipe(pipe2)==-1){
        printf("Pipe-Error!");
        return -1;
    }
    else{
        initiator(fork());
    }
}