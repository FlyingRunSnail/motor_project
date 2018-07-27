
CC:=gcc
CFLAGS:=-g -Werror -Wall
LDFLAGS:=
TARGET:=motor
RM:=rm
INCLUDES=-Imotor_driver/ 
INCLUDES+=-Irs485_driver/

OBJS=main.o motor_driver/motor.o motor_driver/protocol.o rs485_driver/rs485.o rs485_driver/serial.o

all:$(OBJS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJS)

.c.o:
	$(CC) -c -o $*.o $< $(INCLUDES)

.PHONY:all clean

clean:
	${RM} -f $(OBJS)
	${RM} -f $(TARGET)
