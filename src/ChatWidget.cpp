#include "ChatWidget.h"
#include "ui_chatwidget.h"

#include <Protocol/NetworkLayer/Router.h>

#include <config.h>

#include <Protocol/TransportLayer/ReliableLink.h>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWidget),
      m_router(new Protocol::NetworkLayer::Router(this)) {

  ui->setupUi(this);
  ui->splitter->setSizes(QList<int>() << 1 << 2);
  m_listener = new Protocol::TransportLayer::ReliableLink(0, m_router, this);
  connect(m_listener, SIGNAL(peerAccepted(qint8)), this,
          SLOT(peerAccepted(qint8)));
  connect(m_listener, SIGNAL(newMessage(QString, QString)),
          SLOT(receivedMessage(QString, QString)));

}

ChatWidget::~ChatWidget() { delete ui; }

void ChatWidget::receivedMessage(const QString &message,
                                 const QString &sender) {
  ui->log->appendPlainText(QString("%1 said : %2").arg(sender, message));
}

void ChatWidget::peerAccepted(qint8 peer) {
  m_connections[peer] = m_listener;
  disconnect(m_listener, SIGNAL(peerAccepted(qint8)), this,
             SLOT(peerAccepted(qint8)));

  m_listener = new Protocol::TransportLayer::ReliableLink(0, m_router, this);
  connect(m_listener, SIGNAL(peerAccepted(qint8)), SLOT(peerAccepted(qint8)));
}

void ChatWidget::sendMessage() {
  QString message = ui->msg->text();
  if (!message.isEmpty()) {
    qint8 target = qint8(ui->target->value());

    Protocol::TransportLayer::ReliableLink *link = nullptr;
    if (m_connections.contains(target)) {
      link = m_connections.value(target);
    } else {
      link = m_connections[target] =
          new Protocol::TransportLayer::ReliableLink(target, m_router, this);
      connect(link, SIGNAL(newMessage(QString, QString)),
              SLOT(receivedMessage(QString, QString)));
    }

    link->sendMessage(message, MY_IP);
    ui->msg->clear();
  }
}
