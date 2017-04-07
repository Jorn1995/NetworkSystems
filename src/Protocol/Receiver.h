#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

class QUdpSocket;

namespace Protocol {

class Receiver : public QObject
{
    Q_OBJECT

    QUdpSocket *m_socket;
public:
    explicit Receiver(QObject *parent = 0);

signals:
    void newMessage(QString message, QString from);

private slots:
    void fetchPacket();
};

}

#endif // RECEIVER_H
