# Makefile for tinyproxy.
# tinyproxy is GPLed.  Please see the file 'COPYING'.

#CFLAGS	= -g                    # For debugging.
CFLAGS = -O2                   # For production.
CC	= gcc			# Change this if needed.
LIBS	= -lbsd -lsocket		# For Linux.
#LIBS	= -lnsl -lsocket	# For Solaris/SunOS (maybe)
INSTALL	= /usr/bin/install 	# Set to path for install.
BINPATH	= /usr/local/bin	# Where binaries will be placed.
SRC	= conns.c log.c reqs.c sock.c tinyproxy.c utils.c
OBJ	= conns.o log.o reqs.o sock.o tinyproxy.o utils.o

all: ${OBJ}
	${CC} ${CFLAGS} -o tinyproxy.exe ${OBJ} ${LIBS}
install: all
	${INSTALL} -c -s tinyproxy ${BINPATH}
uninstall:
	rm -f ${BINPATH}/tinyproxy

clean: 
	rm -f *.o *~
tidy:	clean
	rm -f tinyproxy
