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

  // Sequence number tracking
  qint32 m_seqNum = 0, m_ackNum = 0;

  // Timers
  QTimer m_resendTimeout;

protected:
  // Link tracking
  qint8 m_peer;
  enum State {
    Disconnected = 0,
    Listening = 0,
    SynReceived,
    SynSent,
    Connecting,
    Connected,
  } m_state;

public:
  struct Header {
    qint32 seqNum = 0;
    qint32 ackNum = 0;
    Flags flags;
  };

private:
  // Send packet convenience functions
  void sendPacket(Header header, QByteArray payload);
  void sendPacket(Header header);

  // Handle acks and received packets
  void handleAcknowledgement(qint32 ackNum);
  void handleReceivedPackets();

  // Process the send buffer
  void processBuffer();

  // Convenience next seq/ack numbers
  inline qint32 newSeq() { return m_seqNum++; }
  inline qint32 lastSeq() { return m_seqNum - 1; }
  inline qint32 newAck() { return m_ackNum++; }
  inline qint32 lastAck() { return m_ackNum - 1; }

  void sendSyn();
  void sendSynAck();

protected:
  // Higher protocol interface
  virtual void readPayload(const QByteArray &payload);
  void writePayload(const QByteArray &payload);

public:
  // Handle incomming packet
  bool handlePacket(qint8 target, qint8 nextHeader, const QByteArray &data);

public:
  /**
   * @brief ReliableLink
   * @param peer, peer to connect to, 0 = listen for others
   * @param router
   * @param parent
   */
  explicit ReliableLink(qint8 peer, NetworkLayer::Router *router);

  explicit ReliableLink(NetworkLayer::Router *router);

signals:
  void peerAccepted(qint8 address);

private slots:
  void resendBuffer();
  void handleConnectionTimeout();
};

QDataStream &operator<<(QDataStream &stream,
                        const ReliableLink::Header &object);
QDataStream &operator>>(QDataStream &stream, ReliableLink::Header &object);

} // namespace TransportLayer
} // namespace Protocol

#endif // PROTOCOL_TRANSPORTLAYER_RELIABLELINK_H
