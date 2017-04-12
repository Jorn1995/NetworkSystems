#include "ReliableLink.h"

#include <QDataStream>
#include <QDebug>

#include "../NetworkLayer/Router.h"

namespace Protocol {
namespace TransportLayer {

void ReliableLink::sendSyn() {
  Header header;

  header.ackNum = lastAck();
  header.seqNum = lastSeq();

  header.flags = Sync;

  // Await connection
  m_state = SynSent;

  sendPacket(header);

  QTimer::singleShot(5000, this, SLOT(handleConnectionTimeout()));
}

void ReliableLink::sendSynAck() {
  Header header;

  header.ackNum = lastAck();
  header.seqNum = lastSeq();

  // This is an Syn+Ack packet
  header.flags = Sync | Acknowledgement;

  // And send it
  sendPacket(header);

  QTimer::singleShot(5000, this, SLOT(handleConnectionTimeout()));
}

ReliableLink::ReliableLink(qint8 peer, NetworkLayer::Router *router)
    : QObject(router), HigherProtocolInterface(router), m_resendTimeout(this),
      m_ackSendTimeout(this), m_peer(peer) {
  connect(&m_resendTimeout, SIGNAL(timeout()), SLOT(resendBuffer()));
  connect(&m_ackSendTimeout, SIGNAL(timeout()), SLOT(ackSendTimeout()));

  m_ackSendTimeout.setInterval(1000);

  if (peer != 0) {
    qDebug() << "[RELIABLE] Link created with peer" << peer;

    newSeq();

    // Send out SYN packet
    sendSyn();
  }
}

ReliableLink::ReliableLink(NetworkLayer::Router *router)
    : ReliableLink(0, router) {}

void ReliableLink::handleAcknowledgement(qint32 ackNum) {
  QMutableMapIterator<qint32, QByteArray> it(m_sendBuffer);

  if (m_state == SynReceived) {
    m_state = Connected;
  }

  while (it.hasNext()) {
    it.next();

    if (it.key() < ackNum) {
      it.remove();
    }
  }
}

void ReliableLink::handleReceivedPackets() {
  // Iterator to loop over the buffer
  QMutableMapIterator<qint32, QByteArray> iter(m_receiveBuffer);

  // Remove old already received packets (duplicates)
  while (iter.hasNext()) {
    iter.next();
    if (iter.key() < m_ackNum) {
      // Found a duplicate, resend the ack we are at
      m_needAck = true;
      iter.remove();
    } else {
      break;
    }
  }

  // Find the next ack
  while (m_receiveBuffer.contains(m_ackNum)) {
    QByteArray data = m_receiveBuffer.take(m_ackNum);

    QDataStream reader(data);

    Header readHeader;

    reader >> readHeader;

    // Make it certainly big enough
    QByteArray payload(data.size(), 0);

    // Read the remaining data
    int read = reader.readRawData(payload.data(), payload.size());

    // and trim the excess
    payload.resize(read);

    if (!payload.isEmpty()) {
      m_needAck = true;

      readPayload(payload);
    }

    newAck();
  }

  // There are gaps, resend our last ack
  if(m_receiveBuffer.size()) {
      m_needAck = true;
  }

  // Set the timer to send the ack
  if (m_needAck) {
    m_ackSendTimeout.start();
  }
}

bool ReliableLink::handlePacket(qint8 target, qint8 nextHeader,
                                const QByteArray &data) {
  if (nextHeader != NetworkLayer::ReliableLink) {
    return false;
  }

  QDataStream reader(data);

  Header header;

  reader >> header;

  // Currently listening
  if (m_peer == 0) {
    if (m_state != SynReceived) {
      // Connection request received or connection acknowledgement received
      if (header.flags.testFlag(Sync)) {
        if (header.flags.testFlag(Acknowledgement)) {
          // Connection acknowledgement received, ignore as we are listening for
          // a
          // new connection
          return false;
        } else {
          // Connection request received, send a syn reply
          qDebug() << "[RELIABLE] Received (SYN) - Seq:" << header.seqNum
                   << "Ack:" << header.ackNum << "Flags:" << header.flags;

          // Assign to the new peer
          m_peer = target;

          // Set the ack counter to the peer seq number
          m_ackNum = header.seqNum;

          newAck();
          newSeq();

          // Write the ack and our seq counter
          sendSynAck();

          // Notify interested objects that we accepted a peer and are no longer
          // listening
          emit peerAccepted(m_peer);

          // We are now in connected state
          m_state = SynReceived;

          return true;
        }
      }
    }
  } else if (m_peer != target) {
    return false;
  } else if (m_state == SynSent && header.flags.testFlag(Sync) &&
             header.flags.testFlag(Acknowledgement)) {
    qDebug() << "[RELIABLE] Received (SYN|ACK) - Seq:" << header.seqNum
             << "Ack:" << header.ackNum
             << "Flags:" << TransportLayer::flagDebug(header.flags);

    // We are now connected
    m_state = Connected;

    m_ackNum = header.seqNum;

    newAck();

    // Process any packets in our early send buffer
    processBuffer();

    return true;
  } else {
    qDebug() << "[RELIABLE] Received - Seq:" << header.seqNum
             << "Ack:" << header.ackNum
             << "Flags:" << TransportLayer::flagDebug(header.flags);

    if (header.flags.testFlag(Acknowledgement)) {
      handleAcknowledgement(header.ackNum);
    }

    m_receiveBuffer.insert(header.seqNum, data);

    handleReceivedPackets();

    return true;
  }
  return false;
}

void ReliableLink::resendBuffer() {
  for (auto I = m_sendBuffer.cbegin(); I != m_sendBuffer.cend(); I++) {
    qDebug() << "[RELIABLE] Resend - Seq:" << I.key();
    NetworkLayer::HigherProtocolInterface::sendPacket(
        m_peer, NetworkLayer::ReliableLink, I.value());
  }
}

void ReliableLink::handleConnectionTimeout() {
  switch (m_state) {
  case SynSent:
    sendSyn();
    break;
  case SynReceived:
    sendSynAck();
    break;
  default:
    return;
  }
}

void ReliableLink::ackSendTimeout() {
  if (m_needAck) {
    // Create new header
    Header writeHeader;

    // Set the sequence number
    writeHeader.seqNum = newSeq();

    // Write the ack number
    writeHeader.ackNum = lastAck();
    writeHeader.flags = Acknowledgement;

    // Send the packet with empty message
    sendPacket(writeHeader);

    m_needAck = false;
  }
}

void ReliableLink::processBuffer() {
  while (m_databuffer.size()) {
    QByteArray payload = m_databuffer.dequeue();

    Header header;

    header.ackNum = lastAck();
    header.flags = Acknowledgement;
    header.seqNum = newSeq();

    m_needAck = false;

    sendPacket(header, payload);
  }
}

void ReliableLink::sendPacket(ReliableLink::Header header, QByteArray payload) {
  QByteArray buffer;

  {
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    writer << header;
    writer.writeRawData(payload.data(), payload.size());
  }

  m_sendBuffer.insert(header.seqNum, buffer);

  qDebug() << "[RELIABLE] Send - Seq:" << header.seqNum
           << "Ack:" << header.ackNum
           << "Flags:" << TransportLayer::flagDebug(header.flags)
           << "Data:" << true;

  NetworkLayer::HigherProtocolInterface::sendPacket(
      m_peer, NetworkLayer::ReliableLink, buffer);
  m_resendTimeout.start(5000);
}

void ReliableLink::sendPacket(ReliableLink::Header header) {
  QByteArray buffer;

  {
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    writer << header;
  }

  qDebug() << "[RELIABLE] Send - Seq:" << header.seqNum
           << "Ack:" << header.ackNum
           << "Flags:" << TransportLayer::flagDebug(header.flags)
           << "Data:" << false;

  NetworkLayer::HigherProtocolInterface::sendPacket(
      m_peer, NetworkLayer::ReliableLink, buffer);
}

void ReliableLink::readPayload(const QByteArray &payload) {
  Q_UNUSED(payload)
  // No Op
}

void ReliableLink::writePayload(const QByteArray &payload) {
  m_databuffer << payload;

  if (m_state == Connected) {
    processBuffer();
  }
}

QDataStream &operator<<(QDataStream &stream,
                        const ReliableLink::Header &object) {
  return stream << object.ackNum << object.seqNum << qint8(object.flags);
}

QDataStream &operator>>(QDataStream &stream, ReliableLink::Header &object) {
  qint8 flags;
  stream >> object.ackNum >> object.seqNum >> flags;
  object.flags = Flags(flags);

  return stream;
}

} // namespace TransportLayer
} // namespace Protocol
