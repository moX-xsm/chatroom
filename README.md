广域聊天室

路径`/home/mox/1.HaiZei/6.os/7.chatroom`

# 广域聊天室整体要求

## client端

> 执行./client
>
> 请使用配置文件配置服务器监听端口等
>
> Server_ip = 192.168.1.40
>
> Server_port=8731
>
> My_name=xxxx
>
> Log_FIle=./chat.log

1. 假设A用户想给B用户发送私聊信息，可以发送`@B 这是来自A的私聊信息`
2. 所有公聊信息，服务器端会转发给Client端
3. 将`Client`收到的所有聊天信息，保存在`${Log_File}`中
4. 使用`tail -f ${Log_File}`查看文件，获取聊天信息，发送和接受独立
5. 由服务器发送到本地的数据是一个结构体，`Client`端自行解析

```c
struct Message{
    char from[20];
    int flag;//若flag为1则为私聊信息，0为公聊信息，2则为服务器的通知信息或客户端验证信息，3为客户端下线信息
    char message[1024];
};
```

## server端

> 执行./server
>
> 使用配置文件，将服务器监听端口，客户端监听端口等都写到配置文件
>
> Server_port=8731

1. Server将在`${Server_port}`上进行监听,等待用户上该端口上接受线，并在该端口上接收用户输出信息
2. Server每收到一条私聊信息，需要将消息转发给其他所有在线的用户
3. 如果用户发送的信息是一条私聊信息，则只转发给目标用户
4. 所有转发给用户的信息都将使用结构体Message进行封装
5. 私聊信息中所指定的用户不存在或已经下线，需要通知信息告知
6. 请选用合理的数据结构，储存用户信息
7. 支持100个以上的在线用户
8. 在`Client`上线时，发送欢迎信息，告知当前所有在线人数等
9. 需要考虑当两个用户在某一时刻同时上线的情况

# v1.0

```c
//发送的信息包 from为用户名 flag标记信息类型 message为信息内容
struct Msg{
    char from[20];
    int flag;
    char message[512];
};
//收到的信息包 因为下面将接收数据recv进行了打包 所以需要靠retval判断接收数据是否成功
struct RecvMsg{
    struct Msg msg;
    int retval;
};

//解析conf文件 从path拿到对应的key的value值
char *get_value(char *path, char *key);
//从fd接收数据
struct RecvMsg chat_recv(int fd);
//将msg发送到fd
int chat_send(struct Msg msg, int fd);



```

server端：

1. 创建socket连接，等待连接
2. 建立连接后先收一个是否建立连接的包 判断用户名是否被占用 如果被占用 回复类型3的数据断开连接 如果没被占用 回复类型2的信息说明新的client可以上线
3. 如果没被占用 寻找client数组最小下标记录信息 
4. 开辟线程一直接收并打印信息

```c
//服务端需要记录客户信息 name  是否在线 线程tid 建立的连接fd
struct User{
    char name[20];
    int online;
    pthread_t tid;
    int fd;
};

//判断用户名是否在线
bool check_online(char *name);
//寻找最小online为0的下标
int find_sub();
```

client端：

1. 建立socket连接
2. 连接成功后发送数据类型为2的客户端验证信息 等待接受服务端的通知信息
3. 如果通知信息类型为3 拒绝连接 关闭sockfd并退出
4. 如果通知类型是2不是3 则成功建立连接 
5. 开辟新的进程用于发送自己的信息
6. 主进程等待子进程结束

bug：

