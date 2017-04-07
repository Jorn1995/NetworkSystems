#include "ChatWidget.h"
#include "ui_chatwidget.h"

#include <Protocol/Receiver.h>
#include <Protocol/Sender.h>

#include <config.h>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWidget),
      m_sender(new Protocol::Sender(this)),
      m_receiver(new Protocol::Receiver(this)) {
  connect(m_receiver, SIGNAL(newMessage(QString, QString)),
          SLOT(receivedMessage(QString, QString)));

      ui->setupUi(this);
}

ChatWidget::~ChatWidget() { delete ui; }

void ChatWidget::receivedMessage(const QString &message,
                                 const QString &sender) {
  ui->log->appendPlainText(QString("%1 said : %2").arg(sender, message));
}

void ChatWidget::sendMessage() {
  QString message = ui->msg->text();
  m_sender->sendMessage(message, MY_IP);
  ui->msg->clear();
}
