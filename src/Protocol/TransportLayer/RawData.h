#ifndef PROTOCOL_TRANSPORTLAYER_RAWDATA_H
#define PROTOCOL_TRANSPORTLAYER_RAWDATA_H

#include <QObject>

#include "../NetworkLayer/HigherProtocolInterface.h"

namespace Protocol {
namespace NetworkLayer {
class Router;
}

namespace TransportLayer {

class RawData : public QObject, public NetworkLayer::HigherProtocolInterface {
  Q_OBJECT

protected:
    qint8 m_bind;

    enum State {
        Unbound = 0,
        Bound = 1,
    } m_state;

public:
    // Handle incomming packets from the router
    bool handlePacket(qint8 target, qint8 nextHeader, const QByteArray & data);

public:
    // Create new RawData socket
  explicit RawData(NetworkLayer::Router *router);

    // Bind this socket to an address (for listening)
    void bind(qint8 address);
    // Unbind this socket
    void unbind();

protected:
    // Higher level callbacks
    // write a payload out
    void writePayload(qint8 target, const QByteArray &payload);
    // decode a payload
    virtual void readPayload(const QByteArray &payload);
};

} // namespace TransportLayer
} // namespace Protocol

#endif // PROTOCOL_TRANSPORTLAYER_RAWDATA_H
