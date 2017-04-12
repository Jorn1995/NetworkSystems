#include "IpHeader.h"
#include "Router.h"
#include <QDataStream>
#include <QUdpSocket>
#include "config.h"
#include <Protocol/TransportLayer/RawData.h>
#include <Protocol/TransportLayer/ReliableLink.h>

namespace Protocol {
namespace NetworkLayer {

void Router::sendRouteRequest(qint8 destID) {
  m_routeRequestID++;
  m_routeSeqNum++;

  RouteRequest request;

  request.dest.id = destID;
  request.dest.seqNum = m_nodes[destID].destSeqNum;
  request.originator.id = MY_NODE_IP;
  request.originator.seqNum = m_routeSeqNum;
  request.routeRequestID = m_routeRequestID;

  QByteArray packet;
  {
    QDataStream writer(&packet, QIODevice::WriteOnly);

    writer << request;
  }

  writePacket(0, RouteRequestPacket, packet);
}

void Router::handleRouteRequest(RouteRequest request,
                                QHostAddress from_address) {
  if (m_nodes[request.dest.id].seenRequestIDs.contains(
          request.routeRequestID)) {
    return;
  }

  RoutingInformation &entry = m_nodes[request.dest.id];
  entry.seenRequestIDs << request.routeRequestID;

  request.hopCount++;
  request.ttl--;

  RoutingInformation &reverseEntry = m_nodes[request.originator.id];

  if (request.originator.seqNum > reverseEntry.destSeqNum ||
      (request.originator.seqNum == reverseEntry.destSeqNum &&
       request.hopCount <= reverseEntry.hopCount)) {

    reverseEntry.destSeqNum = request.originator.seqNum;
    reverseEntry.hopCount = request.hopCount;
    reverseEntry.nextHop = from_address;
    reverseEntry.validTill = QDateTime::currentDateTimeUtc().addSecs(60);
  }

  if (request.dest.id == MY_NODE_IP) {
    sendRouteReplyAsDestination(request.originator.id, request.hopCount);
  } else if (!entry.nextHop.isNull()) {
    sendRouteReplyAsItermediate(request.originator.id, request.dest.id,
                                request.hopCount);
  } else if (request.ttl > 0) {
    QByteArray packet;
    {
      QDataStream writer(&packet, QIODevice::WriteOnly);

      writer << request;
    }

    writePacket(0, RouteRequestPacket, packet);
  }
}

void Router::handleRouteReply(RouteReply reply, QHostAddress from_address) {
  if (reply.originatorID == MY_NODE_IP) {
    RoutingInformation &entry = m_nodes[reply.dest.id];

    entry.destSeqNum = reply.dest.seqNum;
    entry.hopCount = reply.hopCount;
    entry.nextHop = from_address;

    entry.validTill = QDateTime::currentDateTimeUtc().addSecs(60);
  } else {
    RoutingInformation &entry = m_nodes[reply.dest.id];

    entry.destSeqNum = reply.dest.seqNum;
    entry.hopCount = reply.hopCount - m_nodes[reply.originatorID].hopCount;
    entry.previousHop = entry.nextHop;
    entry.nextHop = m_nodes[reply.originatorID].nextHop;

    entry.validTill = QDateTime::currentDateTimeUtc().addSecs(60);

    QByteArray packet;
    {
      QDataStream writer(&packet, QIODevice::WriteOnly);

      writer << reply;
    }

    writePacket(reply.originatorID, RouteReplyPacket, packet);
  }
}

void Router::handleRouteError(RouteError error, QHostAddress from_address) {
  RouteError routeError;

  for (Node node : error.unreachables) {
    if (m_nodes[node.id].nextHop == from_address) {
      m_nodes[node.id].nextHop.clear();

      routeError.unreachables << node;
    }
  }

  if (routeError.unreachables.size() > 0) {
    sendDetectedRouteError(routeError);
  }
}

void Router::sendRouteReplyAsDestination(qint8 originatorID, qint8 hopCount) {
  m_routeSeqNum++;

  RouteReply reply;

  reply.dest.id = MY_NODE_IP;
  reply.dest.seqNum = m_routeSeqNum;
  reply.originatorID = originatorID;
  reply.hopCount = hopCount;
  reply.validTill = QDateTime::currentDateTimeUtc().addSecs(60);

  QByteArray packet;
  {
    QDataStream writer(&packet, QIODevice::WriteOnly);

    writer << reply;
  }

  writePacket(originatorID, RouteReplyPacket, packet);
}

void Router::sendRouteReplyAsItermediate(qint8 originatorID, qint8 destID,
                                         qint8 hopCount) {
  RouteReply reply;

  reply.dest.id = destID;
  reply.dest.seqNum = m_nodes[destID].destSeqNum;
  reply.originatorID = originatorID;
  reply.hopCount = m_nodes[destID].hopCount + hopCount;
  reply.validTill = QDateTime::currentDateTimeUtc().addSecs(60);

  QByteArray packet;
  {
    QDataStream writer(&packet, QIODevice::WriteOnly);

    writer << reply;
  }

  writePacket(originatorID, RouteReplyPacket, packet);
}

void Router::sendDetectedRouteError(RouteError error) {
  QByteArray packet;
  {
    QDataStream writer(&packet, QIODevice::WriteOnly);

    writer << error;
  }

  QMapIterator<qint8, RoutingInformation> iter(m_nodes);

  while (iter.hasNext()) {
    iter.next();

    if (iter.value().hopCount == 1) {
      writePacket(iter.key(), RouteErrorPacket, packet);
    }
  }
}

qint8 Router::resolveHostAddress(QHostAddress address) {
  QMapIterator<qint8, RoutingInformation> iter(m_nodes);

  while (iter.hasNext()) {
    iter.next();

    if (iter.value().hopCount == 1 && iter.value().nextHop == address) {
      return iter.key();
    }
  }

  return 0;
}

void Router::registerHigherProtocol(HigherProtocolInterface *self) {
  m_listeners.append(self);
}

void Router::deregisterHigherProtocol(HigherProtocolInterface *self) {
  m_listeners.removeAll(self);
}

void Router::writePacket(qint8 target, NextHeaderType nextHeader,
                         const QByteArray &payload) {
  IpHeader header;

  header.sourceIp = MY_NODE_IP;
  header.targetIp = target;
  header.nextHeader = nextHeader;

  header.hops = 0;
  header.ttl = 8;

  QByteArray packet;

  {
    QDataStream writer(&packet, QIODevice::WriteOnly);

    writer.setVersion(QDataStream::Qt_4_0);

    writer << header << payload;
  }

  qDebug() << "[ROUTER]   Send datagram to " << QHostAddress(GROUP);

  routePacket(header, packet, true);
}

void Router::routePacket(const IpHeader &header, const QByteArray &datagram,
                         bool generated) {
  Q_UNUSED(header)

  if (!m_seenDatagrams.contains(datagram) || generated) {
    m_seenDatagrams[datagram] = QDateTime::currentDateTimeUtc();

    // broadcast packet
    if (header.targetIp == 0) {
      m_socket->writeDatagram(datagram, QHostAddress(GROUP), 1337);
    } else {

      // routing code
      if (m_nodes.contains(header.targetIp)) {
        RoutingInformation info = m_nodes.value(header.targetIp);

        m_socket->writeDatagram(datagram, info.nextHop, 1337);
      } else {
        m_packets << datagram;

        sendRouteRequest(header.targetIp);
      }
    }
  }
}

Router::Router(QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)), m_cleanTimeout(this) {
  // Listen for messages
  connect(m_socket, SIGNAL(readyRead()), SLOT(fetchPacket()));

  // Bind to any address
  m_socket->bind(QHostAddress::AnyIPv4, PORT, QAbstractSocket::ShareAddress);

  // Join the multicast group
  m_socket->joinMulticastGroup(QHostAddress(GROUP));

  // Do not send to loopback
  m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);

