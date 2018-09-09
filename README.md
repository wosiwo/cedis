# cedis
a slimple implement or redis like nosql

 使用c实现类似redis的nosql服务(暂时只支持macos)
 目前已经支持kv,hash
* 编译服务
  ```
  make
  ```
* 启动服务
  ```
  ./server
  ```
* 客户端编译
  ```
  gcc ctest/client.c -o ctest/client
  ```
* 客户端运行
  ```
  ./ctest/client
  ```
* python客户端
  ```
  python tests/test_redis/test_protocol.py
  ```
## TODO
* 支持linux
* 实现redis中db数据结构
* 实现有续集跳跃表
* 支持set数据类型
* 支持事务ACID特性
* 性能优化


> 引用代码
* [https://github.com/otuk/kvdb) 引用了数据结构
* [https://github.com/twitter/twemproxy) 引用了python客户端

