#include "IpHeader.h"

QDataStream &Protocol::NetworkLayer::operator<<(QDataStream &stream, const Protocol::NetworkLayer::IpHeader &object)
{
    return stream << object.sourceIp << object.targetIp << object.hops << object.ttl;
}

QDataStream &Protocol::NetworkLayer::operator>>(QDataStream &stream, Protocol::NetworkLayer::IpHeader &object)
{
    return stream >> object.sourceIp >> object.targetIp >> object.hops >> object.ttl;
}