1. 1111和xsm上线 下线时的名称都是xsm ？？？？

   ![image-20210402112450957](https://gitee.com/xsm970228/images2020.9.5/raw/master/20210402112935.png)

2. ctrl + c 不能进行退出

# v2.0

bug解决：

1. 1111和xsm上线  下线时的名称都是xsm

   ```c
   //server mian
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
           pthread_create(&client[sub].tid, NULL, work, (void *)&sub);
           
   
       }
   
       return 0;
   }
   //线程处理函数
   void *work(void *arg){
       int *sub = (int *)arg;
       int client_fd = client[*sub].fd;
       struct RecvMsg rmsg;
       printf(GREEN"Login : "NONE"%s\n", client[*sub].name);
       while(1){
           rmsg = chat_recv(client_fd);
           if(rmsg.retval < 0){
               printf(PINK"Logout : %s\n"NONE, client[*sub].name);
               close(client_fd);
               client[*sub].online = 0;
               return NULL;
           }
           printf(BLUE"%s"NONE": %s\n", rmsg.msg.from, rmsg.msg.message);
       }
       //printf(BLUE"%s login!\n"NONE, recvmsg.msg.from);
       return NULL;
   }
   
   
   
   ```

   线程处理函数的sub地址指向主函数中的sub  而每次有新的客户上线 则主函数中的sub值就会改变 而线程处理函数中一直用的是地址 所以它指向的地址的取值是一直改变的

   解决：线程处理函数中将主函数中的值copy一份 而不是直接用同一个地址

   ```c
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
               return NULL;
           }
           printf(BLUE"%s"NONE": %s\n", rmsg.msg.from, rmsg.msg.message);
       }
       //printf(BLUE"%s login!\n"NONE, recvmsg.msg.from);
       return NULL;
   }
   ```

2. ctrl + c 不能进行退出

   ```c
   //将sockfd变为全局变量
   int sockfd;
   
   void handle(int signalnum){
       close(sockfd);
       exit(1);
   }
   
   //client子线程
   signal(SIGINT, handle);
   //在client子线程注册ctrl+c的SIGINT信号 当有信号来时 关闭sockfd
   ```

server端：

1. 不断的接收client的消息 判断消息类型是否为0（公有消息）如果是0就转发给所有在线的人
2. 如果是其他的 另外处理

```c
//将公有消息转发给所有在线的人
void send_all(struct Msg msg)；
```

client端：

1. 子进程发送消息 主进程接收消息并将接收到的消息写入文件中
2. tail -f chat.log查看接受到的消息

bug：为什么明明已经转发了公有消息文件中还是没有接收到消息呢？

![image-20210402143207196](https://gitee.com/xsm970228/images2020.9.5/raw/master/20210402143212.png)

# v3.0

bug解决：为什么明明已经转发了公有消息文件中还是没有接收到消息呢？

```c
//client主进程
FILE *log_fp = fopen(logfile, "w");
        while(1){
            rmsg = chat_recv(sockfd);
            fprintf(log_fp, "%s : %s\n", rmsg.msg.from, rmsg.msg.message);
        }
        wait(NULL);
        close(sockfd);

```

因为fopen是标准IO，全缓冲，需要达到全缓冲的阈值才会写进文件中

只要在fprintf后fllush一下就可以了`fllush(log_fp);`

server端：

1. 当接收到的信息是私有信息时，首先判断目标用户是否在线
2. 如果没在线 给客户端发送类型2的通知信息告知未在线
3. 如果在线 将信息转发给目标用户

```c
//找到目标用户的index
int check_name(char *name);
```

client端：

1. 收到的信息进行分类

2. 0公聊信息

3. 1私聊信息

4. 2通知信息

   

bug:

1. 当server先断开时，chatlog会被无限打印空值

   ![image-20210402192244202](https://gitee.com/xsm970228/images2020.9.5/raw/master/20210402192247.png)

2. 当server断开后 client仍没有退出

   ![image-20210402192416490](/home/mox/图片/Typora/image-20210402192416490.png)

# v4.0

bug解决：

1. 当server先断开时，chatlog会被无限打印空值

   ```c
   //client 用于接收的主进程的逻辑
   //没有判断chat_recv是否成功
   //加上程序的第7行即可
   freopen(logfile, "w+", stdout);
           //FILE *log_fp = fopen(logfile, "w");
           while(1){
               rmsg = chat_recv(sockfd);
               //if(rsmg.retval < 0) break;
               if(rmsg.msg.flag == 0){
                   printf(L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
               }else if(rmsg.msg.flag == 2){
                   printf(L_YELLOW"NOTICE"NONE": %s\n", rmsg.msg.message);
               }else if(rmsg.msg.flag == 1){
                   printf(L_GREEN"<private> "L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
               }else if(rmsg.msg.flag == 3){
                   printf(RED"Server end\n"NONE); 
                   break;
               }else{
                   printf("error\n");
               }
               fflush(stdout);
           }
           wait(NULL);
           close(sockfd);
   
   ```

   

2. 当server断开后 client仍没有退出

   当服务端来了SIGINT信号，我们需要注册信号 当SIGINT信号来时进入handle处理函数 

   函数需要将所有在线的fd关闭并发出类型3的数据包进行客户端下线

   客户端有两个进程 只有接收端下线还不够

   接收端即为主进程，当主进程下线后应该给子进程发送9信号

   ```c
   //server 信号处理函数
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
   
   //client 接收端逻辑
   else{
           freopen(logfile, "w+", stdout);
           //FILE *log_fp = fopen(logfile, "w");
           while(1){
               rmsg = chat_recv(sockfd);
               if(rmsg.retval < 0) break;
               if(rmsg.msg.flag == 0){
                   printf(L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
               }else if(rmsg.msg.flag == 2){
                   printf(L_YELLOW"NOTICE"NONE": %s\n", rmsg.msg.message);
               }else if(rmsg.msg.flag == 1){
                   printf(L_GREEN"<private> "L_BLUE"%s"NONE" : %s\n", rmsg.msg.from, rmsg.msg.message);
               }else if(rmsg.msg.flag == 3){
                   printf(RED"Server end\n"NONE); 
                   break;
               }else{
                   printf("error\n");
               }
               fflush(stdout);
           }
           kill(pid, 9);
           wait(NULL);
           close(sockfd);
       }
   
   
   ```

   




