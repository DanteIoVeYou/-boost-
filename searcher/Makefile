FLAG=-std=c++11 -I ../../cppjieba/include -lboost_system -ljsoncpp
.PHONY: all
all: test_searcher test_search libsearcher.so 
test_searcher: searcher.cpp test_searcher.cpp 
	g++ $^ -o $@ $(FLAG)
test_search: searcher.cpp test_search.cpp 
	g++ $^ -o $@ $(FLAG)
libsearcher.so: searcher.h searcher.cpp
	g++ searcher.cpp -o $@ -shared -fPIC $(FLAG)
.PHONY: clean
clean: 
	rm -f test_searcher test_search libsearcher.so
