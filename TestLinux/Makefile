all:
	rm -r -f include
	cp -r ../Win-Net/bin/include/ .
	cp ../Win-Net/bin/NetCore.a NetCore.a

	mkdir -p bin
	g++ -std=c++11 -I ./include -I ../extern/ -w main.cpp -o bin/test -L . -l:NetCore.a -pthread -ldl -lresolv
