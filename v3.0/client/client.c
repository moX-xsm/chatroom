/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 21时06分40秒
 ************************************************************************/

#include"../common/head.h"
#include"../common/chatroom.h"
#include"../common/tcp_client.h"
#include"../common/common.h"
#include"../common/chatroom.h"
#include"../common/color.h"
char *conf = "./client.conf";
char logfile[50] = {0};
int sockfd;

void handle(int signalnum){
    close(sockfd);
    printf("您已退出\n");
    exit(1);
}

int main(){
    int port;
    char ip[20] = {0};
    struct Msg msg;
    port = atoi(get_value(conf, "SERVER_PORT"));
    strcpy(ip, get_value(conf, "SERVER_IP"));
    strcpy(logfile, get_value(conf, "LOG_FILE"));
    //printf("ip = %s, port = %d\n", ip, port);
    
    

    if((sockfd = socket_connect(ip, port)) < 0){
        perror("socke_connect");
        return 1;
    }
    printf("socket_connect success\n");
    strcpy(msg.from, get_value(conf, "MY_NAME"));
    msg.flag = 2;
    
    if(chat_send(msg, sockfd) < 0){
        return 2;
    }
    
    struct RecvMsg rmsg = chat_recv(sockfd);
    if(rmsg.retval < 0){
        fprintf(stderr, "ERROE : server!\n");
        return 1;
    }

    printf(RED"Server:"NONE"%s\n", rmsg.msg.message);
    if(rmsg.msg.flag == 3){
        //printf(RED"Server confused! Maybe name reuse!\n,"NONE);
        close(sockfd);
        return 1;
    }
    signal(SIGINT, handle);
    pid_t pid;
    if((pid = fork()) < 0){
        perror("fork");
        return 1;
    }
    if(pid == 0){
        sleep(2);
        system("clear");
        while(1){
            memset(msg.message, 0, sizeof(msg.message));
            printf(L_PINK"Please input message :\n "NONE);
            scanf("%[^\n]s", msg.message);
            getchar();
            msg.flag = 0;
            if(msg.message[0] == '@'){
                msg.flag = 1;
            }
            chat_send(msg, sockfd);
            system("clear");
        }
        close(sockfd);
    }else{
        freopen(logfile, "w+", stdout);
        //FILE *log_fp = fopen(logfile, "w");
        while(1){
            rmsg = chat_recv(sockfd);
            if(rmsg.msg.flag == 0){
                printf(L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
            }else if(rmsg.msg.flag == 2){
                printf(L_YELLOW"NOTICE"NONE": %s\n", rmsg.msg.message);
            }else if(rmsg.msg.flag == 1){
                printf(L_GREEN"<private> "L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
            }else{
                printf("error\n");
            }
            fflush(stdout);
        }
        wait(NULL);
        close(sockfd);
    }
    return 0;
}
