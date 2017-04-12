#ifndef PROTOCOL_NETWORKLAYER_ROUTEREPLY_H
#define PROTOCOL_NETWORKLAYER_ROUTEREPLY_H

#include <QDateTime>
#include <QList>

#include <QDataStream>

namespace Protocol {
namespace NetworkLayer {

struct Node {
  qint8 id = 0;
  qint8 seqNum = 0;

public:
  Node();
  Node(qint8 _id, qint8 _seqNum) {
    id = _id;
    seqNum = _seqNum;
  }
};

QDataStream &operator<<(QDataStream &stream, const Node &object);
QDataStream &operator>>(QDataStream &stream, Node &object);

struct RouteRequest {
  Node dest;
  Node originator;
  qint8 hopCount = 0;
  qint8 routeRequestID = 0;
  qint8 ttl = 8;
};

QDataStream &operator<<(QDataStream &stream, const RouteRequest &object);
QDataStream &operator>>(QDataStream &stream, RouteRequest &object);

struct RouteReply {
  Node dest;
  qint8 originatorID;
  qint8 hopCount = 0;
  QDateTime validTill;
};

QDataStream &operator<<(QDataStream &stream, const RouteReply &object);
QDataStream &operator>>(QDataStream &stream, RouteReply &object);

struct RouteError {
  QList<Node> unreachables;
};

QDataStream &operator<<(QDataStream &stream, const RouteError &object);
QDataStream &operator>>(QDataStream &stream, RouteError &object);

} // namespace NetworkLayer
} // namespace Protocol

#endif // PROTOCOL_NETWORKLAYER_ROUTEREPLY_H
