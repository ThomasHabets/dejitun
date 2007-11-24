#include<string>
#include<sys/types.h>

#include"util.h"

class Dejitun {
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

    std::list<PacketEntry> packetQueue;
    void
    schedulePacket(Packet *p, size_t len, FDWrapper *dev);
    void
    writePacket(Packet *p, size_t len, FDWrapper*dev);
    void
    packetWriter();
public:	
    void
    run(const std::string &rhost, int rport, int lport);
};


/*
 * Local variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
