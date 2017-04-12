#include "PresenceBroadcaster.h"

namespace Protocol {
namespace ApplicationLayer {

PresenceBroadcaster::PresenceBroadcaster(NetworkLayer::Router *router) : TransportLayer::RawData(router) {}

} // namespace ApplicationLayer
} // namespace Protocol
