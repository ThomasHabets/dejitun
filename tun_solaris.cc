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

    if (0 > (udpfd.fd = open("/dev/udp",O_RDWR))) {
	throw "Tunnel::Tunnel(): open(/dev/udp): FIXME";
    }
    if (0 > (fd.fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	throw "Tunnel::Tunnel(): open(/dev/tun): FIXME";
    }
    if (0 > (ppa = ioctl(fd.fd, TUNNEWPPA, ppa))) {
	throw "Tunnel::Tunnel(): ioctl(TUNNEWPPA): FIXME";
    }
    {
	LLFDWrap tfd;
	if (0 > (tfd.fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	    throw "Tunnel::Tunnel(): open(/dev/tun)(2): FIXME";
	}

	if (0 > (ioctl(tfd.fd,I_PUSH,"ip"))) {
	    throw "Tunnel::Tunnel(): ioctl(I_PUSH): FIXME";
	}
	if (0 > (ppa = ioctl(tfd.fd, I_UNITSEL, (char*)ppa))) {
	    throw "Tunnel::Tunnel(): ioctl(TUNNEWPPA): FIXME";
	}
	if (0 > (muxid = ioctl(udpfd.fd, I_PLINK, tft))) {
	    throw "Tunnel::Tunnel(): ioctl(I_PLINK): FIXME";
	}
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

    if (0 > ioctl(udpfd.fd, SIOCSIFMUXID, &ifr) < 0) {
	ioctl(udpfd.fd,I_PULINK,muxid);
	throw "Tunnel::Tunnel(): ioctl(SIOCSIFMUXID): FIXME";
    }

#if 0
    // FIXME: multi-AF support
    int head = header?1:0;
    // include 4byte header "multi-af"
    if (0 > ioctl(fd.fd,TUNSIFHEAD,(void*)&head)) {
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

    if (ioctl (udpfd.fd, SIOCGIFFLAGS, &ifr) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }

    if (ioctl (udpfd.fd, SIOCGIFMUXID, &ifr) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }

    if (ioctl (udpfd.fd, I_PUNLINK, ifr.ifr_ip_muxid) < 0) {
	throw "Tunnel::solarisDestructor(): ioctl(): FIXME";
    }
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
    return putmsg (fd.fd, NULL, &sbuf, 0) >= 0 ? sbuf.len : -1;
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
    if (0 > getmsg(fd.fd, NULL, &sbuf, &f)) {
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
