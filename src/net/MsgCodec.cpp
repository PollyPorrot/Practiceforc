#include <cerrno>
#include <cstddef>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include "net/MsgCodec.h"

namespace net {
    bool recvAll(int fd,void* buf,size_t n){
        char* p=static_cast<char*>(buf);
        size_t left=n;
        while(left>0){
            ssize_t r=recv(fd,p,left,0);
            if(r<0){
                if(errno == EINTR){
                    continue;
                }else{
                    return false;
                }
            }else if(r==0){
                return false;
            }else{
                p+=r;
                left-=r;
            }
        }
        return true;
    }

    bool sendAll(int fd,const void* buf,size_t n){
        const char* p=static_cast<const char*>(buf);
        size_t left=n;
        while(left>0){
            ssize_t s=send(fd,p,left,0);
            if(s<0){
                if(errno==EINTR){
                    continue;
                }else{
                    return false;
                }
            }else{
                p+=s;
                left-=s;
            }
        }
        return true;
    }

    bool sendMsg(int fd,const std::string& msg){
        uint32_t len=static_cast<uint32_t>(msg.size());
        uint32_t netLen=htonl(len);
        if(!sendAll(fd, &netLen, sizeof(netLen))){
            return false;
        }
        if(len>0&&!sendAll(fd, msg.data(), msg.size())){
            return false;
        }
        return true;
    }


    bool recvMsg(int fd,std::string& out){
        uint32_t netLen=0;
        if(!recvAll(fd, &netLen, sizeof(netLen))){
            return false;
        }
        uint32_t len=ntohl(netLen);

        if(len>1024*1024)return false;
        
        std::vector<char> buf(len);
        if(len>0&&!recvAll(fd,buf.data(),len)){
            return false;
        }

        out.assign(buf.begin(),buf.end());
        return true;
    }

}