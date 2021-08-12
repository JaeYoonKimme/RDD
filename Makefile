all:
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -o ddchck ddchck.c -pthread
	gcc -o ./test/test1 ./test/test1.c -pthread
	gcc -o ./test/test2 ./test/test2.c -pthread
	gcc -o ./test/test3 ./test/test3.c -pthread
	gcc -o ./test/test4 ./test/test4.c -pthread
	gcc -o ./test/test5 ./test/test5.c -pthread
	gcc -o ./test/test6 ./test/test6.c -pthread
	gcc -o ./test/test7 ./test/test7.c -pthread

clean: 
	rm ./test/test1 
	rm ./test/test2 
	rm ./test/test3 
	rm ./test/test4 
	rm ./test/test5
	rm ./test/test6
	rm ./test/test7
	rm ddchck 
	rm ddmon.so
