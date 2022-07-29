test :
	/usr/local/bin/protoc --cpp_out=. *.proto
	/usr/local/bin/protoc --cpp_out=../src ../src/*.proto -I../src -I/usr/local/include/
	g++ -g -std=c++11 *_test.cpp *.pb.cc ../src/*.cpp ../src/*.cc -o test -I../src -I/usr/local/include/ -L/usr/local/lib/ -L/usr/local/lib64 -lprotoc -lprotobuf -lpthread -lgtest -lgtest_main
	rm *.pb.h *.pb.cc
	rm ../src/*.pb.h ../src/*.pb.cc
