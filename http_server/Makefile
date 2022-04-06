FLAG=-std=c++11 -I ../../cppjieba/include  -lpthread -lboost_system -L. -lsearcher
http_server: http_server.cpp
	g++ $^ -o $@ $(FLAG)
.PHONY: clean
clean: 
	rm -f http_server
