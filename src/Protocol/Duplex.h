#ifndef PROTOCOL_DUPLEX_H
#define PROTOCOL_DUPLEX_H

#include <QMap>
#include <QObject>
#include <QTimer>
#include "Flag.h"
#include "Header.h"

class QUdpSocket;

namespace Protocol {

class Duplex : public QObject
{
    Q_OBJECT

    QUdpSocket *m_socket;

    QMap<qint32, QByteArray> m_sendBuffer;
    QMap<qint32, QByteArray> m_receiveBuffer;

    void handleAcknowledgement(qint32 ackNum);
    void handleReceivedPackets();

    QTimer m_resendTimeout;
public:
    explicit Duplex(QObject *parent = 0);

    qint32 m_seqNum = 0, m_ackNum = 0;
//    Header m_header;

signals:
    void newMessage(QString message, QString from);

private slots:
    void fetchPacket();
    void resendBuffer();

public slots:
    void sendMessage(QString message, QString from);
};

} // namespace Protocol

#endif // PROTOCOL_DUPLEX_H
