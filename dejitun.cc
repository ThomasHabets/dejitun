#include<iostream>
#include<string>
#include<list>

#include<time.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/select.h>

#include"dejitun.h"

void
Dejitun::schedulePacket(Packet *p, size_t len, FDWrapper *dev)
{
    PacketEntry pe;
    pe.packet = p;
    pe.len = len;
    pe.dev = dev;
    packetQueue.push_back(pe);
}

void
Dejitun::packetWriter()
{
    int64_t curTime = gettimeofdaymsec();
    std::list<PacketEntry>::iterator tmp;
    for(std::list<PacketEntry>::iterator itr = packetQueue.begin();
	itr != packetQueue.end();
	) {
	if (0) {
	    std::cout << "\tmin: " << itr->packet->minTime
		      << " (curtime + "
		      << itr->packet->minTime - curTime << ")"
		      << std::endl
		      << "\tmax: " << itr->packet->maxTime
		      << " (curtime + "
		      << itr->packet->maxTime - curTime << ")"
		      << std::endl
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
Dejitun::run(const std::string &rhost, int rport, int lport)
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
		p->minTime = htonll(gettimeofdaymsec()+2000);
		p->maxTime = htonll(htonll(p->minTime)+20000);
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

void
Dejitun::writePacket(Packet *p, size_t len, FDWrapper*dev)
{
    std::string s((const char*)p->payload,
		  (const char*)p->payload+len);
    dev->write(s);
}
	

int
main(int argc, char **argv)
{
    if (argc < 4) {
	std::cerr << "Bice!\n";
	return 1;
    }
    try {
	Dejitun tun;
	tun.run(argv[1], atoi(argv[2]), atoi(argv[3]));
    } catch(const char*s) {
	std::cout << s << std::endl;
    } catch(...) {
	std::cout << "Unknown exception" << std::endl;
    }
}


/*
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
