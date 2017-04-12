#ifndef PROTOCOL_NETWORKLAYER_ROUTER_H
#define PROTOCOL_NETWORKLAYER_ROUTER_H

#include <QHostAddress>
#include <QMap>
#include <QQueue>
#include <QObject>
#include <QTimer>
#include "../TransportLayer/Flag.h"

#include "IpHeader.h"

#include "HigherProtocolInterface.h"
#include "RouterPackets.h"

class QUdpSocket;

bool operator<(const QHostAddress & a, const QHostAddress & b);


namespace Protocol {
namespace TransportLayer {
class RawData;
class ReliableLink;
}

namespace NetworkLayer {

class Router : public QObject {
  friend class HigherProtocolInterface;

  Q_OBJECT

  struct RoutingInformation {
    qint8 destSeqNum = 0;
    qint8 hopCount = 0;
    QHostAddress nextHop;
    QHostAddress previousHop;
    QDateTime validTill;

    QList<qint8> seenRequestIDs;
  };

  qint8 m_routeRequestID = 0;
  qint8 m_routeSeqNum = 0;

  QUdpSocket *m_socket;

  QMap<qint8, RoutingInformation> m_nodes;
//  QMap<qint8, QHostAddress> m_nodeIP;
  QMap<QHostAddress, QDateTime> m_neighborTimeouts;
  QMap<QByteArray, QDateTime> m_seenDatagrams;

  QQueue<QByteArray> m_packets;

  QList<HigherProtocolInterface *> m_listeners;

  QTimer m_cleanTimeout;

  void sendRouteRequest(qint8 destID);
  void handleRouteRequest(RouteRequest request, QHostAddress from);
  void handleRouteReply(RouteReply reply, QHostAddress from_address);
  void handleRouteError(RouteError error, QHostAddress from_address);

  void sendRouteReplyAsDestination(qint8 originatorID, qint8 hopCount);
  void sendRouteReplyAsItermediate(qint8 originatorID, qint8 destID, qint8 hopCount);

  void sendDetectedRouteError(RouteError error);
  void sendRouteErrorInvalidPacket();

  qint8 resolveHostAddress(QHostAddress address);

protected:
  // Higher Protocol callbacks
  void registerHigherProtocol(HigherProtocolInterface *self);
  void deregisterHigherProtocol(HigherProtocolInterface *self);

  void writePacket(qint8 target, NextHeaderType nextHeader,
                   const QByteArray &payload);

  void routePacket(const IpHeader &header, const QByteArray &datagram,
                   bool generated);

private slots:
  void fetchPacket();
  void cleanSeenPackets();
  void scanLostNodes();

public:
  explicit Router(QObject *parent = 0);
  ~Router();
};
}
} // namespace Protocol

#endif // PROTOCOL_NETWORKLAYER_ROUTER_H
