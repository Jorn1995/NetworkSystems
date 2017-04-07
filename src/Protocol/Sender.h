#ifndef SENDER_H
#define SENDER_H

#include <QObject>

class QUdpSocket;

namespace Protocol {

class Sender : public QObject
{
    Q_OBJECT

    QUdpSocket *m_socket;
public:
    explicit Sender(QObject *parent = 0);

signals:

public slots:
    void sendMessage(QString message, QString from);
};

}

#endif // SENDER_H
