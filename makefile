all: task.o
	gcc -o task task.o && ./task

compile:
	gcc -c task.c

clean:
	rm *.o