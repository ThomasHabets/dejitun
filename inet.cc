#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>

#include "util.h"

Inet::Inet(const std::string &host,int port,int lport)
	:peer(0)
{
	if (0 > (fd = socket(PF_INET, SOCK_DGRAM, 0))) {
		throw "Inet::Inet(): socket(): FIXME";
	}
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = INADDR_ANY;
	if (lport >= 0) {
		if (bind(fd,
			 (struct sockaddr*)&sa,
			 sizeof(struct sockaddr_in))) {
			close(fd);
			fd = -1;
			throw "Inet::Inet(): bind(): FIXME";
		}
	}
	struct sockaddr_in *t = new struct sockaddr_in;
	t->sin_addr.s_addr = inet_addr(host.c_str());
	t->sin_family = AF_INET;
	t->sin_port = htons(port);
	peer = (struct sockaddr*)t;
	listen(fd,10);
}

// returns false for warnings
bool
Inet::write(const std::string &s)
{
	ssize_t n;
	n = s.length();
	if (n != ::sendto(fd,
			  s.data(),s.length(),
			  0,
			  peer,
			  sizeof(sockaddr_storage))) {
		if (n < 0) {
			stats.writeError++;
			throw "FDWrapper::write(): FIXME";
		}
		stats.shortWrite++;
		return false;
	}
	return true;
}

