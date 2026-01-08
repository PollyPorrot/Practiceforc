#include <arpa/inet.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "net/MsgCodec.h"

#define SER_PORT 8080
#define SER_IP "192.168.137.132"

int main(){
    int listen_fd=socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd<0){
        perror("socket");
        return 1;
    }
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_port=htons(SER_PORT);
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(listen_fd,(sockaddr*)&addr,sizeof(addr))<0){
        perror("bind");
        close(listen_fd);
        return 1;
    }
    if(listen(listen_fd, 5)<0){
        perror("listen");
        close(listen_fd);
        return 1;
    }
    sockaddr_in client{};
    socklen_t client_len=sizeof(client);
    int conn_fd=accept(listen_fd, (sockaddr*)&client, &client_len);
    if(conn_fd<0){
        perror("accept");
        return 1;
    }
    char ip[INET_ADDRSTRLEN];
    if(!inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip))){
        perror("inet_ntop");
    }
    std::cout << "client connected: "<< ip << ":" << ntohs(client.sin_port) << std::endl;
    
    // char buf[1024]{};
    // ssize_t n=recv(conn_fd,buf,sizeof(buf)-1,0);
    // if(n<0){
    //     perror("recv");
    //     close(conn_fd);
    //     return 1;
    // }
    // if(n==0){
    //     std::cout<<"client closed\n";
    //     close(conn_fd);
    //     return 0;
    // }
    // std::cout<<"server received: "<<buf<<std::endl;
    // const char* reply = "HAHAHA,from server\n";
    // ssize_t m=send(conn_fd,reply,strlen(reply),0);
    // if(m<0){
    //     perror("send");
    //     close(conn_fd);
    //     return 1;
    // }

    std::string msg;
    if (!net::recvMsg(conn_fd, msg)) {
        std::cout << "recvMsg failed or peer closed\n";
        close(conn_fd);
        close(listen_fd);
        return 0;
    }
    std::cout << "server got: " << msg << std::endl;
    if (!net::sendMsg(conn_fd, "reply from server\n")) {
        std::cout << "sendMsg failed\n";
    }

    close(conn_fd);
    close(listen_fd);

    return 0;
}