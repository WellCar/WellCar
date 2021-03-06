# Top Makefile 

T	?=y

ifeq ($(T),y)
ARCH	:=
else
ARCH	:=arm64
endif 

ifeq ($(ARCH),arm64)
CROSS_COMPILE	:=aarch64-linux-gnu-
else
CROSS_COMPILE	:=
endif


CC	:=${CROSS_COMPILE}gcc

objs_app	:=cli.o crc.o common.o 
objs_ser	:=ser.o crc.o common.o 
OBJS	:=-lpthread

all: app  ser

ser:$(objs_ser)
	${CC} -o ser $^ ${OBJS}
	

app: $(objs_app)
	${CC} -o app $^ ${OBJS}

%.o: %.c
	${CC} -c $< -o $@

.PHONY:clean
clean:
	rm *.o app  ser
