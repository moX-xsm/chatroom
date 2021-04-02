/*************************************************************************
	> File Name: server/server.c
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 16时23分18秒
 ************************************************************************/

#include"../common/head.h"
#include"../common/common.h"
#include"../common/tcp_server.h"
#include"../common/chatroom.h"
#include"../common/color.h"

#define MAX_CLIENT 512

char *conf = "./server.conf";

struct User{
    char name[20];
    int online;
    pthread_t tid;
    int fd;
};

struct User *client;
int sum = 0;

bool check_online(char *name){
    for(int i = 0; i < MAX_CLIENT; i++){
        if(client[i].online && !strcmp(name, client[i].name)){
            return true;
        }
    }
    return false;
}

int find_sub(){
    for(int i = 0; i < MAX_CLIENT; i++){
        if(!client[i].online) return i;
    }
    return -1;
}

void send_all(struct Msg msg){
    for(int i = 0; i < MAX_CLIENT; i++){
        if(client[i].online == 0) continue;
        chat_send(msg, client[i].fd);
    }
}

int check_name(char *name){
    for(int i = 0; i < MAX_CLIENT; i++){
        if(client[i].online && !strcmp(client[i].name, name))
            return i;
    }
    return -1;
}

void handle(int sig){
    struct Msg msg;
    msg.flag = 3;
    for(int i = 0; i < MAX_CLIENT; i++){
        if(client[i].online){
            chat_send(msg, client[i].fd);
            close(client[i].fd);
        }
    }
    exit(0);
}

void *work(void *arg){
    int sub = *(int *)arg;
    int client_fd = client[sub].fd;
    struct RecvMsg rmsg;
    printf(GREEN"Login : "NONE"%s\n", client[sub].name);
    while(1){
        rmsg = chat_recv(client_fd);
        if(rmsg.retval < 0){
            printf(PINK"Logout : %s\n"NONE, client[sub].name);
            close(client_fd);
            client[sub].online = 0;
            sum--;
            return NULL;
        }
        //printf("flag = %d\n", rmsg.msg.flag);
        printf(BLUE"%s"NONE": %s\n", rmsg.msg.from, rmsg.msg.message);
        //printf("flag = %d\n", rmsg.msg.flag);
        if(rmsg.msg.flag == 0){
            send_all(rmsg.msg);
        }else if(rmsg.msg.flag == 1){
            
            if(rmsg.msg.message[0] == '@'){
                char to[20];
                int i;
                for(i = 0; i < 20; i++){
                    if(rmsg.msg.message[i] == ' ') break;
                }
                strncpy(to, rmsg.msg.message + 1, i - 1);
                int ind;
                if((ind = check_name(to)) < 0){
                    sprintf(rmsg.msg.message, "%s is not online.", to);
                    rmsg.msg.flag = 2;
                    chat_send(rmsg.msg, client_fd);
                    continue;
                }
                rmsg.msg.flag = 1;
                chat_send(rmsg.msg, client[ind].fd);
            }
        }else if(rmsg.msg.flag == 4){
            if(rmsg.msg.message[0] == '*' && rmsg.msg.message[1] == '@'){
                char to[20];
                strcpy(to, rmsg.msg.message + 2);
                printf("picture to %s\n", to);
                FILE *fp = fopen("new.jpg", "wb");
                char buff[1024] = {0};
                while(1){
                    int n = read(client_fd, buff, 1024);
                    printf("n = %d\n", n);
                    if(n == 0) break;
                    fwrite(buff, 1, n, fp);
                }
                printf("picture translate done!\n");
                fclose(fp);
            }
        }
    }
    printf("pthread end : %d\n", client_fd);
    //printf(BLUE"%s login!\n"NONE, recvmsg.msg.from);
    return NULL;
}

int main(){
    int port, server_listen, fd;
    struct RecvMsg recvmsg;
    struct Msg msg;
    port = atoi(get_value(conf, "SERVER_PORT"));
    //printf("port = %d\n", port);
    client = (struct User *)calloc(MAX_CLIENT, sizeof(struct User));
    if((server_listen = socket_create(port)) < 0){
        perror("socket_create");
        return 1;
    }

    signal(SIGINT, handle);

    while(1){
        if((fd = accept(server_listen, NULL, NULL)) < 0){
            perror("accept");
            continue;
        }
        
        recvmsg = chat_recv(fd);
        if(recvmsg.retval < 0){
            close(fd);
            continue;
        } 
        if(check_online(recvmsg.msg.from)){
            //拒绝连接
            msg.flag = 3;
            strcpy(msg.message, "You have Already Logined!");
            chat_send(msg, fd);
            close(fd);
            printf(YELLOW"D :" NONE"%s have been used!\n", recvmsg.msg.from);
            continue;
            
        }

        msg.flag = 2;
        strcpy(msg.message, "Welcome to this chatroom!");
        chat_send(msg, fd);
        int sub;
        sub = find_sub();
        client[sub].online = 1;
        client[sub].fd = fd;
        strcpy(client[sub].name, recvmsg.msg.from);
        sum++;
        pthread_create(&client[sub].tid, NULL, work, (void *)&sub);
    }
    return 0;
}
