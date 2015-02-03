CC     = gcc
CFLAGS = -g -Wall -pthread
OBJS   = add_example.o hashset_chain.o treeset.o
#OBJS2  = union_example.o hashset_chain.o treeset.o

add_example : $(OBJS)
			$(CC) $(CFLAGS) -o $@ $(OBJS)

#union_example : $(OBJS2)
#			$(CC) $(CFLAGS) -o $@ $(OBJS2)

add_example.o : ./example/add_example.c
			$(CC) -c $(CFLAGS) -o $@ $<

union_example.o : ./example/union_example.c
			$(CC) -c $(CFLAGS) -o $@ $<

hashset_chain.o : hashset_chain.c
			$(CC) -c $(CFLAGS) -o $@ $<

treeset.o : treeset.c
			$(CC) -c $(CFLAGS) -o $@ $<