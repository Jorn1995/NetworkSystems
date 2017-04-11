#ifndef PROTOCOL_NETWORKLAYER_ROUTER_H
#define PROTOCOL_NETWORKLAYER_ROUTER_H

#include <QHostAddress>
#include <QMap>
#include <QObject>
#include <QTimer>
#include "../TransportLayer/Flag.h"

#include "IpHeader.h"

#include "HigherProtocolInterface.h"

class QUdpSocket;

namespace Protocol {
namespace NetworkLayer {

class Router : public QObject
{
    friend class HigherProtocolInterface;

    Q_OBJECT

    struct RoutingInformation {
        QHostAddress nextHop;
        int          cost;
    };

    QUdpSocket *m_socket;

    QMap<qint8, RoutingInformation> m_nodes;

    QList<HigherProtocolInterface*> m_listeners;

protected:
    // Higher Protocol callbacks
    void registerHigherProtocol(HigherProtocolInterface * self);
    void deregisterHigherProtocol(HigherProtocolInterface * self);

    void writePacket(qint8 target, NextHeaderType nextHeader, const QByteArray & payload);

private slots:
    void fetchPacket();

public:
    explicit Router(QObject *parent = 0);
};

}
} // namespace Protocol

#endif // PROTOCOL_NETWORKLAYER_ROUTER_H
