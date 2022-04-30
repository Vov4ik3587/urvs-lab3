all: task.o
	gcc -o ./bin/task task.o && ./bin/task

compile:
	gcc -c task.c

clean:
	rm *.o