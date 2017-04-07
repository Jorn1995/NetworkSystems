#include "Sender.h"

#include "config.h"

#include <QDataStream>
#include <QUdpSocket>

namespace Protocol {

Sender::Sender(QObject *parent)
    : QObject(parent), m_socket(new QUdpSocket(this)) {

  // Do not send to loopback
  m_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, false);
}

void Sender::sendMessage(QString message, QString from) {
  // Create buffer for the message
  QByteArray buffer;

  {
    // Create a binary writer on the buffer
    QDataStream writer(&buffer, QIODevice::WriteOnly);
    // Ensure a stable version
    writer.setVersion(QDataStream::Qt_4_0);

    // Write our message
    writer << from << message;
  }

  // Send the message to the group on the configured port
  m_socket->writeDatagram(buffer, QHostAddress(GROUP), PORT);
}
}
