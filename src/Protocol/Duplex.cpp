#include "Duplex.h"
#include <QDataStream>
#include <QUdpSocket>
#include "config.h"
#include "Flag.h"

namespace Protocol {

Duplex::Duplex(QObject *parent) : QObject(parent) , m_socket(new QUdpSocket(this))
{
    // Listen for messages
    connect(m_socket, SIGNAL(readyRead()), SLOT(fetchPacket()));

    // Bind to any address
    m_socket->bind(QHostAddress::AnyIPv4, PORT, QAbstractSocket::ShareAddress);

    // Join the multicast group
    m_socket->joinMulticastGroup(QHostAddress(GROUP));

    // Do not send to loopback
    m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);

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

      // Buffers for the sender and message
      QString sender, message;

      // Read the message
      reader >> m_ackNum >> m_flag >> sender >> message;

      if((m_flag & ACK) != 0) {
        m_lar = m_ackNum;
      }

      emit newMessage(message, sender);
    }
  }
}

void Duplex::sendMessage(QString message, QString from) {
  // Create buffer for the message
  QByteArray buffer;

  {
    // Create a binary writer on the buffer
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    // Ensure a stable version
    writer.setVersion(QDataStream::Qt_4_0);

    // Write our the typed message
    writer << from << message;
  }

  // Send the message to the group on the configured port
  m_socket->writeDatagram(buffer, QHostAddress(GROUP), PORT);

  emit newMessage(message, from);
}

} // namespace Protocol

