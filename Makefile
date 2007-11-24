TARGETS=dejitun
CXXFLAGS=-g
LD=g++
LDFLAGS=-g
RM=rm
GIT=git
GREP=grep
SED=sed
GZIP=gzip

all: $(TARGETS)

VER=$(shell $(GREP) '^static const double version' dejitun.cc \
	| $(SED) 's/.*=[^0-9]//' | $(SED) 's/[^.0-9]//g')
dist: dejitun-$(VER).tar.gz
dejitun-$(VER).tar.gz:
	$(GIT) archive --format=tar --prefix=dejitun-$(VER)/ v$(VER) \
		| $(GZIP) -9 > $@
tag:
	$(GIT) tag -s v$(VER)

dejitun: dejitun.o tun.o inet.o util.o
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f *.o $(TARGETS)
