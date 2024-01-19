include config.mk

SRC = tracking.c drw.c util.c winutil.c
SRCH= 			 drw.h util.h winutil.h
OBJ = ${SRC:.c=.o}
EXE = a.out
REBUILD = rm -f *.o
all: options default

options:
	@echo build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "rebuild  = ${REBUILD}"
.c.o:
	${CC} -c ${CFLAGS} $<

default: ${OBJ}
	${CC} -o ${EXE} ${OBJ} ${LDFLAGS}
	${REBUILD}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${EXE} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${EXE}
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${EXE}

.PHONY: all options release dist install uninstall
