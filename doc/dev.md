Srain Doc
=========
当前版本：0.02

> 我是很想用英文写文档的……但是怕写了之后自己都看不懂，所以还是上中文好了。 

# Summary
虽然 Srain 致力于实现一个轻量/美观/易用的 IRC 客户端，但事实上这是一个玩票项目。

当前项目的目录结构如下：

    srain
    ├── build
    ├── data
    │   ├── img     // 程序用到的图片
    │   ├── theme   // 主题
    │   └── ui      // glade 生成的文件
    ├── doc         // 文档
    ├── plugin      // 插件示例
    ├── po          // 多语言支持
    └── src
        ├── inc     // 头文件
        ├── irc     // IRC 协议实现
        ├── server  // 多服务器支持
        ├── test    // 测试用例
        └── ui      // 界面相关代码


# Plugin
srain 简陋地支持了插件功能，允许使用 python 3 编写插件。

> **注意：**
> - 插件在运行过程中 **不允许** 向外抛出任何异常，所有异常都应该在插件内得到处理
> - 插件接受的参数永远是字符串，返回的值也必须是字符串，不允许返回其他值

当前支持两种插件：

- avatar: 接收一个 nick，返回一个该 nick 对应的头像的 url
- upload: 接收一个本地文件的路径，将文件上传到指定的 pastebin 并返回对应的 url

// TODO

# Arch
程序目前包含三大模块：

- ui：
    - `SrainApp` 和 `SrainWindow` 类分别代表一个 Application 和 一个 Window，在程序中有且只有一个实例
    - `SrainChan` 用来表示一个会话， **而不是一个频道（Channel）**，未来可能会改名 :-|
    - 接口：`SrainApp` 提供了操作 server 模块的函数 `ui_intf_server_*()` （由 `src/ui/ui_intf.c` 提供）
        而 `ui_intf_server_*()` 最终调用了 `SrainApp` 中默认指向 server 模块的函数指针，现有函数如下：

        typedef int (*ServerJoinFunc) (void *server, const char *chan_name);
        typedef int (*ServerPartFunc) (void *server, const char *chan_name);
        typedef int (*ServerSendFunc) (void *server, const char *target, const char *msg);
        typedef int (*ServerCmdFunc) (void *server, const char *source, const char *cmd);

        self->server_join = (ServerJoinFunc)server_join;
        self->server_part = (ServerPartFunc)server_part;
        self->server_send = (ServerSendFunc)server_send;
        self->server_cmd = (ServerCmdFunc)server_cmd;

- irc:
    - 对 RFC 1459 所规定的 client 协议的部分实现
    - 提供了接口函数以及对 IRC 消息的 parser

- server:
    - 结构体 `IRCServer` 代表了一个 IRC 服务器，可以有多个不同地址的实例
    - 每个 `IRCServer` 维护着一个监听线程，`server_recv()` 在该线程上循环监听，等待 `irc_recv()` 获取数据之后通过
        `gdk_threads_add_idle()` 主线程的  `server_msg_dispatch()` 分派消息
    - 接口：`IRCServer` 提供了操作 ui 模块的函数 `server_intf_ui_*()` （由 `src/server/server_intf.c` 提供）
        而 `ui_intf_server_*()` 最终调用了 `SrainApp` 中默认指向 ui 模块的函数指针，现有函数如下：

        typedef void* (*UIAddChanFunc) (void *server, const char *srv_name, const char *chan_name);
        typedef void (*UIRmChanFunc) (void *chan);
        typedef void (*UISysMsgFunc) (void *chan, const char *msg, SysMsgType type);
        typedef void (*UISendMsgFunc) (void *chan, const char *msg);
        typedef void (*UIRecvMsgFunc) (void *chan, const char *nick, const char *id, const char *msg);
        typedef int (*UIUserListAddFunc) (void *chan, const char *nick, IRCUserType type);
        typedef int (*UIUserListRmFunc) (void *chan, const char *nick, const char *reason);
        typedef int (*UIUserListRenameFunc) (void *chan, const char *old_nick, const char *new_nick);
        typedef void (*UISetTopicFunc) (void *chan, const char *topic);
