#include<iostream>
#include<string>
#include<list>

#include<time.h>
#include<sys/time.h>
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
		if (n != ::write(fd,s.data(),n)) {
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
	int64_t   minTime __attribute__ ((__packed__));  // ms since 1970
	int64_t   maxTime __attribute__ ((__packed__));  // ms since 1970
	uint32_t  jitter  __attribute__ ((__packed__));   // jitter in ms
	char      payload[0];
};

int64_t
htonll(int64_t n)
{
	return n; // FIXME
}

int64_t
ntohll(int64_t n)
{
	return htonll(n);
}

int64_t
gettimeofdaymsec()
{
	struct timeval tv;
	if (-1 == gettimeofday(&tv, NULL)) {
		throw "gettimeofday(): FIXME";
	}
	return tv.tv_sec * int64_t(1000) + tv.tv_usec/1000;
}

typedef struct {
	Packet *packet;
	size_t len;
	FDWrapper *dev;
} PacketEntry;

std::list<PacketEntry> packetQueue;
void
schedulePacket(Packet *p, size_t len, FDWrapper *dev)
{
	PacketEntry pe;
	pe.packet = p;
	pe.len = len;
	pe.dev = dev;
	packetQueue.push_back(pe);
}
void
writePacket(Packet *p, size_t len, FDWrapper*dev);

void
packetWriter()
{
	int64_t curTime = gettimeofdaymsec();
	std::list<PacketEntry>::iterator tmp;
	for(std::list<PacketEntry>::iterator itr = packetQueue.begin();
	    itr != packetQueue.end();
	    ) {
		if (0) {
			std::cout << "\tmin: " << itr->packet->minTime
				  << " (curtime + " << itr->packet->minTime - curTime << ")" << std::endl
				  << "\tmax: " << itr->packet->maxTime
				  << " (curtime + " << itr->packet->maxTime - curTime << ")" << std::endl
				  << "\tcur: " << curTime << std::endl;
		}
		
		if (0 && itr->packet->maxTime > curTime) {
			delete[] itr->packet;
			// FIXME: stats.drop++
			packetQueue.erase(itr);
			itr = packetQueue.begin();
		} else if (itr->packet->minTime < curTime) {
			writePacket(itr->packet, itr->len, itr->dev);
			// FIXME: stats.sent++
			delete[] itr->packet;
			packetQueue.erase(itr);
			itr = packetQueue.begin();
		} else {
			++itr;
		}
	}

}

void
writePacket(Packet *p, size_t len, FDWrapper*dev)
{
	std::string s((const char*)p->payload,
		      (const char*)p->payload+len);
	dev->write(s);
}

void
run(const std::string &rhost, int rport, int lport)
{
	Tunnel tun("dejitun0");
	Inet inet(rhost, rport, lport);
	std::cout << tun.getDevname() << std::endl;

	for(;;) {
		int n;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(tun.getFd(), &fds);
		FD_SET(inet.getFd(), &fds);
		n = select(std::max(tun.getFd(), inet.getFd())+1,
			   &fds,
			   NULL,
			   NULL,
			   &tv);
		if (n < 0) {
			std::cerr << "select(): " << strerror(errno)
				  << std::endl;
		}
		if (FD_ISSET(tun.getFd(), &fds)) {
			const std::string data = tun.read();
			Packet *p = 0;
			try {
				p=(Packet*)(new char[sizeof(Packet)
							     +data.length()]);

				p->version = 0;
				p->minTime = htonll(gettimeofdaymsec() + 2000);
				p->maxTime = htonll(htonll(p->minTime) + 20000);
				p->jitter = 0;
				memcpy(p->payload, data.data(), data.length());
				std::string s((char*)p,
					      (char*)p+data.length()
					      + sizeof(Packet));
				inet.write(s);
			} catch(...) {
				delete[] p;
				throw;
			}
			delete[] p;
		}

		// -----
		if (FD_ISSET(inet.getFd(), &fds)) {
			const std::string data = inet.read();

			size_t len = data.length();
			Packet *p = (Packet*)new char[len];
			memcpy(p, data.data(), len);
			len -= sizeof(struct Packet);
			try {
				schedulePacket(p, len, &tun);
			} catch(...) {
				delete[] p;
				throw;
			}
		}
		packetWriter();
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
