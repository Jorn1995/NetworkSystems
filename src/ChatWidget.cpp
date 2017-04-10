#include "ChatWidget.h"
#include "ui_chatwidget.h"

#include <Protocol/Duplex.h>

#include <config.h>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWidget),
      m_duplex(new Protocol::Duplex(this)) {
  connect(m_duplex, SIGNAL(newMessage(QString, QString)),
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
  if(!message.isEmpty()) {
      m_duplex->sendMessage(message, MY_IP);
      ui->msg->clear();
  }
}
