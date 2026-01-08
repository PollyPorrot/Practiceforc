#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cerrno>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include "net/MsgCodec.h"

int main(){
    int fd=socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        perror("socket");
        return 1;
    }
    sockaddr_in server{};
    server.sin_family=AF_INET;
    server.sin_port=htons(8080);
    if(inet_pton(AF_INET, "127.0.0.1", &server.sin_addr)<=0){
        perror("inet_pton");
        close(fd);
        return 1;
    }

    if(connect(fd, (sockaddr*)&server, sizeof(server))<0){
        perror("connect");
        close(fd);
        return 1;
    }

    // const char *msg="HA,from slient\n";
    // send(fd,msg,strlen(msg),0);

    // char buf[1024]{};
    // memset(buf,0,sizeof(buf));
    // ssize_t n=recv(fd, buf, sizeof(buf)-1, 0);
    // if(n>0){
    //     std::cout<<"reply: "<<buf;
    // }else if(n==0){
    //     std::cout<<"server closed\n";
    // }else{
    //     perror("recv");
    //     return 1;
    // }

    if (!net::sendMsg(fd, "hello length-prefix")) {
        std::cout << "sendMsg failed\n";
        return 1;
    }
    send(fd, "a", 1, 0);
    std::string reply;
    if (!net::recvMsg(fd, reply)) {
        std::cout << "recvMsg failed\n";
        return 1;
    }
    std::cout << "client got: " << reply << std::endl;
    close(fd);
    return 0;
}