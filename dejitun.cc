/** dejitun/dejitun.cc
 *
 */
#include<memory>
#include<iostream>
#include<string>
#include<list>

#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/select.h>
#include <arpa/inet.h>

#include"dejitun.h"

static const double version = 0.15f;
const unsigned char Dejitun::protocolVersion = 1;

#if defined (__SVR4) && defined (__sun)
const std::string Dejitun::defaultTunnelDevice = "tun";
#elif defined(__linux__)
const std::string Dejitun::defaultTunnelDevice = "dejitun%d";
#elif defined(__FreeBSD__)
const std::string Dejitun::defaultTunnelDevice = "tun";
#endif

const int Dejitun::defaultListenPort = 12345;

/**
 *
 */
void
Dejitun::schedulePacket(Packet *p, size_t len, FDWrapper *dev)
{
    PacketEntry pe;
    pe.packet = p;
    pe.len = len;
    pe.dev = dev;
    packetQueue.push_back(pe);
}

/**
 *
 */
void
Dejitun::packetWriter()
{
    int64_t curTime = gettimeofdaymsec();
    std::list<PacketEntry>::iterator tmp;
    for(std::list<PacketEntry>::iterator itr = packetQueue.begin();
	itr != packetQueue.end();
	) {
	cdebug << "Sending packets, " << packetQueue.size() << " entries"
	       << std::endl;
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
        if (itr->packet->maxTime && (itr->packet->maxTime < curTime)) {
	    cdebug << "Packet too old, discarding" << std::endl;
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
	
/**
 *
 */
void
Dejitun::run()
{
    if (options.daemonize) {
	if (0 > daemon(0,0)) {
	    std::cerr << "daemon(): " << strerror(errno) << std::endl;
	}
    }

    for(;;) {
	int n;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 5000;
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

	    std::auto_ptr<char> cp(new char[sizeof(Packet)+data.length()]);
	    Packet *p = (Packet*)cp.get();
	    
	    p->version = protocolVersion;
	    p->minTime=htonll(gettimeofdaymsec()+f2i64(options.minDelay));
	    p->maxTime=htonll(htonll(p->minTime)+f2i64(options.maxDelay));
	    p->jitter = htonll(f2i64(options.jitter));

	    if (!options.minDelay) {
		p->minTime = 0;
	    }
	    if (!options.maxDelay) {
		p->maxTime = 0;
	    }
	    memcpy(p->payload, data.data(), data.length());
	    std::string s((char*)&*p,
			  (char*)&*p+data.length()
			  + sizeof(Packet));
	    inet.write(s);
	}

	// -----
	if (FD_ISSET(inet.getFd(), &fds)) {
	    const std::string data = inet.read();

	    size_t len = data.length();

	    // deleted by scheduler or below
	    Packet *p = (Packet*)new char[len];

	    memcpy(p, data.data(), len);
	    len -= sizeof(struct Packet);
	    if (p->version != protocolVersion) {
		cdebug << "Wrong version "
		       << p->version << " != " << protocolVersion
		       << std::endl;
		delete[] p;
	    } else {
		p->minTime = htonll(p->minTime);
		p->maxTime = htonll(p->maxTime);
		p->jitter = htonl(p->jitter);
		double j = (double)rand()/RAND_MAX;
		cdebug << "Jitter: " << (options.jitter * 1000.0 * j)
		       << std::endl;

		p->minTime += (int64_t)(options.jitter * 1000.0 * j);

		try {
		    cdebug << "Scheduling packet for insert into tunnel"
			   << std::endl;
		    schedulePacket(p, len, &tun);
		} catch(...) {
		    delete[] p;
		    throw;
		}
	    }
	}
	packetWriter();
    }
}

/**
 *
 */
void
Dejitun::writePacket(Packet *p, size_t len, FDWrapper*dev)
{
    std::string s((const char*)p->payload,
		  (const char*)p->payload+len);
    dev->write(s);
}
	


/**
 *
 */
static void
usage(const char *a0, int err)
{
    printf("Dejitun %.2f, by Thomas Habets <thomas@habets.se>\n"
	   "Usage: %s [ -d <mindelay> ] [ -D <maxdelay> ] [ -i <tunnel dev> ]"
	   "\n\t[ -j <hitter> ] "
	   "[ -h ] [ -p <local port> ]"
	   " <remote host> <remote port>\n"
	   "\t-A               Expert use only: turn off AF-info in proto\n"
	   "\t-d <mindelay>    Min (optimal) delay in secs (default: 0.0)\n"
	   "\t-D <maxdelay>    Max delay (drop-limit)  (default: 10.0)\n"
	   "\t-f               Run in foreground\n"
	   "\t-h               Show this help text\n"
	   "\t-i <tunneldev>   Name of tunnel device (default: %s)\n"
	   "\t-j <jitter>      Jitter between min and min+jitter "
           "(default: 0.0)"
	   "\n"
	   "\t-p <local port>  Local port to listen to (default: %d)\n"
	   "\t-v <debugfile>   Output verbose (debug) stuff to file\n"
	   ,version,a0,
           Dejitun::defaultTunnelDevice.c_str(),
           Dejitun::defaultListenPort
           );
    exit(err);
}

/**
 *
 */
int
main(int argc, char **argv)
{
    Dejitun::Options opts;

    int c;
    while (-1 != (c = getopt(argc, argv, "Ad:D:fhj:i:p:v:"))) {
	switch(c) {
	case 'A':
	    opts.multiAF = false;
	    break;
	case 'd':
	    opts.minDelay = atof(optarg);
	    break;
	case 'D':
	    opts.maxDelay = atof(optarg);
	    break;
	case 'f':
	    opts.daemonize = false;
	    break;
	case 'h':
	    usage(argv[0], 0);
	case 'i':
	    opts.tunnelDevice = optarg;
	    break;
	case 'j':
	    opts.jitter = atof(optarg);
	    break;
	case 'p':
	    opts.localPort = atoi(optarg);
	    break;
	case 'v':
	    opts.debugfile = optarg;
	    break;
	default:
	    usage(argv[0], 1);
	}
    }
    if (optind + 2 != argc) {
	usage(argv[0], 1);
    }
    opts.peer = argv[optind];
    opts.remotePort = atoi(argv[optind+1]);

    try {
	Dejitun tun(opts);
	printf("Dejitun %.2f, by Thomas Habets <thomas@habets.se>, "
	       "is up on %s\n",
	       version, tun.getDevname().c_str());
	tun.run();
    } catch(const char*s) {
	std::cerr << s << std::endl;
    } catch(const std::string &s) {
	std::cerr << s << std::endl;
    } catch(const std::exception &e) {
	std::cerr << "std::exception: " << e.what() << std::endl;
    } catch(...) {
	std::cerr << "Unknown exception" << std::endl;
    }
}

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
