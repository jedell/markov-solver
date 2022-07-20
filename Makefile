CC = g++

CFLAGS = -std=c++2a

TARGET = mdp

LINKED_FILES = 

all:
	$(CC) -o $(TARGET) $(CFLAGS) $(TARGET).cpp $(LINKED_FILES)

clean:
	$(RM) -rf $(TARGET)  $(TARGET).dSYM