## UnixNetwork
此项目就是一个简单的聊天服务器，该服务器支持多人聊天。
本项目使用协议如下，每个消息都有一个头文件   
` 4位开头，4位数据类型，8位接受用户数量，32位数据大小，（最大4g文件）  ` 

 * 四位开头可以用于判断版本号或者其他，  
 * 4位数据类型可以用于判断发送数据的类型，如文本，语音，视频等
 * 8位接受用户数量表示发送接受方的数量，如群聊时这个就>1,(每个客户端第一次连接到服务器时有一个简单的判断，有一个固定的头文件，该头文件中用户数量为0，后面携带发送方的名字。)  
 * 32位数据大小为数据长度。   
    该头文件6个字节，后面紧跟20个字节的发送方用户名，然后再跟接受方的用户名，每个用户名不超过20字节。要显示用户名，就遍历20字节的数据，直到遇到空格字符或者0。   
#### 其中项目结构为：

* sql.h  sql.cpp   

   提供了对sql库的访问，sql库主要存储了有关账户的一些信息。这个文件依赖<mysqlx/xdevapi.h>头文件，需要自行安装。为测试改程序，建议本地安装mysql服务，为方便多用户测试（需要部署云服务器，或者局域网)，教程如下https://www.zhihu.com/people/liu-xin-65-77

* 
   Log.h 这也是一个工具类，只要是打印log

*  service-jincheng.cpp， service_thread.cpp mmap.cpp都是测试其他三个高并发对应的test，并不是本项目工作。

本项目有两个主函数，一个在account下，一个在service下，其中client在account下被调用。

* 在client下主要是两个线程，副线程一直从标准输入里面入去数据，然后放在消息池messagebuf中。
  主线程一直读取服务器端发送的数据，和将messagebuf中的数据发送到服务器
* service的epoll_wait下有三个分支，一个是判断是不是客户端发送了connect，此时准备监听新的客户端，第
  二个分支就是读取数据，这个数据是客户端发送的数据，然后将这个数据按照接受方进行整理，最后一个分支就是写操作，按照第二个分支整理的接受数据，将这些数据发送到对应客户端上。