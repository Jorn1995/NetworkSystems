#include "Duplex.h"
#include <QDataStream>
#include <QUdpSocket>
#include "config.h"
#include "Flag.h"

namespace Protocol {

void Duplex::handleAcknowledgement(qint32 ackNum)
{
    for(;m_ackNum < ackNum; m_ackNum ++) {
        m_sendBuffer.remove(m_ackNum);
    }
}

void Duplex::handleReceivedPackets()
{
    while(m_receiveBuffer.contains(m_seqNum)) {
        {
            QByteArray data = m_receiveBuffer.take(m_seqNum);

            QDataStream reader(data);

            Header header;

            QString message, from;
            reader >> header >> from >> message;

            emit newMessage(message, from);
        }

        m_seqNum ++;
    }
}

Duplex::Duplex(QObject *parent) : QObject(parent) , m_socket(new QUdpSocket(this)), m_resendTimeout(this)
{
    // Listen for messages
    connect(m_socket, SIGNAL(readyRead()), SLOT(fetchPacket()));

    // Bind to any address
    m_socket->bind(QHostAddress::AnyIPv4, PORT, QAbstractSocket::ShareAddress);

    // Join the multicast group
    m_socket->joinMulticastGroup(QHostAddress(GROUP));

    // Do not send to loopback
    m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);

    connect(&m_resendTimeout, SIGNAL(timeout()),SLOT(resendBuffer()));
}

void Duplex::fetchPacket() {

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

    // Debug output
    qDebug() << "Received datagram from" << from_address << "on" << from_port;
    qDebug() << datagram;

    {
      // Build the reader
      QDataStream reader(datagram);

      // Ensure a stable version
      reader.setVersion(QDataStream::Qt_4_0);

      Header header;
      // Read the message
      reader >> header;

      if(header.flags.testFlag(Acknowledgement)) {
          handleAcknowledgement(header.ackNum);
      }

      m_receiveBuffer.insert(header.seqNum, datagram);
    }
  } // while hasPendingDatagrams

  handleReceivedPackets();
}

void Duplex::resendBuffer()
{
    for(auto I = m_receiveBuffer.cbegin(); I != m_receiveBuffer.cend(); I++) {
        m_socket->writeDatagram(I.value(), QHostAddress(GROUP), PORT);
    }
}

void Duplex::sendMessage(QString message, QString from) {
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
    m_seqNum ++;

    // Possibly send acknowledgement ?
    header.ackNum = m_ackNum;
    header.flags = Acknowledgement;

    // Write our the typed message
    writer << header << from << message;
  }

  m_sendBuffer.insert(header.seqNum, buffer);
  // Send the message to the group on the configured port
  m_socket->writeDatagram(buffer, QHostAddress(GROUP), PORT);

  m_resendTimeout.setInterval(5000);

  emit newMessage(message, from);
}

} // namespace Protocol

