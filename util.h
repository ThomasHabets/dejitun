#include<string>

#include<string.h>
#include<errno.h>
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
    LLFDWrap fd;
    virtual void osdepDestructor() {}
public:
    class ErrBase: public std::exception {
	const std::string msg;
    public:
	ErrBase(const std::string &msg_):msg(msg_) {}
	virtual const char *what() const throw() { return msg.c_str(); }
	virtual ~ErrBase() throw() {};
    };
    class ErrSys: public ErrBase {
    public:
	ErrSys(const std::string &msg)
	    :ErrBase(msg + ": " + strerror(errno))
	    {
	    }
    };
    
    struct {
	uint64_t shortWrite;
	uint64_t readError;
	uint64_t writeError;
    } stats;
    int getFd() const { return fd.fd; }

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
    LLFDWrap udpfd;
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
