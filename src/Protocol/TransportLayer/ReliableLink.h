#ifndef PROTOCOL_TRANSPORTLAYER_RELIABLELINK_H
#define PROTOCOL_TRANSPORTLAYER_RELIABLELINK_H

#include <QMap>
#include <QObject>
#include <QTimer>
#include <QQueue>

#include "Flag.h"

#include "../NetworkLayer/HigherProtocolInterface.h"

namespace Protocol {
namespace NetworkLayer {
class Router;
}

namespace TransportLayer {

class ReliableLink : public QObject,
                     public NetworkLayer::HigherProtocolInterface {
  Q_OBJECT

  // Transmission buffers
  QMap<qint32, QByteArray> m_sendBuffer;    // To retransmit possibly
  QMap<qint32, QByteArray> m_receiveBuffer; // To sort

  QQueue<QByteArray> m_databuffer;

  // Link tracking
  qint8 m_peer;
  enum State {
    Disconnected = 0,
    Connecting,
    Connected,
  } m_state;

  // Sequence number tracking
  qint32 m_seqNum = 0, m_ackNum = 0;

  // Timers
  QTimer m_resendTimeout;

  void handleAcknowledgement(qint32 ackNum);
  void handleReceivedPackets();

  void processBuffer();

  qint32 newSeq() { return m_seqNum++; }
  qint32 newAck() { return m_ackNum++; }

public:
  struct Header {
    qint32 seqNum;
    qint32 ackNum;
    Flags flags;
  };

private:
  void sendPacket(Header header, QByteArray payload);
  void sendPacket(Header header);

protected:
  virtual void readPayload(const QByteArray & payload);
  void writePayload(const QByteArray & payload);

public:
  bool handlePacket(qint8 target, qint8 nextHeader, const QByteArray &data);

public:
  explicit ReliableLink(qint8 peer, NetworkLayer::Router *router,
                        QObject *parent = 0);

signals:
  void peerAccepted(qint8 address);

private slots:
  void resendBuffer();
};

QDataStream &operator<<(QDataStream &stream,
                        const ReliableLink::Header &object);
QDataStream &operator>>(QDataStream &stream, ReliableLink::Header &object);

} // namespace TransportLayer
} // namespace Protocol

#endif // PROTOCOL_TRANSPORTLAYER_RELIABLELINK_H
