# protobuf_utils

## install google protobuf
### install step
```
git clone https://github.com/google/protobuf
./autogen.sh
./configure
make
make check
sudo make install
sudo ldconfig  # with risk
```

### use in code
* protoc path: /usr/local/bin/protoc
* include path: -I/usr/local/include
* .a path: -L/usr/local/lib -lprotoc -lprotobuf
* .so path: export LD_LIBRARY_PATH=/usr/local/lib
