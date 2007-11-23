#include<iostream>
#include<string>

#include<sys/select.h>

class FDWrapper {
protected:
	int fd;
public:
	struct {
		uint64_t shortWrite;
		uint64_t readError;
		uint64_t writeError;
	} stats;
	FDWrapper(): fd(-1) {}
	int getFd() const { return fd; }

	// returns false for warnings
	bool write(const std::string &s)
	{
		ssize_t n;
		n = s.length();
		if (n != ::write(fd,s.data(),s.length())) {
			if (n < 0) {
				stats.writeError++;
				throw "FDWrapper::write(): FIXME";
			}
			stats.shortWrite++;
			return false;
		}
		return true;
	}
	std::string read()
	{
		char buf[102400]; // 100k enough?
		ssize_t n;
		if (0 > (n = ::read(fd,buf,sizeof(buf)))) {
			stats.readError++;
			throw "FDWrapper::read(): FIXME";
		}
		return std::string(buf,&buf[n]);
	}
	virtual ~FDWrapper()
	{
		if (fd >= 0) {
			close(fd);
			fd = -1;
		}
	}

};

#include"tun.cc"
#include"inet.cc"

struct Packet {
	char version;
	int64_t   minTime;  // ms since 1970
	int64_t   maxTime;  // ms since 1970
	uint32_t  jitter;   // jitter in ms
	char      payload[0];
};


void run(const std::string &rhost, int rport, int lport)
{
	Tunnel tun("dejitun0");
	Inet inet(rhost, rport, lport);
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

			Packet *p = 0;
			try {
				p=(Packet*)(new char[sizeof(Packet)
							     +data.length()]);

				p->version = 0;
				p->minTime = 0;
				p->maxTime = 0;
				p->jitter = 0;
				memcpy(p->payload, data.data(), data.length());
				std::string s((char*)p,(char*)p+data.length());
				inet.write(s);
			} catch(...) {
				delete[] p;
				throw;
			}
			delete[] p;

			std::cout << ".\n";
		}
	}
}
int
main(int argc, char **argv)
{
	
	if (argc < 4) {
		std::cerr << "Bice!\n";
		return 1;
	}
	try {
		run(argv[1], atoi(argv[2]), atoi(argv[3]));
	} catch(const char*s) {
		std::cout << s << std::endl;
	} catch(...) {
		std::cout << "Unknown exception" << std::endl;
	}
}
