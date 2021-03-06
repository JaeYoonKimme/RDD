all:
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -shared -fPIC -o ddtect.so ddtect.c -ldl
	gcc -o ddchck ddchck.c -pthread
	gcc -o dpred dpred.c -pthread
	gcc -g -o ./test/test1 ./test/test1.c -pthread
	gcc -g -o ./test/test2 ./test/test2.c -pthread
	gcc -g -o ./test/test3 ./test/test3.c -pthread
	gcc -g -o ./test/test4 ./test/test4.c -pthread
	gcc -g -o ./test/test5 ./test/test5.c -pthread
	gcc -g -o ./test/test6 ./test/test6.c -pthread
	gcc -g -o ./test/test7 ./test/test7.c -pthread
	gcc -g -o ./test/ptest1 ./test/ptest1.c -pthread
	gcc -g -o ./test/ptest2 ./test/ptest2.c -pthread

clean: 
	rm ./test/test1 
	rm ./test/test2 
	rm ./test/test3 
	rm ./test/test4 
	rm ./test/test5
	rm ./test/test6
	rm ./test/test7
	rm ./test/ptest1
	rm ./test/ptest2
	rm ddchck 
	rm dpred
	rm ddmon.so
	rm ddtect.so
