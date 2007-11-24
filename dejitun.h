#include<string>
#include<sys/types.h>

#include"util.h"

/**
 *
 */
class Dejitun {
public:	
    class Options {
    public:
	std::string peer;
	int remotePort;
	int localPort;
	double minDelay;
	double maxDelay;
	double jitter;
	std::string tunnelDevice;
	Options()
	    :
	    remotePort(12345),
	    localPort(12345),
	    minDelay(0),
	    maxDelay(10),
	    jitter(0),
	    tunnelDevice("dejitun%d")
	{
	}
    };
    Options options;
    static const unsigned char protocolVersion;

    Tunnel tun;
    Inet inet;
    Dejitun(const Options&opts)
	:options(opts),
	 tun(opts.tunnelDevice),
	 inet(opts.peer, opts.remotePort, opts.localPort)
    {
    }
    const std::string &getDevname() const { return tun.getDevname(); }
    void run();

protected:
    // over-the-wire protocol
    struct Packet {
	char version;
	// ms since 1970
	int64_t   minTime __attribute__ ((__packed__));
	int64_t   maxTime __attribute__ ((__packed__));
	uint32_t  jitter  __attribute__ ((__packed__));
	char      payload[0];
    };

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
