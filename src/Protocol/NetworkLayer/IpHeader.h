#ifndef IPHEADER_H
#define IPHEADER_H

#include <QtCore>

namespace Protocol {
namespace NetworkLayer {

enum NextHeaderType : qint8 {
    Empty = 0,
    ReliableLink,
    DataPacket,
    RouteRequestPacket,
    RouteReplyPacket,
    RouteErrorPacket,
};

struct IpHeader
{
    qint8 sourceIp = 0, targetIp = 0;
    qint8 hops = 0, ttl = 8;
    qint8 nextHeader = Empty;
};

QDataStream & operator<< (QDataStream & stream, const IpHeader & object);
QDataStream & operator>> (QDataStream & stream, IpHeader & object);

}
}

#endif // IPHEADER_H
