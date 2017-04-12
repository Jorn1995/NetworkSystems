#ifndef PROTOCOL_APPLICATIONLAYER_PRESENCEBROADCASTER_H
#define PROTOCOL_APPLICATIONLAYER_PRESENCEBROADCASTER_H

#include <Protocol/TransportLayer/RawData.h>



namespace Protocol {

namespace NetworkLayer {
class Router;
}

namespace ApplicationLayer {

class PresenceBroadcaster : public TransportLayer::RawData
{
public:
    PresenceBroadcaster(NetworkLayer::Router * router);
};

} // namespace ApplicationLayer
} // namespace Protocol

#endif // PROTOCOL_APPLICATIONLAYER_PRESENCEBROADCASTER_H
