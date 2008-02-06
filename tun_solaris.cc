#if defined (__SVR4) && defined (__sun)
#include<sys/sockio.h>
#include<net/if.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stropts.h>
#include<net/if_tun.h>

#include"util.h"

/**
 *
 */
Tunnel::Tunnel(const std::string &dev,bool header)
{
    int muxid,ppa = -1;
    struct ifreq ifr;

    if (0 > (udpfd.fd = open("/dev/udp",O_RDWR))) {
	throw ErrSys("Tunnel::Tunnel(): open(/dev/udp)");
    }
    if (0 > (fd.fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	throw ErrSys("Tunnel::Tunnel(): open(/dev/tun)");
    }
    if (0 > (ppa = ioctl(fd.fd, TUNNEWPPA, ppa))) {
	throw ErrSys("Tunnel::Tunnel(): ioctl(TUNNEWPPA)");
    }
    {
	LLFDWrap tfd;
	if (0 > (tfd.fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	    throw ErrSys("Tunnel::Tunnel(): open(/dev/tun)(2)");
	}

	if (0 > (ioctl(tfd.fd,I_PUSH,"ip"))) {
	    throw ErrSys("Tunnel::Tunnel(): ioctl(I_PUSH)");
	}
	if (0 > (ppa = ioctl(tfd.fd, IF_UNITSEL, (char*)&ppa))) {
	    throw ErrSys("Tunnel::Tunnel(): ioctl(IF_UNITSEL)");
	}
	if (0 > (muxid = ioctl(udpfd.fd, I_PLINK, tfd.fd))) {
	    throw ErrSys("Tunnel::Tunnel(): ioctl(I_PLINK)");
	}
    }

    // Get name
    {
	char buf[32];
	snprintf(buf,sizeof(buf),"tun%d",ppa);
	devname = buf;
    }

    memset(&ifr,0,sizeof(ifr));
    strncpy(ifr.ifr_name, devname.c_str(), sizeof(ifr.ifr_name));
    ifr.ifr_ip_muxid = muxid;

    if (0 > ioctl(udpfd.fd, SIOCSIFMUXID, &ifr) < 0) {
	ioctl(udpfd.fd,I_PLINK,muxid);
	throw ErrSys("Tunnel::Tunnel(): ioctl(SIOCSIFMUXID)");
    }

#if 0
    // FIXME: multi-AF support
    int head = header?1:0;
    // include 4byte header "multi-af"
    if (0 > ioctl(fd.fd,TUNSIFHEAD,(void*)&head)) {
	throw ErrSys("Tunnel::Tunnel(): ioctl()");
    }
#endif
}

/**
 *
 */
void
Tunnel::osdepDestructor()
{
    if (0 > udpfd.fd) {
	return;
    }
    struct ifreq ifr;
    memset(&ifr,0,sizeof(ifr));
    strncpy(ifr.ifr_name, devname.c_str(), sizeof (ifr.ifr_name));

    if (ioctl (udpfd.fd, SIOCGIFFLAGS, &ifr) < 0) {
	throw ErrSys("Tunnel::solarisDestructor(): ioctl(SIOCGIFFLAGS)");
    }

    if (ioctl (udpfd.fd, SIOCGIFMUXID, &ifr) < 0) {
	throw ErrSys("Tunnel::solarisDestructor(): ioctl(SIOCGIFMUXID)");
    }

    if (ioctl (udpfd.fd, I_PUNLINK, ifr.ifr_ip_muxid) < 0) {
	throw ErrSys("Tunnel::solarisDestructor(): ioctl(I_PUNLINK)");
    }
}

/**
 *
 */
bool
Tunnel::write(const std::string &s)
{
    struct strbuf sbuf;
    sbuf.len = s.length();
    sbuf.buf = (char *)s.data();
    return putmsg(fd.fd, NULL, &sbuf, 0) >= 0 ? sbuf.len : -1;
}

/**
 *
 */
std::string Tunnel::read()
{
    char buf[10240];
    struct strbuf sbuf;
    int f = 0;
    
    sbuf.maxlen = sizeof(buf);
    sbuf.buf = (char *)buf;
    if (0 > getmsg(fd.fd, NULL, &sbuf, &f)) {
	throw ErrSys("Tunnel::read(): getmsg()");
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
