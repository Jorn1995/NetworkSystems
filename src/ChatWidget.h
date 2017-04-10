#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QMap>
#include <QWidget>

namespace Ui {
class ChatWidget;
}

namespace Protocol {
namespace NetworkLayer {
class Router;
}
namespace TransportLayer {
class ReliableLink;
}
}

class ChatWidget : public QWidget {
  Q_OBJECT

    QMap<qint8,Protocol::TransportLayer::ReliableLink*> m_connections;
    Protocol::TransportLayer::ReliableLink *m_listener;

public:
  explicit ChatWidget(QWidget *parent = 0);
  ~ChatWidget();

  // GUI slots
private slots:
  void sendMessage();

  // Protocol slots
private slots:
  void receivedMessage(const QString &message, const QString &sender);
  void peerAccepted(qint8 peer);

private:
  Ui::ChatWidget *ui;

  Protocol::NetworkLayer::Router *m_router;
};

#endif // CHATWIDGET_H
