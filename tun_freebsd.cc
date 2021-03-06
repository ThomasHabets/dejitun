/** dejitun/tun_freebsd.cc
 *
 */
#ifdef __FreeBSD__
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
Tunnel::Tunnel(const std::string &dev,bool header)
{
    memset(&stats, 0, sizeof(stats));
    
    if (0 > (fd.fd = open((std::string("/dev/") + dev).c_str(),O_RDWR))) {
	throw ErrSys("Tunnel::Tunnel(): open(/dev/" + dev + ") ");
    }

    struct stat st;
    if (0 > fstat(fd.fd, &st)) {
	throw ErrSys("Tunnel::Tunnel(): fstat(/dev/tun)");
    }
    
    devname = ::devname(st.st_rdev,S_IFCHR);

    int head = header?1:0;
    // include 4byte header "multi-af"
    if (0 > ioctl(fd.fd,TUNSIFHEAD,(void*)&head)) {
	throw ErrSys("Tunnel::Tunnel(): ioctl(TUNSIFHEAD)");
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
