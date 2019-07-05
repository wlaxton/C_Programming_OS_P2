CC = cc
CFLAGS = -g
OBJ1 = main.o
OBJ2 = child.o
TARGET1 = P2
TARGET2 = child

.SUFIXES:.c.o

all: $(TARGET1) $(TARGET2) clean

$(TARGET1): $(OBJ1)
	$(CC) -o $(CFLAGS) -o $(TARGET1) $(OBJ1)
$(TARGET2): $(OBJ2)
	$(CC) -o $(CFLAGS) -o $(TARGET2) $(OBJ2)
.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o
