# Written by Pedro Martelletto in September 2016. Public domain.

# These are the -O99 --omg-optimized of the Linux security world. :)
CFLAGS+=	-O2 -W -Wall -Wextra -Wno-unused-parameter -MD \
		-fstack-protector-all -fPIC -DPIC -D_FORTIFY_SOURCE=2
LDFLAGS+=	-Wl,-z,noexecstack -pie -fPIE -Wl,-z,relro,-z,now
LUAC?=		luac

all: deliver switch

deliver: deliver.o
	${CC} ${LDFLAGS} -o $@ $<

deliver.o: deliver.c
	${CC} ${CFLAGS} -c -o $@ $<

switch: switch.lua
	${LUAC} -o $@ $<

clean:
	rm -f deliver deliver.[od] switch

-include deliver.d
