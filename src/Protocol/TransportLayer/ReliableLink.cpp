#include "ReliableLink.h"

#include <QDataStream>
#include <QDebug>

namespace Protocol {
namespace TransportLayer {

ReliableLink::ReliableLink(qint8 peer, NetworkLayer::Router *router,
                           QObject *parent)
    : QObject(parent), HigherProtocolInterface(router), m_peer(peer),
      m_resendTimeout(this) {
  connect(&m_resendTimeout, SIGNAL(timeout()), SLOT(resendBuffer()));

  if (peer != 0) {
    qDebug() << "[RELIABLE] Link created with peer" << peer;
    // Send out SYN packet
    Header header;

    header.ackNum = m_ackNum;
    header.seqNum = newSeq();

    header.flags = Sync;

    // Await connection
    m_state = Connecting;

    sendPacket(header);
  }
}

void ReliableLink::handleAcknowledgement(qint32 ackNum) {
  QMutableMapIterator<qint32, QByteArray> it(m_sendBuffer);

  while (it.hasNext()) {
    it.next();

    if (it.key() < ackNum) {
      it.remove();
    }
  }
}

void ReliableLink::handleReceivedPackets() {
  bool needAck = false;
  while (m_receiveBuffer.contains(m_ackNum)) {
    {
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

      if (!payload.isEmpty())
        needAck = true;

      readPayload(payload);
    }

    m_ackNum++;
  }

  if (needAck) {
    // Create new header
    Header writeHeader;

    // Set the sequence number
    writeHeader.seqNum = newSeq();

    // Write the ack number
    writeHeader.ackNum = m_ackNum - 1; // Compensate for while loop
    writeHeader.flags = Acknowledgement;

    // Send the packet with empty message
    sendPacket(writeHeader);
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
    // Connection request received or connection acknowledgement received
    if (header.flags.testFlag(Sync)) {

      if (header.flags.testFlag(Acknowledgement)) {
        // Connection acknowledgement received, ignore as we are listening for a
        // new connection
        return false;
      } else {
        Header replyHeader;
        // Connection request received, send a syn reply
        qDebug() << "[RELIABLE] Received (SYN) - Seq:" << header.seqNum << "Ack:" << header.ackNum
                 << "Flags:" << header.flags;

        // Assign to the new peer
        m_peer = target;

        // Set the ack counter to the peer seq number
        m_ackNum = replyHeader.seqNum;

        // Write the ack and our seq counter
        replyHeader.ackNum = m_ackNum;
        replyHeader.seqNum = newSeq();

        // This is an Syn+Ack packet
        replyHeader.flags = Sync | Acknowledgement;

        // And send it
        sendPacket(replyHeader);

        // Notify interested objects that we accepted a peer and are no longer
        // listening
        emit peerAccepted(m_peer);

        // We are now in connected state
        m_state = Connected;

        return true;
      } // end !Acknowledgement
    }   // end Sync
  } else if (m_peer != target) {
    // We're not listening and this is not who we are talking to, ignore
    return false;
  } else if (m_state == Connecting && header.flags.testFlag(Sync) &&
             header.flags.testFlag(Acknowledgement)) {
      qDebug() << "[RELIABLE] Received (SYN|ACK) - Seq:" << header.seqNum << "Ack:" << header.ackNum
               << "Flags:" << header.flags;

    // Store the header
    Header replyHeader;

    // Initialize the ack number to their seq number
    m_ackNum = header.seqNum;

    // Write the ack and our seq number
    replyHeader.ackNum = m_ackNum;
    replyHeader.seqNum = newSeq();

    // This is an acknowledgement
    replyHeader.flags = Acknowledgement;

    // Send packet
    sendPacket(replyHeader);

    // We are now connected
    m_state = Connected;

    // Process any packets in our early send buffer
    processBuffer();

    return true;
  }

  if (header.flags.testFlag(Acknowledgement)) {
    handleAcknowledgement(header.ackNum);
  }

  m_receiveBuffer.insert(header.seqNum, data);

  handleReceivedPackets();

  return true;
}

void ReliableLink::resendBuffer() {
  for (auto I = m_sendBuffer.cbegin(); I != m_sendBuffer.cend(); I++) {
      qDebug() << "[RELIABLE] Resend - Seq:" << I.key();
    NetworkLayer::HigherProtocolInterface::sendPacket(m_peer, NetworkLayer::ReliableLink, I.value());
  }
}

void ReliableLink::processBuffer() {
  while (m_databuffer.size()) {
    QByteArray payload = m_databuffer.dequeue();

    Header header;

    header.seqNum = newSeq();
    sendPacket(header, payload);
  }
}

void ReliableLink::sendPacket(ReliableLink::Header header, QByteArray payload)
{
    QByteArray buffer;

    {
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    writer << header;
    writer.writeRawData(payload.data(), payload.size());
    }

    m_sendBuffer.insert(header.seqNum, buffer);

    qDebug() << "[RELIABLE] Send - Seq:" << header.seqNum
             << "Ack:" << header.ackNum
             << "Flags:" << header.flags;

    NetworkLayer::HigherProtocolInterface::sendPacket(m_peer, NetworkLayer::ReliableLink, buffer);
    m_resendTimeout.setInterval(5000);
}

void ReliableLink::sendPacket(ReliableLink::Header header)
{
    QByteArray buffer;

    {
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    writer << header;
    }

    qDebug() << "[RELIABLE] Send - Seq:" << header.seqNum
             << "Ack:" << header.ackNum
             << "Flags:" << header.flags;

    NetworkLayer::HigherProtocolInterface::sendPacket(m_peer, NetworkLayer::ReliableLink, buffer);
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
