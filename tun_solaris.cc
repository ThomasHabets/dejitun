#ifdef __SunOS__
#include<sys/sockio.h>
#include<net/if.h>

#include"util.h"

/**
 *
 */
Tunnel::Tunnel(const std::string &dev,bool header)
{
    int muxid,ppa = -1;

    if (0 > (udpfd = open("/dev/udp",O_RDWR))) {
	throw "Tunnel::Tunnel(): open(/dev/udp): FIXME";
    }
    if (0 > (fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	close(udpfd);
	throw "Tunnel::Tunnel(): open(/dev/tun): FIXME";
    }
    if (0 > (ppa = ioctl(fd, TUNNEWPPA, ppa))) {
	close(udpfd);
	close(fd);
	throw "Tunnel::Tunnel(): ioctl(TUNNEWPPA): FIXME";
    }
    {
	int tfd;
	if (0 > (fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	    close(udpfd);
	    throw "Tunnel::Tunnel(): open(/dev/tun)(2): FIXME";
	}

	if (0 > (ioctl(fd,I_PUSH,"ip"))) {
	    close(udpfd);
	    close(fd);
	    close(tfd);
	    throw "Tunnel::Tunnel(): ioctl(I_PUSH): FIXME";
	}
	if (0 > (ppa = ioctl(fd, I_UNITSEL, (char*)ppa))) {
	    close(udpfd);
	    close(fd);
	    close(tfd);
	    throw "Tunnel::Tunnel(): ioctl(TUNNEWPPA): FIXME";
	}
	if (0 > (muxid = ioctl(udpfd, I_PLINK, tft))) {
	    close(udpfd);
	    close(fd);
	    close(tfd);
	    throw "Tunnel::Tunnel(): ioctl(I_PLINK): FIXME";
	}
	close(tfd);
    }
    if (0 > (fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	throw "Tunnel::Tunnel(): open(/dev/tun): FIXME";
    }

    // Get name
    {
	char buf[32];
	snprintf(buf,sizeof(buf),"tun%d",ppa);
	devname = buf;
    }

    memset(&ifr,0,sizeof(ifr));
    strncpynt(ifr.ifr_name, devname.c_str(), sizeof(ifr.ifr_name));
    ifr.ifr_ip_muxid = muxid;

    if (0 > ioctl(udpfd, SIOCSIFMUXID, &ifr) < 0) {
	ioctl(udpfd,I_PULINK,muxid);
	throw "Tunnel::Tunnel(): ioctl(SIOCSIFMUXID): FIXME";
    }

#if 0
    // FIXME: multi-AF support
    int head = header?1:0;
    // include 4byte header "multi-af"
    if (0 > ioctl(fd,TUNSIFHEAD,(void*)&head)) {
	close(fd);
	throw "Tunnel::Tunnel(): ioctl(): FIXME";
    }
#endif
}

/**
 *
 */
void
Tunnel::osdepDestructor()
{
    if (0 > udpfd) {
	return;
    }
    struct ifreq ifr;
    memset(&ifr,0,sizeof(ifr));
    strncpynt (ifr.ifr_name, devname.c_str(), sizeof (ifr.ifr_name));

    if (ioctl (udpfd, SIOCGIFFLAGS, &ifr) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }

    if (ioctl (udpfd, SIOCGIFMUXID, &ifr) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }

    if (ioctl (udpfd, I_PUNLINK, ifr.ifr_ip_muxid) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }

    close(udpfd);
    udpfd = -1;
}

/**
 *
 */
bool
Tunnel::write(const std::string &s)
{
    struct strbuf sbuf;
    sbuf.len = len;
    sbuf.buf = (char *)buf;
    return putmsg (fd, NULL, &sbuf, 0) >= 0 ? sbuf.len : -1;
}

/**
 *
 */
std::string Tunnel::read()
{
    struct strbuf sbuf;
    int f = 0;
    
    sbuf.maxlen = len;
    sbuf.buf = (char *)buf;
    if (0 > getmsg(fd, NULL, &sbuf, &f)) {
	throw "Tunnel::read(): FIXME";
    }
    return std::string(sbuf.buf,sbuf.buf+sbuf.len);
}

#endif /* __SunOS__ */
/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
