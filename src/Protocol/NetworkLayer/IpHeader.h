#ifndef IPHEADER_H
#define IPHEADER_H

#include <QtCore>

namespace Protocol {
namespace NetworkLayer {

struct IpHeader
{
    qint8 sourceIp, targetIp;
    qint8 hops, ttl;
};

QDataStream & operator<< (QDataStream & stream, const IpHeader & object);
QDataStream & operator>> (QDataStream & stream, IpHeader & object);

}
}

#endif // IPHEADER_H
