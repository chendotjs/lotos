# 并发模型

## 0x01 介绍

常见的并发模型主要有Fork-Exec模型和event driven模型。 Lotos采用的就是多进程Reactor模型，属于event driven模型。

传统的网络服务器的构建中，IO模式会按照Blocking/Non-Blocking、Synchronous/Asynchronous这两个标准进行分类，其中Blocking与Synchronous基本上一个意思，而NIO与Async的区别在于NIO强调的是Polling(即用户进程需要时常询问IO操作是否就绪)，而Async强调的是Notification(kernel将IO数据拷贝入用户空间给用户进程使用)。

将同步与否、阻塞与否组合一下，可以得到3种主要的IO模式：

- **同步阻塞**：在此种方式下，用户进程在发起一个IO操作以后，必须等待IO操作的完成，只有当真正完成了IO操作以后，用户进程才能运行。

- **同步非阻塞**：在此种方式下，用户进程发起一个IO操作以后边可返回做其它事情，但是用户进程需要时不时的询问IO操作是否就绪，这就要求用户进程不停的去询问，从而引入不必要的CPU资源浪费。

- **异步非阻塞**：在此种模式下，用户进程只需要发起一个IO操作然后立即返回，等IO操作真正的完成以后，应用程序会得到IO操作完成的通知，此时用户进程只需要对数据进行处理就好了，不需要进行实际的IO读写操作，因为真正的IO读取或者写入操作已经由内核完成了。

