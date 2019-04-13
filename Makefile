all: fr_simulator.out

fr_simulator.out: fr_simulator.o
	 gcc -o fr_simulator.out fr_simulator.o queue.h linkedlist.h -lpthread

fr_simulator.o: fr_simulator.c 
	 gcc -c fr_simulator.c 

clean:
	 rm fr_simulator.o fr_simulator.out output.txt
	 
