/** dejitun/tun_linux.cc
 *
 */
#ifdef __linux__
#include<string>

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<syslog.h>
#include<errno.h>

#include<sys/ioctl.h>
#include<sys/types.h>
#include<sys/socket.h>

#include<linux/if.h>
#include<linux/if_tun.h>
#include"util.h"

/**
 *
 */
Tunnel::Tunnel(const std::string &dev, bool header)
{
    memset(&stats, 0, sizeof(stats));
    
    if (0 > (fd.fd = open("/dev/net/tun",O_RDWR))) {
	throw ErrSys("Tunnel::Tunnel(): open(/dev/net/tun)");
    }
    
    struct ifreq ifr;
    memset(&ifr,0,sizeof(struct ifreq));

    ifr.ifr_flags = IFF_TUN;

    if (!header) {
	// AF_INET-only tunnel. For debugging and porting.
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    }

    strncpy(ifr.ifr_name,dev.c_str(),IFNAMSIZ);
    if (0 > ioctl(fd.fd,TUNSETIFF,(void*)&ifr)) {
	throw ErrSys("Tunnel::Tunnel(): ioctl(TUNSETIFF)");
    }
    devname = ifr.ifr_name;
}
#endif /* __LINUX__ */
/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
