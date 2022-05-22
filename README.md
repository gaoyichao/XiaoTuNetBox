
# 小土的网络工具箱

* Ubuntu 18.04
* g++ 7.4.0
* gcc 7.4.0
* make 4.1

## 安装:

```
git clone https://github.com/gaoyichao/XiaoTuNetBox.git
cd XiaoTuNetBox
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/local ..
make
make install
# 卸载
make uninstall
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


