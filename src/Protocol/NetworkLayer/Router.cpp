#include "IpHeader.h"
#include "Router.h"
#include <QDataStream>
#include <QUdpSocket>
#include "config.h"

namespace Protocol {
namespace NetworkLayer {


void Router::registerHigherProtocol(HigherProtocolInterface *self)
{
    m_listeners.append(self);
}

void Router::deregisterHigherProtocol(HigherProtocolInterface *self)
{
    m_listeners.removeAll(self);
}

void Router::writePacket(qint8 target, NextHeaderType nextHeader, const QByteArray &payload)
{
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

    qDebug() << "[ROUTER]   Send datagram to "<< QHostAddress(GROUP);
    m_socket->writeDatagram(packet, QHostAddress(GROUP), 1337);
}

Router::Router(QObject *parent) : QObject(parent) , m_socket(new QUdpSocket(this))
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

      QByteArray payload;

      reader >> payload;

      for(HigherProtocolInterface * listener : m_listeners) {
          bool done = listener->handlePacket(header.sourceIp, header.nextHeader, payload);

          if(done) {
              break;
          }
      }


    }
  } // while hasPendingDatagrams
}


}
} // namespace Protocol

