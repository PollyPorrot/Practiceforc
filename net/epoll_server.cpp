#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/timerfd.h>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "utils/FdGuard.h"
// #include "net/MsgCodec.h"

#define SER_PORT 8080
#define SER_IP "192.168.137.132"

enum class PopResult{
    Ok,
    NeedMore,
    BadProto
};

struct Conn{
    FdGuard fd;

    std::vector<char> inbuf;
    std::vector<char> outbuf;

    // 头阶段
    bool waiting_header = false;
    std::time_t header_since = 0;

    // payload 阶段（只有读到 len 才进入）
    bool waiting_payload = false;
    std::time_t payload_since = 0;
    uint32_t expected_len = 0;   // 已解析出的 payload 长度

    explicit Conn(int rawfd) : fd(rawfd) {}
    Conn() = default;
    Conn(Conn&&) noexcept =default;
    Conn& operator=(Conn&&) noexcept =default;

    Conn(const Conn&)=delete;
    Conn& operator=(const Conn&)=delete;
};
std::unordered_map<int, Conn> conns;

// static bool isWaitingTimeOut(const Conn& c,std::time_t now,int timeout_sec){
//     return c.waiting_payload && (now - c.waiting_since >= timeout_sec);
// }

static void closeConn(int epfd ,int fd){
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    conns.erase(fd);
}

static PopResult tryPopMsg(Conn& c,std::string& msg ){
    auto& inbuf =c.inbuf;

    if(inbuf.size()<4){
        if(!inbuf.empty()&&!c.waiting_header){
            c.waiting_header=true;
            c.header_since=std::time(nullptr);
        }
        return PopResult::NeedMore;
    }

    c.waiting_header=false;
    c.header_since=0;

    uint32_t netLen=0;
    std::memcpy(&netLen,inbuf.data(),4);
    uint32_t len=ntohl(netLen);

    //上限
    const uint32_t kMax=1024*1024;
    if(len>kMax){
        return PopResult::BadProto;
    }

    //不够读,len就是payload加上长度占的4格
    if(inbuf.size()<4+len){
        if(!c.waiting_payload){
            c.waiting_payload=true;
            c.payload_since=std::time(nullptr);
            c.expected_len=len;
        }
        return PopResult::NeedMore;
    }

    msg.assign(inbuf.data()+4,inbuf.data()+4+len);
    inbuf.erase(inbuf.begin(),inbuf.begin()+4+len);

    c.waiting_payload=false;
    c.payload_since=0;
    c.expected_len=0;

    return PopResult::Ok;
}

