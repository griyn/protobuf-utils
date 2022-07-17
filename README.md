# protobuf_utils

protobuf工具箱。封装了针对message基类的操作，主要使用反射完成。

## install google protobuf
### install step
```
git clone https://github.com/google/protobuf
cd protobuf/
./autogen.sh
./configure
make -j8
make check
sudo make install
sudo ldconfig  # with risk, not nessesary
```

### use in code
* protoc path: /usr/local/bin/protoc
* include path: -I/usr/local/include
* .a path: -L/usr/local/lib -lprotoc -lprotobuf
* .so path: export LD_LIBRARY_PATH=/usr/local/lib

## install camke
### install step
```
download tar from https://cmake.org/
tar zxvf
cd cmake
./booststrap
make
make install
```

## install gtest
### install step
```
git clone https://github.com/google/googletest.git -b release-1.12.0
cd googletest        # Main directory of the cloned repository.
mkdir build          # Create a directory to hold the build output.
cd build
cmake ..             # Generate native build scripts for GoogleTest.
make
sudo make install    # Install in /usr/local/ by default
```
