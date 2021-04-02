/*************************************************************************
	> File Name: chatroom.c
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 21时40分36秒
 ************************************************************************/

#include"head.h"
#include"chatroom.h"
int chat_send(struct Msg msg, int fd){
    if(send(fd, &msg, sizeof(msg), 0) <= 0){
        return -1;
    }
    return 0;
}

struct RecvMsg chat_recv(int fd){
    struct RecvMsg tmp;
    memset(&tmp, 0, sizeof(tmp));
    if(recv(fd, &tmp.msg, sizeof(struct Msg), 0) <= 0){
        tmp.retval = -1;
    }
    return tmp;

}

