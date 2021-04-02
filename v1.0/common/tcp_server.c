/*************************************************************************
	> File Name: tcp_server.c
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 15时40分16秒
 ************************************************************************/

#include"./head.h"

int socket_create(int port){
    printf("in socket_create : %d\n", port);
    int server_listen;
    if((server_listen = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        return -1;
    }
    //设置暴力关闭 延时设置为0表示立刻关闭
    struct linger m_linger;
    m_linger.l_onoff = 1;
    m_linger.l_linger = 0;
    if((setsockopt(server_listen, SOL_SOCKET, SO_LINGER, &m_linger, (socklen_t)sizeof(m_linger))) < 0){
        perror("setsockopt");
        return -1;
    }

    //地址重用
    int flag = 1;
    if(setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0){
        perror("setsockopt");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_listen, (struct sockaddr *)(&addr), sizeof(struct sockaddr)) == -1){
        perror("bind");
        return -1;
    }

    if(listen(server_listen, 20) == -1){
        perror("listen");
        return -1;
    }
    return server_listen;

}
