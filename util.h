#include<string>
#include<sys/types.h>
#include<sys/socket.h>

class LLFDWrap {
protected:
    LLFDWrap(const LLFDWrap&);
    const LLFDWrap &operator=(const LLFDWrap&) const;
public:
    int fd;
    LLFDWrap(): fd(-1) {}
    void close()  { if (fd > 0) { ::close(fd); } }
    ~LLFDWrap() { close(); }
};

/**
 *
 */
class FDWrapper {
protected:
    int fd;
    virtual void osdepDestructor() {}
public:
    struct {
	uint64_t shortWrite;
	uint64_t readError;
	uint64_t writeError;
    } stats;
    FDWrapper(): fd(-1) {}
    int getFd() const { return fd; }

    // returns false for warnings. FIXME: change this?
    virtual bool write(const std::string &s);
    virtual std::string read();
    virtual ~FDWrapper();
};

/**
 *
 */
class Inet: public FDWrapper {
    struct sockaddr *peer;
    socklen_t peerlen;
public:
    Inet(const std::string &host,int port,int lport = -1);
    bool write(const std::string &s);
};

/**
 *
 */
class Tunnel: public FDWrapper {
    std::string devname;
#ifdef __SunOS__
    int udpfd;
    void osdepDestructor();
#endif
public:
    Tunnel(const std::string &dev,bool header=true);
    const std::string &getDevname() const { return devname; }
};


/**
 *
 */
int64_t htonll(int64_t n);

/**
 *
 */
int64_t ntohll(int64_t n);

/**
 *
 */
int64_t gettimeofdaymsec();

/**
 *
 */
int64_t f2i64(double f);

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
