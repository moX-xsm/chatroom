/*************************************************************************
	> File Name: chatroom.h
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 16时34分52秒
 ************************************************************************/

#ifndef _CHATROOM_H
#define _CHATROOM_H

struct Msg{
    char from[20];
    int flag;
    char message[512];
};
struct RecvMsg{
    struct Msg msg;
    int retval;
};
struct RecvMsg chat_recv(int fd);
int chat_send(struct Msg msg, int fd);
#endif
