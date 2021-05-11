CC = clang
CCFLAGS := -Wall -Wextra -Werror
INCLUDEDIR = ./
OUTPUT = resolv
SOURCES = main.c resolver.c
OBJECTS=${SOURCES:.c=.o}

all : ${OUTPUT}

clean:
	rm -f ${OUTPUT} *.o *~

.if make(debug)
  CCFLAGS += -g -DDEBUG -O0
.endif
debug: all


${OUTPUT}: ${OBJECTS}
	${CC} ${LDFLAGS} ${OBJECTS} -o ${OUTPUT}

.c.o:
	${CC} -I${INCLUDEDIR} ${CCFLAGS} -c ${.IMPSRC} -o ${.TARGET}
