/** dejitun/dejitun.h
 *
 */
#include<string>
#include<fstream>

#include<sys/types.h>

#include"util.h"

#ifdef __GNUC__
#define ATTR_PACKED __attribute__ ((__packed__))
#elif defined(__SUNPRO_CC)
#define ATTR_PACKED
#else
#error Unknown compiler
#endif

/**
 *
 */
class Dejitun {
public:
    static const std::string defaultTunnelDevice;
    static const int defaultListenPort;
    class Options {
    public:
	std::string peer;
	int remotePort;
	int localPort;
	double minDelay;
	double maxDelay;
	double jitter;
	std::string tunnelDevice;
	bool multiAF;
	std::string debugfile;
	bool daemonize;
	Options()
	    :
	    remotePort(-1),
	    localPort(defaultListenPort),
	    minDelay(0),
	    maxDelay(10),
	    jitter(0),
	    tunnelDevice(defaultTunnelDevice),
	    multiAF(true),
	    debugfile("/dev/null"),
	    daemonize(true)
	{
	}
    };
    Options options;
    static const unsigned char protocolVersion;

    Tunnel tun;
    Inet inet;
    Dejitun(const Options&opts)
	:options(opts),
	 tun(opts.tunnelDevice,opts.multiAF),
	 inet(opts.peer, opts.remotePort, opts.localPort)
    {
	cdebug.open(options.debugfile.c_str());
    }
    const std::string &getDevname() const { return tun.getDevname(); }
    void run();
    std::ofstream cdebug;

protected:
    // over-the-wire protocol
#pragma pack(1)
    struct Packet {
	char version;
	// ms since 1970
	int64_t   minTime ATTR_PACKED;
	int64_t   maxTime ATTR_PACKED;
	uint32_t  jitter  ATTR_PACKED;
	char      payload[0];
    };
#pragma pack()

    typedef struct {
	Packet *packet;
	size_t len;
	FDWrapper *dev;
    } PacketEntry;
    void writePacket(Packet *p, size_t len, FDWrapper*dev);

    /**
     * Packet scheduler version 1.
     * It stinks! No support for jitter.
     */
    std::list<PacketEntry> packetQueue;
    void schedulePacket(Packet *p, size_t len, FDWrapper *dev);
    void packetWriter();
};

/**
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
