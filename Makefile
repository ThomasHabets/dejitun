TARGETS=dejitun
CXXFLAGS=-g
LD=g++
LDFLAGS=-g
RM=rm

all: $(TARGETS)

VER=$(shell grep '^static const double version' dejitun.cc \
	| sed 's/.*=[^0-9]//' | sed 's/[^.0-9]//g')
dist: dejitun-$(VER).tar.gz
dejitun-$(VER).tar.gz:
	git archive --format=tar --prefix=dejitun-$(VER)/ v$(VER) \
		| gzip -9 > $@
tag:
	git tag -s v$(VER)

dejitun: dejitun.o tun.o inet.o util.o
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f *.o $(TARGETS)
