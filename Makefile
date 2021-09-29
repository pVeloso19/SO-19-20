TARGET=argus argusd 
CC=gcc
CFLAGS= -o2

normal: $(TARGET)

client: argus.c argus.h
	$(CC) $(CFLAGS) argus.c -o argus

argusd: argusd.c argus.h
	$(CC) $(CFLAGS) argusd.c -o argusd

all: $(TARGET)

clean:
	$(RM) $(TARGET)
	-@find .  -maxdepth 1 -type p -delete
	-@rm *.txt
	-@rm *.idx
	-@killall -g argusd
	-@killall -g argus