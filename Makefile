# dejitun/Makefile
TARGETS=dejitun
CXX=g++
CXXFLAGS=-O2 -g -Wall -W -pipe
LD=g++
LDFLAGS=-g
RM=rm
GIT=git
GREP=grep
SED=sed
GZIP=gzip
TEST=test
WC=wc
ECHO=echo

CXX_SUNCC=CC
LD_SUNCC=CC
CXXFLAGS_SUNCC=-features=zla

LIBS_SOLARIS=-lsocket -lnsl
LIBS=
#LIBS+=$(LIBS_SOLARIS)

all: $(TARGETS)

VER=$(shell $(GREP) '^static const double version' dejitun.cc \
	| $(SED) 's/.*=[^0-9]//' | $(SED) 's/[^.0-9]//g')
dist: dejitun-$(VER).tar.gz

dejitun-%.tar.gz:
	$(GIT) archive --format=tar \
	    --prefix=$(shell $(ECHO) $@ | $(SED) 's/\.tar\.gz//')/ \
	    v$(shell $(ECHO) $@ | $(SED) 's/.*-//' | $(SED) 's/\.tar\.gz//') \
	    | $(GZIP) -9 > $@
tag:
	$(GIT) tag -l | $(GREP) -vq '^v$(VER)' \
		|| ($(ECHO) -e "---\nError: Version $(VER) already exists!\n" \
		&& false)
	$(TEST) $(shell $(GIT) status | $(WC) -l) -lt 3 \
		|| ($(ECHO) -e "---\nError: You have uncommitted changes!\n" \
		&& false)
	$(GIT) log > ChangeLog
	$(GIT) add ChangeLog
	$(GIT) commit -m"Updated ChangeLog"
	$(GIT) tag -s v$(VER)

dejitun: dejitun.o tun_linux.o tun_freebsd.o tun_solaris.o inet.o util.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) -f *.o $(TARGETS)
