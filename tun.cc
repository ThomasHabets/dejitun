#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/if.h>
#include <linux/if_tun.h>



class Tunnel {
	int fd;
	std::string devname;
public:
	struct {
		uint64_t shortWrite;
		uint64_t readError;
		uint64_t writeError;
	} stats;
	Tunnel(const std::string &dev)
	{
		memset(&stats, 0, sizeof(stats));

		if (0 > (fd = open("/dev/net/tun",O_RDWR))) {
			throw "FIXME";
		}
		
		struct ifreq ifr;
		memset(&ifr,0,sizeof(struct ifreq));
		ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
		strncpy(ifr.ifr_name,dev.c_str(),IFNAMSIZ);
		if (0 > ioctl(fd,TUNSETIFF,(void*)&ifr)) {
			close(fd);
			throw "FIXME";
		}
		devname = ifr.ifr_name;
	}
	const std::string &getDevname() const { return devname; }
	int getFd() const { return fd; }
	~Tunnel()
	{
		close(fd);
	}

	// returns false for warnings
	bool write(const std::string &s)
	{
		ssize_t n;
		n = s.length();
		if (n != ::write(fd,s.data(),s.length())) {
			if (n < 0) {
				stats.writeError++;
				throw "FIXME";
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
			throw "FIXME";
		}
		return std::string(buf,&buf[n]);
	}
};
