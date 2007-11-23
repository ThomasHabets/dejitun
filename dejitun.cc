#include<iostream>
#include<string>

#include<sys/select.h>


#include"tun.cc"

struct Packet {
	char version;
	int64_t   minTime;  // ms since 1970
	int64_t   maxTime;  // ms since 1970
	uint32_t  jitter;   // jitter in ms
	char      payload[0];
};

int
main()
{
	Tunnel tun("dejitun0");
	std::cout << tun.getDevname() << std::endl;

	for(;;) {
		int n;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(tun.getFd(), &fds);
		n = select(std::max(tun.getFd(), 0)+1,
			   &fds,
			   NULL,
			   NULL,
			   NULL);
		if (n < 0) {
			std::cerr << "select(): " << strerror(errno)
				  << std::endl;
		}
		if (n == 0) {
			continue;
		}
		if (FD_ISSET(tun.getFd(), &fds)) {
			std::cout << "Got packet!\n";
			const std::string data = tun.read();
			
			Packet *p = (Packet*)(new char[sizeof(Packet)
						       + data.length()]);

			p->version = 0;
			p->minTime = 0;
			p->maxTime = 0;
			p->jitter = 0;
			memcpy(p->payload, data.data(), data.length());
			// FIXME: send packet

			delete[] p;

			std::cout << ".\n";
		}
	}
}
