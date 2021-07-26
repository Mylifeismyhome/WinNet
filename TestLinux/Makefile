all:
	rm -r -f include
	cp -r ../Win-Net/bin/include/ .
	cp ../Win-Net/bin/NetCore.a NetCore.a

	mkdir -p bin
	g++ -I ./include -w main.cpp -o bin/test -L . -l:NetCore.a
