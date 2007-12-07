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
	throw "gettimeofday(): FIXME";
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
    if (n != ::write(fd,s.data(),n)) {
	if (n < 0) {
	    stats.writeError++;
	    throw "FDWrapper::write(): FIXME";
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
    if (0 > (n = ::read(fd,buf,sizeof(buf)))) {
	stats.readError++;
	throw "FDWrapper::read(): FIXME";
    }
    return std::string(buf,&buf[n]);
}

/**
 *
 */
FDWrapper::~FDWrapper()
{
    if (fd >= 0) {
	close(fd);
	fd = -1;
    }
    osdepDestructor();
}

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
