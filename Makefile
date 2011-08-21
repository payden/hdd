all:
	gcc -Wall -Werror -c signal.c
	gcc -Wall -Werror -c hdd.c
	gcc -Wall -Werror hdd.o signal.o -o hdd
clean:
	rm -f hdd *.o
