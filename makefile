SRC= $(wildcard *.c)
OBJ= $(SRC: .c=.o)

CC= cc
CFLAGS = -g -Wall 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
ipmsg: $(OBJ)
	$(CC) $(CFLAGS) -o ipmsg $(OBJ) -lpthread

.PHONY: clean
clean:
	rm -f ipmsg *.o *.gch
cgch:
	rm -f *.gch
