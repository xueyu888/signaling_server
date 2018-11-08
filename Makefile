.PHONY:clean
CFLAGS=-Wall -g
CC=g++ -std=c++11
OBJ=main.o peer_channel.o data_socket.o command_line_parser.o stringencode.o 
main:$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@
main.o:main.cc 
	$(CC) $(CFLAGS) -c $< -o $@
peer_channel.o:peer_channel.cc 
	$(CC) $(CFLAGS) -c $< -o $@	
data_socket.o:data_socket.cc 
	$(CC) $(CFLAGS) -c $< -o $@
command_line_parser.o:./base/command_line_parser.cc
	$(CC) $(CFLAGS) -c $< -o $@
stringencode.o:./base/stringencode.cc ./base/stringutils.h 
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o main
