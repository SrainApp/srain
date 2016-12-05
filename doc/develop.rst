Srain Doc
=========

Current version：0.05

Summary
-------

Srain 致力于实现一个轻量/美观/易用的现代化 IRC 客户端。

当前项目的目录结构如下::

    srain
    ├── build
    ├── data
    │   ├── pixmaps // 图片
    │   ├── plugins // 插件示例
    │   ├── themes  // 主题
    │   └── ui      // glade 文件
    ├── doc         // 文档
    ├── po          // 翻译文件
    ├── test        // 部分测试脚本
    └── src
        ├── inc     // 头文件
        ├── srv     // IRC 会话管理
        └── ui      // 界面

Codeing Style
-------------

* Public header 用 ``__XXX_H`` 形式
* 提倡在函数开头统一声明变量，而不是即用即声明，方便释放的时候统一处理
* 缩进：四空格，无 Tab
* 折行：

  - 头文件中的函数签名不折行，其他地方的代码一律小于 80 char per line
  - 参数过长时，从超过长度的第一个参数开始折行，参数无需对齐
  - 太长的字符串可以不折行
  - ``gtk_widget_class_bind_template_child()`` 由于实在太长，可以不折行
  - 花括号不换行（除非在 ``case`` 中），右圆括号和左花括号之间没有空格（待定）

* 命名：

  - 鼓励局部变量和函数参数使用缩写，全局变量及函数名，类型名不鼓励缩写
  - 宏大写，数据类型，类名用大驼峰法，变量使用全小写 + 下划线
  - 文件名全小写，以下划线分隔（考虑换成连字符）
  - UI 模块的导出函数必须以 ``ui_`` 开头，SRV 模块同理

* 注释 & 日志：

  - 按英文规范，句首第一个单词首字符大写，多句时使用标点符号，同样每行不得超过
    80 字符（汉字以两字符计），链接可以不折行
  - 日志级别：``DGB_FR`` ``LOG_FR`` ``WARN_FR`` ``ERROR_FR`` ，输出时包含回车和
    函数名， 目前仅能 在编译时使用宏 ``__DBG_ON`` ``__LOG_ON`` 控制
  - Doxygen 貌似不好用（考虑换 sphinx）

* Git commit: TODO
* 单元测试：考虑引入 libcheck

Python Plugin
-------------

插件以 ``插件名.py`` 形式命名，放置在 ``$PREFIX/usr/share/srain/plugins`` 或者
``$HOME/.config/srain/plugins`` 下，程序启动时（``plguin_init``）会自动加载。

avatar
******

根据 ``token`` 和 ``nick`` 接收一个 nick，下载 36x36 的头像图片到 ``path``，以
``nick`` 命名

签名如下::

    def avatar(nick, token, path):
        pass

upload
******

接收一个本地文件的路径 ``img`` ，将文件上传到指定的图床并返回对应的 url

签名如下::

    def upload(img):
        pass

scrshot
*******

调用外部工具截屏并上传，TODO

Test
----

我不知道怎么写测试…… :-(

Internal Interface
------------------

接口的定义需要简化，改接口的时候会牵动一大堆东西。

UI
**

::

    void ui_init        (int argc, char **argv);
    void ui_add_chat    (const char *srv_name, const char *chat_name, const char *nick, ChatType type);
    void ui_rm_chat     (const char *srv_name, const char *chat_name);
    void ui_sys_msg     (const char *srv_name, const char *chat_name, const char *msg, int mention, SysMsgType type);
    void ui_send_msg    (const char *srv_name, const char *chat_name, const char *msg);
    void ui_recv_msg    (const char *srv_name, const char *chat_name, const char *nick, const char *id, const char *msg, int mention);
    void ui_add_user    (const char *srv_name, const char *chat_name, const char *nick, UserType type);
    void ui_rm_user     (const char *srv_name, const char *chat_name, const char *nick);
    void ui_ren_user    (const char *srv_name, const char *chat_name, const char *old_nick, const char *new_nick, UserType type);
    void ui_set_topic   (const char *srv_name, const char *chat_name, const char *topic);

SRV
***

::

    void srv_init       ();
    void srv_finalize   ();
    void srv_connnect   (const char *host, int port, const char *passwd, const char *nickname, const char *username, const char *realname, int ssl);
    void srv_cmd        (const char *srv_name, const char *source, char *cmd, int block);
    void srv_query      (const char *srv_name, const char *nick);
    void srv_unquery    (const char *srv_name, const char *nick);
    void srv_join       (const char *srv_name, const char *chan_name, const char *passwd);
    void srv_part       (const char *srv_name, const char *chan_name);
    void srv_send       (const char *srv_name, const char *target, const char *msg);
    void srv_quit       (const char *srv_name, const char *reason);
    void srv_kick       (const char *srv_name, const char *chan_name, const char *nick);
    void srv_whois      (const char *srv_name, const char *nick);
    void srv_invite     (const char *srv_name, const char *chan_name, const char *nick);
