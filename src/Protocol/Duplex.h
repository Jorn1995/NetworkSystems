#ifndef PROTOCOL_DUPLEX_H
#define PROTOCOL_DUPLEX_H

#include <QObject>

class QUdpSocket;

namespace Protocol {

class Duplex : public QObject
{
    Q_OBJECT

    QUdpSocket *m_socket;
public:
    explicit Duplex(QObject *parent = 0);

    // Initialize Last Frame Sent and Last Ack Received
    qint32 m_lfs = 0;
    qint32 m_lar = 0;
    qint8 m_flag = 0;
    qint32 m_ackNum = 0;
    qint32 m_seqNum = 0;

signals:
    void newMessage(QString message, QString from);

private slots:
    void fetchPacket();

public slots:
    void sendMessage(QString message, QString from);
};

} // namespace Protocol

#endif // PROTOCOL_DUPLEX_H
