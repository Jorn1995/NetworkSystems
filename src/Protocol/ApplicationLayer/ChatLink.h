#ifndef PROTOCOL_APPLICATIONLAYER_CHATLINK_H
#define PROTOCOL_APPLICATIONLAYER_CHATLINK_H

#include "../TransportLayer/ReliableLink.h"

namespace Protocol {
namespace ApplicationLayer {

class ChatLink : public TransportLayer::ReliableLink
{
    Q_OBJECT

protected:
    void readPayload(const QByteArray & payload);

public:
    ChatLink(qint8 target, NetworkLayer::Router * router, QObject * parent);
    ~ChatLink();

signals:
    void newMessage(QString message, QString from);

public slots:
  void sendMessage(QString message, QString from);

};

} // namespace ApplicationLayer
} // namespace Protocol

#endif // PROTOCOL_APPLICATIONLAYER_CHATLINK_H
