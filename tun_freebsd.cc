#if 1
#include<sys/stat.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<sys/stat.h>
#include<net/if_tun.h>

#include"util.h"

/**
 *
 */
Tunnel::Tunnel(const std::string &dev)
{
    memset(&stats, 0, sizeof(stats));
    
    if (0 > (fd = open("/dev/tun8",O_RDWR))) {
	throw "Tunnel::Tunnel(): open(/dev/tun): FIXME";
    }

    struct stat st;
    if (0 > fstat(fd, &st)) {
	throw "Tunnel::Tunnel(): fstat(/dev/tun): FIXME";
    }
    
    devname = ::devname(st.st_rdev,S_IFCHR);

    int on = 1;
    int off = 0;
    // include 4byte header "multi-af"
    if (0 > ioctl(fd,TUNSIFHEAD,(void*)&off)) {
	close(fd);
	throw "Tunnel::Tunnel(): ioctl(): FIXME";
    }
}

// for reading, use FIONREAD
#endif /* __FreeBSD__ */
/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
