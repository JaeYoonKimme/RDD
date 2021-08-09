all:
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -o ddchck ddchck.c -pthread
	gcc -o test1 test1.c
