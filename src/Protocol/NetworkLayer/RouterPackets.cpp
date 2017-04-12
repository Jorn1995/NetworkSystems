#include "RouterPackets.h"

namespace Protocol {
namespace NetworkLayer {

QDataStream &operator<<(QDataStream &stream, const Node &object)
{
    return stream << object.id << object.seqNum;
}

QDataStream &operator>>(QDataStream &stream, Node &object)
{
    return stream >> object.id >> object.seqNum;
}

QDataStream &operator<<(QDataStream &stream, const RouteRequest &object)
{
    return stream << object.dest << object.originator << object.routeRequestID << object.hopCount << object.ttl;
}

QDataStream &operator>>(QDataStream &stream, RouteRequest &object)
{
    return stream >> object.dest >> object.originator >> object.routeRequestID >> object.hopCount >> object.ttl;
}

QDataStream &operator<<(QDataStream &stream, const RouteReply &object)
{
    return stream << object.dest << object.originatorID << object.hopCount << object.validTill;
}

QDataStream &operator>>(QDataStream &stream, RouteReply &object)
{
    return stream >> object.dest >> object.originatorID >> object.hopCount >> object.validTill;
}

QDataStream &operator<<(QDataStream &stream, const RouteError &object)
{
    return stream << object.unreachables;
}

QDataStream &operator>>(QDataStream &stream, RouteError &object)
{
    return stream >> object.unreachables;
}

Node::Node()
{

}

} // namespace NetworkLayer
} // namespace Protocol
