#ifndef HIGHERPROTOCOLINTERFACE_H
#define HIGHERPROTOCOLINTERFACE_H

#include <QByteArray>

#include "IpHeader.h"

namespace Protocol {
namespace NetworkLayer {

class Router;

class HigherProtocolInterface
{
    Router * m_owner;

public:
    HigherProtocolInterface(Router * owner);
    virtual ~HigherProtocolInterface();

public:
    virtual bool handlePacket(qint8 target, NextHeaderType nextHeader, const QByteArray & data) = 0;
    void sendPacket(qint8 target, NextHeaderType nextHeader, const QByteArray & data);
};

}
}
#endif // HIGHERPROTOCOLINTERFACE_H
