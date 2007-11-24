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
Tunnel::Tunnel(const std::string &dev)
{
    memset(&stats, 0, sizeof(stats));
    
    if (0 > (fd = open("/dev/net/tun",O_RDWR))) {
	throw "Tunnel::Tunnel(): FIXME";
    }
    
    struct ifreq ifr;
    memset(&ifr,0,sizeof(struct ifreq));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name,dev.c_str(),IFNAMSIZ);
    if (0 > ioctl(fd,TUNSETIFF,(void*)&ifr)) {
	close(fd);
	throw "Tunnel::Tunnel(): ioctl()";
    }
    devname = ifr.ifr_name;
}
/*
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
