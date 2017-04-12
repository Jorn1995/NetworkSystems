#include "ChatLink.h"

namespace Protocol {
namespace ApplicationLayer {

void ChatLink::readPayload(const QByteArray &payload) {
  QString message, from;

  QDataStream reader(payload);

  reader >> from >> message;

  emit newMessage(message, from);
}

ChatLink::ChatLink(qint8 target, NetworkLayer::Router *router)
    : TransportLayer::ReliableLink(target, router) {}

ChatLink::~ChatLink() {}

void ChatLink::sendMessage(QString message, QString from) {

  // Create buffer for the message
  QByteArray buffer;

  {
    // Create a binary writer on the buffer
    QDataStream writer(&buffer, QIODevice::WriteOnly);

    // Ensure a stable version
    writer.setVersion(QDataStream::Qt_4_0);

    // Write our typed message
    writer << from << message;
  }

  writePayload(buffer);
  emit newMessage(message, from);
}

} // namespace ApplicationLayer
} // namespace Protocol
