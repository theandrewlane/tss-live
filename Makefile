CC=g++

all : tss-live

libcurl :
		cd ./libs/libtwitcurl && make install
		cd ..

tss-live : libcurl tss-live.cpp
	$(CC) -std=c++11 tss-live.cpp -I./include/ -L ./include/json.hpp -L /usr/local/lib/ -ltwitcurl -o tss-live

clean :
		rm tss-live

