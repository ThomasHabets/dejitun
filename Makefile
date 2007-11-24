TARGETS=dejitun
CXXFLAGS=-g
LD=g++
LDFLAGS=-g
RM=rm

all: $(TARGETS)

dejitun: dejitun.o tun.o inet.o util.o
	$(LD) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f *.o $(TARGETS)
