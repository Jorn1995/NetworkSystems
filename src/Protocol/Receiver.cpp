#include "Receiver.h"

#include "config.h"

#include <QDataStream>
#include <QUdpSocket>

namespace Protocol {

Receiver::Receiver(QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)) {
  // Listen for messages
  connect(m_socket, SIGNAL(readyRead()), SLOT(fetchPacket()));

  // Bind to any address
  m_socket->bind(QHostAddress::AnyIPv4, PORT, QAbstractSocket::ShareAddress);

  // Join the multicast group
  m_socket->joinMulticastGroup(QHostAddress(GROUP));
}

void Receiver::fetchPacket() {
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
      reader >> sender >> message;

      emit newMessage(message, sender);
    }
  }
}
}