前些年在IO并发领域有个很著名的[C10K](http://www.kegel.com/c10k.html)问题，即有10000个客户端需要连上一个服务器并保持TCP连接，客户端会不定时的发送请求给服务器，服务器收到请求后需及时处理并返回结果。

## 0x02 Unix下5种IO模式

### 1.阻塞 I/O（blocking IO）

在linux中，默认情况下所有的socket都是blocking，一个典型的读操作流程大概是这样： ![](https://lukangping.gitbooks.io/java-nio/content/resources/blocking_io.jpg)

当用户进程调用了recvfrom这个系统调用，kernel就开始了IO的第一个阶段：准备数据（对于网络IO来说，很多时候数据在一开始还没有到达。比如，还没有收到一个完整的UDP包。这个时候kernel就要等待足够的数据到来）。这个过程需要等待，也就是说数据被拷贝到操作系统内核的缓冲区中是需要一个过程的。而在用户进程这边，整个进程会被阻塞（当然，是进程自己选择的阻塞）。当kernel一直等到数据准备好了，它就会将数据从kernel中拷贝到用户内存，然后kernel返回结果，用户进程才解除block的状态，重新运行起来。

所以，blocking IO的特点就是在IO执行的两个阶段都被block了。

### 2.非阻塞 I/O（nonblocking IO）

linux下，可以通过设置socket使其变为non-blocking。当对一个non-blocking socket执行读操作时，流程是这个样子： ![](https://sfault-image.b0.upaiyun.com/961/916/961916360-570a10b5a0ea9_articlex)

当用户进程发出read操作时，如果kernel中的数据还没有准备好，那么它并不会block用户进程，而是立刻返回一个error。从用户进程角度讲 ，它发起一个read操作后，并不需要等待，而是马上就得到了一个结果。用户进程判断结果是一个error时，它就知道数据还没有准备好，于是它可以再次发送read操作。一旦kernel中的数据准备好了，并且又再次收到了用户进程的system call，那么它马上就将数据拷贝到了用户内存，然后返回。

所以，nonblocking IO的特点是用户进程需要不断的主动询问kernel数据好了没有。

### 3.I/O 多路复用（ IO multiplexing）

IO multiplexing就是我们说的select，poll，epoll，有些地方也称这种IO方式为event driven IO。select/epoll的好处就在于单个process就可以同时处理多个网络连接的IO。它的基本原理就是select，poll，epoll这个function会不断的轮询所负责的所有socket，当某个socket有数据到达了，就通知用户进程。 ![](https://sfault-image.b0.upaiyun.com/304/440/3044406194-570a10b9efcb2_articlex)

当用户进程调用了select，那么整个进程会被block，而同时，kernel会"监视"所有select负责的socket，当任何一个socket中的数据准备好了，select就会返回。这个时候用户进程再调用read操作，将数据从kernel拷贝到用户进程。

所以，I/O 多路复用的特点是通过一种机制一个进程能同时等待多个文件描述符，而这些文件描述符（套接字描述符）其中的任意一个进入读就绪状态，select()函数就可以返回。

这个图和blocking IO的图其实并没有太大的不同，事实上，还更差一些。因为这里需要使用两个system call (select 和 recvfrom)，而blocking IO只调用了一个system call (recvfrom)。但是，用select的优势在于它可以同时处理多个connection。

所以，如果处理的连接数不是很高的话，使用select/epoll的web server不一定比使用multi-threading + blocking IO的web server性能更好，可能延迟还更大。select/epoll的优势并不是对于单个连接能处理得更快，而是在于能处理更多的连接。）

在IO multiplexing Model中，实际中，对于每一个socket，一般都设置成为non-blocking，但是，如上图所示，整个用户的process其实是一直被block的。只不过process是被select这个函数block，而不是被socket IO给block。

### 4.信号驱动式IO

![](https://sfault-image.b0.upaiyun.com/221/712/2217129294-570a10c14d83b_articlex)

### 5.异步 I/O（asynchronous IO）

inux下的asynchronous IO其实用得很少。先看一下它的流程： ![](https://sfault-image.b0.upaiyun.com/401/185/4011854437-570a10c3b6e2c_articlex)

用户进程发起read操作之后，立刻就可以开始去做其它的事。而另一方面，从kernel的角度，当它受到一个asynchronous read之后，首先它会立刻返回，所以不会对用户进程产生任何block。然后，kernel会等待数据准备完成，然后将数据拷贝到用户内存，当这一切都完成之后，kernel会给用户进程发送一个signal，告诉它read操作完成了。

## 0x03 Fork-Exec模型

Fork-Exec模型多采用同步阻塞IO，对每一个客户端的socket连接，都需要一个线程来处理，而且在此期间这个线程一直被占用，直到socket关闭。通常由一个独立的Acceptor线程负责监听客户端的连接，接收到客户端连接之后为客户端连接创建一个新的线程处理请求消息，处理完成之后，返回应答消息给客户端，线程销毁，这就是典型的一请求一应答模型。该架构最大的问题就是不具备弹性伸缩能力，当并发访问量增加后，服务端的线程个数和并发访问数成线性正比。创建线程多了，数据频繁拷贝（I/O，内核数据拷贝到用户进程空间、阻塞），进程/线程上下文切换消耗大，从而导致操作系统崩溃。

面对这种问题，相应的改进方式，可以设计一个线程池，复用线程资源，减少线程新建、销毁的开销。但仍然是一种浪费资源的方式。

面对即时聊天(IM)程序这种连接时间长、载体消息短的应用场景，一台服务器可能要撑起几十万的连接量(C100k问题)，Fork-Exec模型是很难应对的。如果业务逻辑中，线程需要进行时间较长的IO操作(例如跨机房访问接口)，则线程大部分时间都在等待IO的返回，白白浪费了大量CPU时间片。

## 0x04 Reactor模型

什么是Reactor？ 换个名词"non-blocking IO + IO multiplexing"，意思就显而易见了。Reactor模式用非阻塞IO + IO复用函数来处理并发，程序的基本结构是一个事件循环，以事件驱动和事件回调的方式实现业务逻辑。

Lotos就是采用NIO + epoll的方式处理并发。Lotos使用epoll作为同步事件多路分解器(Synchronous Event Demultiplexer)，等待IO事件的发生。当可读或者可写事件发生于某个文件描述符时，事件分解器会去调用之前注册的相应的事件处理器。

对应到Lotos的代码中，[main.c](../src/mainl.c)中`request_handle`和`response_handle`就是对读、写操作的相应handler。

## 0x05 Proactor模型

与Reactor模式对应的就是Proactor模式。Reactor和Proactor模式的主要区别就是真正的读取和写入操作是有谁来完成的，Reactor中需要应用程序自己读取或者写入数据，而 Proactor模式中，应用程序不需要进行实际的读写过程，它只需要从缓存区读取或者写入即可，操作系统会读取缓存区或者写入缓存区到真正的IO设备。Proactor的实现依赖操作系统对异步的支持，目前实现了纯异步操作的操作系统少，实现优秀的如windows IOCP。由于Unix/Linux系统对纯异步的支持有限，应用事件驱动的主流还是通过select/epoll来实现。

## 参考

<https://segmentfault.com/a/1190000004909797#articleHeader14>

<https://segmentfault.com/a/1190000003063859>

<https://segmentfault.com/a/1190000002715832>