int main(){
    // int listen_fd.get()=socket(AF_INET,SOCK_STREAM, 0);
    FdGuard listen_fd(socket(AF_INET, SOCK_STREAM, 0));
    if(listen_fd.get()<0){
        perror("socket");
        return 1;
    }

    int flags = fcntl(listen_fd.get(), F_GETFL, 0);
    fcntl(listen_fd.get(), F_SETFL, flags | O_NONBLOCK);

    int opt=1;
    setsockopt(listen_fd.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    sockaddr_in server{};
    server.sin_family=AF_INET;
    server.sin_port=htons(8080);
    server.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(listen_fd.get(), (sockaddr*)&server, sizeof(server))<0){
        perror("bind");
        return 1;
    }
    if(listen(listen_fd.get(), 5)<0){
        perror("listen");
        return 1;
    }

    // int epfd.get() = epoll_create1(0);
    FdGuard epfd(epoll_create1(0));
    if(epfd.get()<0){
        perror("epoll create");
        return 1;
    }

    // int tfd.get() = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    FdGuard tfd(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC));
    if(tfd.get()<0){
        perror("timerfd_create");
        return 1;
    }

    itimerspec its{};
    its.it_interval.tv_sec=1;  // 周期
    its.it_value.tv_sec=1;     // 第一次出发时间
    
    if(timerfd_settime(tfd.get(), 0, &its, nullptr)<0){
        perror("timerfd_settime");
        return 1;
    }

    epoll_event tev{};
    tev.events=EPOLLIN;
    tev.data.fd=tfd.get();
    if(epoll_ctl(epfd.get(), EPOLL_CTL_ADD, tfd.get(), &tev)<0){
        perror("epoll_ctl timerfd");
        return 1;
    }

    epoll_event ev{};
    ev.events=EPOLLIN;
    ev.data.fd=listen_fd.get();
    if(epoll_ctl(epfd.get(), EPOLL_CTL_ADD, listen_fd.get(), &ev)<0){
        perror("epoll_ctl");
        return 1;
    }
    epoll_event events[10];
    int size =sizeof(events)/sizeof(events[0]);
    while(1){
        int n=epoll_wait(epfd.get(), events, size, -1);
        if(n<0){
            perror("epoll_wait");
            return 1;
        }

        // std::time_t now =std::time(nullptr);
        // for(auto it=conns.begin();it!=conns.end();){
        //     int fd=it->first;
        //     Conn c=it->second;

        //     if(isWaitingTimeOut(c, now, 5)){
        //         std::cout<<"timeout: closing fd= "<<fd<<std::endl;
        //         epoll_ctl(epfd.get(), EPOLL_CTL_DEL, fd, nullptr);
        //         close(fd);
        //         it=conns.erase(it);
        //     }else{
        //         ++it;
        //     }
        // }

        for(int i=0;i<n;i++){
            if(events[i].data.fd==tfd.get()){
                uint64_t exp;
                ssize_t r = read(tfd.get(), &exp, sizeof(exp));
                // (void)r;                                   // 告诉编译器，不会用这个变量
                if(r<0 && errno !=EAGAIN){
                    perror("read timer");
                }
                
                std::time_t now =std::time(nullptr);
                for(auto it=conns.begin();it!=conns.end();){
                    int fd=it->first;
                    Conn& c=it->second;
                    ++it;
                    if(c.waiting_payload && (now -c.payload_since)>=5){
                        std::cout<<"timeout: closing fd= "<<fd<<"-waiting payload len="<<c.expected_len<<std::endl;
                        closeConn(epfd.get(),fd);
                    }else if (c.waiting_header && (now - c.header_since)>=15) {
                        std::cout<<"timeout: closing fd= "<<fd<<"-incomplete header"<<std::endl;
                        closeConn(epfd.get(), fd);
                    }
                }
                continue;
            }else if(events[i].data.fd==listen_fd.get()){
                // 用while循环进行多次连接，避免一次epoll可能发来很多次连接
                while (true) {
                    sockaddr_in client{};
                    socklen_t client_len = sizeof(client);
                    int conn_fd = accept(listen_fd.get(), (sockaddr*)&client, &client_len);

                    if (conn_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 这一波连接都接完了
                            break;
                        }
                        if (errno == EINTR) {
                            continue;
                        }
                        perror("accept");
                        break;
                    }

                    // conn_fd 设置非阻塞
                    int flags = fcntl(conn_fd, F_GETFL, 0);
                    fcntl(conn_fd, F_SETFL, flags | O_NONBLOCK);

                    printf("Client connected\n");

                    epoll_event cev{};
                    cev.events = EPOLLIN;
                    cev.data.fd = conn_fd;

                    if (epoll_ctl(epfd.get(), EPOLL_CTL_ADD, conn_fd, &cev) < 0) {
                        perror("epoll_ctl conn_fd");
                        ::close(conn_fd);
                        continue;
                    }

                    auto [it, ok] = conns.emplace(conn_fd, Conn(conn_fd));
                    if (!ok) {
                        epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                        ::close(conn_fd);
                        continue;
                    }


                }
            }else if (events[i].events & EPOLLIN) {
                int conn_fd = events[i].data.fd;
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    closeConn(epfd.get(), conn_fd);
                    continue;
                }

                // std::string msg;
                // if (!net::recvMsg(conn_fd, msg)) {
                //     // 出错或对端关闭：从 epoll 删除 + 关闭
                //     epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                //     close(conn_fd);
                //     continue;
                // }

                // std::cout << "server got: " << msg << std::endl;
                while (true) {
                    char buf[1024];
                    ssize_t n = recv(conn_fd, buf, sizeof(buf), 0);

                    if (n > 0) {
                        // // 现在：先别解析，先看看能不能连续读
                        // write(1, buf, n); // 打印到终端

                        // 解析,往后追加
                        // auto& c =conns[conn_fd];
                        auto it =conns.find(conn_fd);
                        if(it==conns.end()){
                            break;
                        }
                        auto& c=it->second;
                        c.inbuf.insert(c.inbuf.end(),buf,buf+n);

                        // 用while来trypopmsg是为了处理粘包问题
                        // [10头+10体][5头+5体]
                        while (true) {
                            std::string msg;
                            PopResult pr=tryPopMsg(c, msg);

                            if(pr==PopResult::Ok){
                                std::cout<<"msg from :"<<conn_fd<<" :"<<msg<<std::endl;
                                // auto& outbuf=conns[conn_fd].outbuf;
                                auto &outbuf= c.outbuf;

                                uint32_t len=msg.size();
                                uint32_t netLen =htonl(len);

                                outbuf.insert(outbuf.end(),reinterpret_cast<char*>(&netLen),reinterpret_cast<char*>(&netLen)+4);
                                outbuf.insert(outbuf.end(),msg.begin(),msg.end());

                                // 补上读
                                epoll_event ev{};
                                ev.events =EPOLLIN | EPOLLOUT;
                                ev.data.fd=conn_fd;
                                epoll_ctl(epfd.get(), EPOLL_CTL_MOD, conn_fd, &ev);
                                
                                continue;
                            }

                            if(pr==PopResult::NeedMore){
                                break;
                            }

                            std::cout<<"bad proto: closing fd = "<<conn_fd<<std::endl;
                            closeConn(epfd.get(), conn_fd);
                            break;
                        }
                    }
                    else if (n == 0) {
                        // 对端关闭
                        // epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                        // conns.erase(conn_fd);
                        // close(conn_fd);
                        closeConn(epfd.get(), conn_fd);
                        break;
                    }
                    else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 现在没数据了，回到 epoll_wait
                            break;
                        }
                        if (errno == EINTR) {
                            continue;
                        }
                        // 真错误
                        // epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                        // conns.erase(conn_fd);
                        // close(conn_fd);
                        closeConn(epfd.get(),conn_fd);
                        break;
                    }
                }


                // if (!net::sendMsg(conn_fd, "reply from server\n")) {
                //     epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                //     close(conn_fd);
                //     continue;
                // }
            }else if(events[i].events & EPOLLOUT){
                int conn_fd=events[i].data.fd;
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    closeConn(epfd.get(), conn_fd);
                    continue;
                }

                // auto& outbuf =conns[conn_fd].outbuf;
                auto it = conns.find(conn_fd);
                if (it == conns.end()) continue;
                auto& outbuf = it->second.outbuf;


                while(!outbuf.empty()){
                    ssize_t n= send(conn_fd, outbuf.data(), outbuf.size(), 0);

                    if(n>0){
                        outbuf.erase(outbuf.begin(),outbuf.begin() +n);
                    }else if(n==0){
                        break;
                    }else if(n<0){
                        if(errno == EAGAIN ||errno == EWOULDBLOCK){
                            break;
                        }

                        if(errno ==EINTR){
                            continue;
                        }

                        // epoll_ctl(epfd.get(), EPOLL_CTL_DEL, conn_fd, nullptr);
                        // conns.erase(conn_fd);
                        // close(conn_fd);
                        closeConn(epfd.get(), conn_fd);
                        break;
                    }
                }

                // 发完了，关掉out权限
                if(outbuf.empty()){
                    epoll_event ev{};
                    ev.events=EPOLLIN;
                    ev.data.fd=conn_fd;
                    epoll_ctl(epfd.get(), EPOLL_CTL_MOD, conn_fd, &ev);
                }
            }

        }
    }
    return 0;
}
