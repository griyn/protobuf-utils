test :
	/usr/local/bin/protoc --cpp_out=. *.proto
	g++ -g -std=c++11 test.cpp *.pb.cc ../src/*.cpp -o test -I../src -I/usr/local/include/ -L/usr/local/lib/ -L/usr/local/lib64 -lprotoc -lprotobuf -lpthread -lgtest -lgtest_main
	rm *.pb.h *.pb.cc
