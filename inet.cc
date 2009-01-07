/** dejitun/inet.cc
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "util.h"

/**
 * FIXME: AF-independent
 */
Inet::Inet(const std::string &host,int port,int lport)
    :peer(0)
{
    if (0 > (fd.fd = socket(PF_INET, SOCK_DGRAM, 0))) {
	throw ErrSys("Inet::Inet(): socket(PF_INET,SOCK_DGRAM)");
    }
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(lport);
    sa.sin_addr.s_addr = INADDR_ANY;
    if (lport >= 0) {
	if (bind(fd.fd,
		 (struct sockaddr*)&sa,
		 sizeof(struct sockaddr_in))) {
	    throw ErrSys("Inet::Inet(): bind()");
	}
    }
    struct sockaddr_in *t = new struct sockaddr_in;
    t->sin_addr.s_addr = inet_addr(host.c_str());
    t->sin_family = AF_INET;
    t->sin_port = htons(port);
    peer = (struct sockaddr*)t;
    peerlen = sizeof(struct sockaddr_in);
}

/**
 *  returns false for warnings
 */
bool
Inet::write(const std::string &s)
{
    ssize_t n;
    n = s.length();
    if (n != ::sendto(fd.fd,
		      s.data(),s.length(),
		      0,
		      peer,
		      peerlen)) {
	if (n < 0) {
	    stats.writeError++;
	    throw ErrSys("FDWrapper::write(): sendto()");
	}
	stats.shortWrite++;
	return false;
    }
    return true;
}

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
