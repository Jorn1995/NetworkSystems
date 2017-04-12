#include "RawData.h"

#include "../NetworkLayer/Router.h"

namespace Protocol {
namespace TransportLayer {

bool RawData::handlePacket(qint8 target, qint8 nextHeader,
                           const QByteArray &data) {
  if (nextHeader != NetworkLayer::DataPacket) {
    return false;
  }

  if (m_state == Unbound) {
    return false;
  }

  if (m_bind == 0 || m_bind == target) {
    readPayload(data);
    return true;
  }

  return false;
}

RawData::RawData(NetworkLayer::Router *router)
    : QObject(router), NetworkLayer::HigherProtocolInterface(router), m_bind(0),
      m_state(Unbound) {}

void RawData::bind(qint8 address) {
  m_bind = address;
  m_state = Bound;
}

void RawData::unbind() { m_state = Unbound; }

void RawData::writePayload(qint8 target, const QByteArray &payload) {
  sendPacket(target, Protocol::NetworkLayer::DataPacket, payload);
}

void RawData::readPayload(const QByteArray &payload) { Q_UNUSED(payload) }

} // namespace TransportLayer
} // namespace Protocol
