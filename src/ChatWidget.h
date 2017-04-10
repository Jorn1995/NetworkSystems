#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>

namespace Ui {
class ChatWidget;
}

namespace Protocol {
class Duplex;
}

class ChatWidget : public QWidget {
  Q_OBJECT

public:
  explicit ChatWidget(QWidget *parent = 0);
  ~ChatWidget();

  // GUI slots
private slots:
  void sendMessage();

  // Protocol slots
private slots:
  void receivedMessage(const QString &message, const QString &sender);

private:
  Ui::ChatWidget *ui;

  Protocol::Duplex *m_duplex;
};

#endif // CHATWIDGET_H
