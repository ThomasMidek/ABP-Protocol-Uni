#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MSG_LEN 2
#define BUF_LEN 2
#define RAND 0.2

struct sigaction actioninitiator = {0};
struct sigaction actionresponder = {0};
struct sigaction actiontimer = {0};

int pipe1[2], pipe2[2];
int signummer;
int retransmissionCounter = 0;

void abp_initiator(int signr){
    signummer = signr;
    char msg_buf[BUF_LEN];
    read(pipe2[0],&msg_buf,MSG_LEN);
    alarm(0);
    printf("Initiator erhält: %c | %c\n\n",msg_buf[0], msg_buf[1]);
}

void abp_responder(){
    char msg_buf[BUF_LEN];
    read(pipe1[0],&msg_buf,MSG_LEN);
    printf("Responder erhält: %c | %c\n\n",msg_buf[0],msg_buf[1]);
}

void abp_timer(int signr){
    signummer = signr;
    retransmissionCounter++;
    printf("An Error has occured! Retransmitting...\n\n");
}

void send(int pipeAdress,char control, char message){
    char msg_buf[BUF_LEN];
    msg_buf[0] = message;
    msg_buf[1] = control;
    write(pipeAdress,&msg_buf,MSG_LEN);
}

int main() {
    //both text segments
    char send_msg[] = ("TheQuickBrownFoxOle");
    char respond_msg[] = ("JumpsOverTheLazyDog");

    //countervariables
    int i;
    int state;

    //pipes
    pipe(pipe1);
    pipe(pipe2);

    //random number generator
    srand(time(NULL));

    //process id for parent/child determination
    pid_t r_pid;

    if((r_pid = fork())<0){
        //error handling
        printf("Error!");
        return 1;
    }
    else if(r_pid>0){
        //parent process coding
        i = 0;
        state = 0;

        char msg_buffer[MSG_LEN];

        close(pipe1[0]);
        close(pipe2[1]);

        //sigaction
        actioninitiator.sa_handler = abp_initiator;
        sigaction(SIGUSR2, &actioninitiator, 0);
        actiontimer.sa_handler = abp_timer;
        sigaction(SIGALRM, &actiontimer, 0);

        sleep(2);
        printf("Starting transmission...\n\n");
        while(i<strlen(send_msg)){
            switch (state){
                case 0:
                    msg_buffer[0] =  send_msg[i];
                    msg_buffer[1] = '0';
                    if(((float)rand()/(float)RAND_MAX)>=RAND) {
                        write(pipe1[1], &msg_buffer, MSG_LEN);
                        alarm(2);
                        kill(r_pid,SIGUSR1);
                        i++;
                    }
                    else {
                        alarm(2);
                    }
                    state = 1;
                    break;
                case 1:
                    pause();
                    switch(signummer){
                        case SIGUSR2:
                            msg_buffer[0] = send_msg[i];
                            msg_buffer[1] = '1';
                            if(((float)rand()/(float)RAND_MAX)>=RAND) {
                                write(pipe1[1], &msg_buffer, MSG_LEN);
                                alarm(2);
                                kill(r_pid,SIGUSR1);
                                i++;
                            }
                            else {
                                alarm(2);
                            }
                            state = 2;
                            break;
                        case SIGALRM:
                            msg_buffer[0] =  send_msg[i];
                            msg_buffer[1] = '0';
                            if(((float)rand()/(float)RAND_MAX)>=RAND) {
                                write(pipe1[1], &msg_buffer, MSG_LEN);
                                alarm(2);
                                kill(r_pid,SIGUSR1);
                                i++;
                            }
                            else{
                                alarm(2);
                            }
                            break;
                    }
                    break;
                case 2:
                    pause();
                    switch (signummer) {
                        case SIGUSR2:
                            msg_buffer[0] =  send_msg[i];
                            msg_buffer[1] = '0';
                            if(((float)rand()/(float)RAND_MAX)>=RAND) {
                                write(pipe1[1], &msg_buffer, MSG_LEN);
                                alarm(2);
                                kill(r_pid,SIGUSR1);
                                i++;
                            }
                            else {
                                alarm(2);
                            }
                            state = 1;
                            break;
                        case SIGALRM:
                            msg_buffer[0] = send_msg[i];
                            msg_buffer[1] = '1';
                            if(((float)rand()/(float)RAND_MAX)>=RAND) {
                                write(pipe1[1], &msg_buffer, MSG_LEN);
                                alarm(2);
                                kill(r_pid,SIGUSR1);
                                i++;
                            }
                            else {
                                alarm(2);
                            }
                            break;
                    }
            }
        }
        sleep(2);
        printf("Durchgeführte Retransmissionen: %d\n\n",retransmissionCounter);
        close(pipe1[1]);
        close(pipe2[0]);
        exit(0);
    }
    else{
        //child process coding
        i = 0;
        state = 1;
        close(pipe1[1]);
        close(pipe2[0]);
        actionresponder.sa_handler = abp_responder;
        sigaction(SIGUSR1, &actionresponder, 0);
        while(i<strlen(respond_msg)){
            pause();
            switch(state){
                case 1:
                    send(pipe2[1], '0', respond_msg[i]);
                    state = 2;
                    i++;
                    break;
                case 2:
                    send(pipe2[1], '1', respond_msg[i]);
                    state = 1;
                    i++;
                    break;
            }
            kill(getppid(),SIGUSR2);
        }
        sleep(2);
        close(pipe1[0]);
        close(pipe2[1]);
        exit(0);
    }
}