  connect(&m_cleanTimeout, SIGNAL(timeout()), SLOT(cleanSeenPackets()));

  m_cleanTimeout.setInterval(5000);
  m_cleanTimeout.setSingleShot(false);
  m_cleanTimeout.start();
}

Router::~Router() {}

void Router::fetchPacket() {

  // Process all pending messages
  while (m_socket->hasPendingDatagrams()) {

    // Get the pending datagram size
    qint64 datagram_size = m_socket->pendingDatagramSize();

    // Create the buffer
    QByteArray datagram(int(datagram_size), '0');

    // Read the datagram message and store the address and port
    QHostAddress from_address;
    quint16 from_port;
    m_socket->readDatagram(datagram.data(), datagram.size(), &from_address,
                           &from_port);

    m_neighborTimeouts[from_address] = QDateTime::currentDateTimeUtc().addSecs(5*30);

    // Debug output
    qDebug() << "[ROUTER]   Received datagram from" << from_address;

    {
      // Build the reader
      QDataStream reader(datagram);

      // Ensure a stable version
      reader.setVersion(QDataStream::Qt_4_0);

      IpHeader header;
      // Read the message
      reader >> header;

      switch (header.nextHeader) {
      case RouteRequestPacket: {
        RouteRequest routeRequest;

        reader >> routeRequest;

        handleRouteRequest(routeRequest, from_address);
      } break;
      case RouteReplyPacket: {
        RouteReply routeReply;

        reader >> routeReply;

        handleRouteReply(routeReply, from_address);
      } break;
      case RouteErrorPacket: {
        RouteError routeError;

        reader >> routeError;

        handleRouteError(routeError, from_address);
      }
      default:
        if (header.targetIp == 0 || header.targetIp == MY_NODE_IP) {
          QByteArray payload;

          reader >> payload;

          for (HigherProtocolInterface *listener : m_listeners) {
            bool done = listener->handlePacket(header.sourceIp,
                                               header.nextHeader, payload);
            if (done) {
              break;
            }
          }
        }
      }

      if (header.targetIp == 0 || header.targetIp != MY_NODE_IP) {
        routePacket(header, datagram, false);
      }
    }
  } // while hasPendingDatagrams
}

void Router::cleanSeenPackets() {
  QMutableMapIterator<QByteArray, QDateTime> iter(m_seenDatagrams);

  while (iter.hasNext()) {
    iter.next();

    if (iter.value().secsTo(QDateTime::currentDateTimeUtc()) > 120) {
      iter.remove();
    }
  }
}

void Router::scanLostNodes() {
  QMapIterator<QHostAddress, QDateTime> iter(m_neighborTimeouts);

  RouteError routeError;

  while (iter.hasNext()) {
    if (iter.value() < QDateTime::currentDateTimeUtc()) {
      qint8 nodeID = resolveHostAddress(iter.key());

      routeError.unreachables << Node(nodeID, m_nodes[nodeID].destSeqNum);
    }
  }

  if (routeError.unreachables.size() > 0) {
    sendDetectedRouteError(routeError);
  }
}


}
} // namespace Protocol

bool operator<(const QHostAddress &a, const QHostAddress &b)
{
    return a.toIPv4Address() < b.toIPv4Address();
}
