#ifndef IPHEADER_H
#define IPHEADER_H

#include <QtCore>

namespace Protocol {
namespace NetworkLayer {

enum NextHeaderType {
    Empty = 0,
    ReliableLink,
    DataPacket,
};

struct IpHeader
{
    qint8 sourceIp, targetIp;
    qint8 hops, ttl;
    qint8 nextHeader;
};

QDataStream & operator<< (QDataStream & stream, const IpHeader & object);
QDataStream & operator>> (QDataStream & stream, IpHeader & object);

}
}

#endif // IPHEADER_H
