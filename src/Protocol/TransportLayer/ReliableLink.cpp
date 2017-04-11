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
    qDebug() << "Link created with peer" << peer;
    // Send out SYN packet
    Header header;

    header.ackNum = m_ackNum;
    header.seqNum = m_seqNum;

    header.flags = Sync;

    QByteArray synRequest;

    // Write syn packet
    {
      QDataStream writer(&synRequest, QIODevice::WriteOnly);

      writer << header << QString() << QString();
    }

    // Await connection
    m_state = Connecting;

    sendPacket(m_peer, synRequest);
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

      QString message, from;
      reader >> readHeader >> from >> message;

      if (!message.isEmpty())
        needAck = true;

      emit newMessage(message, from);
    }

    m_ackNum++;
  }

  if (needAck) {
    QByteArray sendBuffer;

    QDataStream writer(&sendBuffer, QIODevice::WriteOnly);

    // Create new header
    Header writeHeader;

    // Set the sequence number
    writeHeader.seqNum = m_seqNum;
    m_seqNum++;

    // Write the ack number
    writeHeader.ackNum = m_ackNum;
    writeHeader.flags = Acknowledgement;

    // Send the packet with empty message
    writer << writeHeader << QString() << QString();
  }
}

bool ReliableLink::handlePacket(qint8 target, const QByteArray &data) {
  QDataStream reader(data);

  Header header;

  reader >> header;

  if (m_peer == 0) {
    if (header.flags.testFlag(Sync)) {
      qDebug() << "Sync received";

      m_peer = target;

      m_ackNum = header.seqNum;
      header.ackNum = m_ackNum;
      header.seqNum = m_seqNum;
      m_seqNum++;

      header.flags |= Acknowledgement;
    }

    QByteArray synReply;

    {
      QDataStream writer(&synReply, QIODevice::WriteOnly);

      writer << header << QString() << QString();

      qDebug() << "Writing reply: Seq:" << header.seqNum
               << "Ack:" << header.ackNum;
    }

    sendPacket(m_peer, synReply);

    emit peerAccepted(m_peer);

    m_state = Connected;

    return true;
  } else if (m_peer != target) {
    return false;
  } else if (m_state == Connecting && header.flags.testFlag(Sync) &&
             header.flags.testFlag(Acknowledgement)) {
    qDebug() << "Seq acknowledged";

    m_ackNum = header.seqNum;
    header.ackNum = m_ackNum;
    header.seqNum = m_seqNum;
    m_seqNum++;

    QByteArray ackReply;

    {
      QDataStream writer(&ackReply, QIODevice::WriteOnly);

      writer << header << QString() << QString();
    }

    sendPacket(m_peer, ackReply);

    m_state = Connected;

    return true;
  }

  QString message, from;

  reader >> from >> message;

  qDebug() << "Seq:" << header.seqNum << "Ack:" << header.ackNum
           << "Flags:" << header.flags;

  if (header.flags.testFlag(Acknowledgement)) {
    handleAcknowledgement(header.ackNum);
  }

  m_receiveBuffer.insert(header.seqNum, data);

  handleReceivedPackets();

  return true;
}

void ReliableLink::resendBuffer() {
  for (auto I = m_receiveBuffer.cbegin(); I != m_receiveBuffer.cend(); I++) {
    sendPacket(m_peer, I.value());
  }
}

void ReliableLink::sendMessage(QString message, QString from) {
  // Create buffer for the message
  QByteArray buffer;
  Header header;

  {
    // Create a binary writer on the buffer
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    // Ensure a stable version
    writer.setVersion(QDataStream::Qt_4_0);

    // Update header
    header.seqNum = m_seqNum;
    m_seqNum++;

    // Possibly send acknowledgement ?
    header.ackNum = m_ackNum;
    header.flags = Acknowledgement;

    // Write our the typed message
    writer << header << from << message;
  }

  m_sendBuffer.insert(header.seqNum, buffer);
  // Send the message to the group on the configured port
  sendPacket(m_peer, buffer);

  m_resendTimeout.setInterval(5000);

  emit newMessage(message, from);
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