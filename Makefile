TARGETS=dejitun

all: $(TARGETS)

dejitun: dejitun.cc tun.cc inet.cc
	g++ -o $@ $<