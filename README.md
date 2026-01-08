# cpp_bigproj_prep

Day 1: project skeleton + low power start.
Build system: CMake.

Day 2: 进行tcp相关内容的再次练习
    着重解决了关于tcp字节流相关问题，通过长度前缀的方法
    增加信息网络传递可复用

Day 3: 进行了epoll io多路复用来解决并发
    并且根据之前tcp的传输数据协议进行了修改，还是按照字节数+payload的形式进行的
    改进tcp种Allsend，Allreecv的阻塞式访问，通过unorderedmap和vector容器解决了tcp_server接受信息recv的半包和粘包问题

Day 4: 增加了send的outbuf处理粘包和半包问题
    并且实现了异常处理的工程化解决方法，利用timer来杀掉读了一半的内容