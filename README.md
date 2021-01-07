
# 小土的网络工具箱

* Ubuntu 18.04
* g++ 7.4.0
* gcc 7.4.0
* make 4.1

## 安装:

```
$ make
# 安装路径/usr/local，详细请查看Makefile
$ sudo make install
# 卸载
$ sudo make uninstall
```

## 依赖:

### 1. gtest
```
$ git clone https://github.com/google/googletest.git
$ cd gooletest
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
```

## 关联
* [Linux下的事件与网络编程](https://gaoyichao.com/Xiaotu/?book=Linux下的事件与网络编程&title=index)


