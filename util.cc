#include<time.h>
#include<sys/time.h>

#include"util.h"

/**
 *
 */
int64_t
htonll(int64_t n)
{
    return n; // FIXME
}

/**
 *
 */
int64_t
ntohll(int64_t n)
{
    return htonll(n);
}

/**
 *
 */
int64_t
gettimeofdaymsec()
{
    struct timeval tv;
    if (-1 == gettimeofday(&tv, NULL)) {
	throw std::string("gettimeofday(): ") + strerror(errno);
    }
    return tv.tv_sec * int64_t(1000) + tv.tv_usec/1000;
}

/**
 *
 */
int64_t f2i64(double f)
{
    return (int64_t)(f*1000);
}


/**
 *
 */
bool
FDWrapper::write(const std::string &s)
{
    ssize_t n;
    n = s.length();
    if (n != ::write(fd.fd,s.data(),n)) {
	if (n < 0) {
	    stats.writeError++;
	    throw ErrSys("FDWrapper::write(): write()");
	}
	stats.shortWrite++;
	return false;
    }
    return true;
}

/**
 *
 */
std::string
FDWrapper::read()
{
    char buf[102400]; // 100k enough?
    ssize_t n;
    if (0 > (n = ::read(fd.fd,buf,sizeof(buf)))) {
	stats.readError++;
	throw ErrSys("FDWrapper::read(): read()");
    }
    return std::string(buf,&buf[n]);
}

/**
 *
 */
FDWrapper::~FDWrapper()
{
    osdepDestructor();
}

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
