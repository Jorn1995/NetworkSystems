#include "HigherProtocolInterface.h"

#include "Router.h"

namespace Protocol {
namespace NetworkLayer {

HigherProtocolInterface::HigherProtocolInterface(Router *owner) : m_owner(owner)
{

}

void HigherProtocolInterface::sendPacket(qint8 target, const QByteArray &data)
{
    m_owner->writePacket(target, data);
}

Protocol::NetworkLayer::HigherProtocolInterface::~HigherProtocolInterface()
{
    m_owner->deregisterHigherProtocol(this);
}

}
}
