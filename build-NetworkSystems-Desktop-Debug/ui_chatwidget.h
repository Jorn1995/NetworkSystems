/********************************************************************************
** Form generated from reading UI file 'chatwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATWIDGET_H
#define UI_CHATWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatWidget
{
public:
    QGridLayout *gridLayout;
    QPlainTextEdit *log;
    QLineEdit *msg;

    void setupUi(QWidget *ChatWidget)
    {
        if (ChatWidget->objectName().isEmpty())
            ChatWidget->setObjectName(QStringLiteral("ChatWidget"));
        ChatWidget->resize(400, 300);
        gridLayout = new QGridLayout(ChatWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        log = new QPlainTextEdit(ChatWidget);
        log->setObjectName(QStringLiteral("log"));

        gridLayout->addWidget(log, 0, 0, 1, 1);

        msg = new QLineEdit(ChatWidget);
        msg->setObjectName(QStringLiteral("msg"));

        gridLayout->addWidget(msg, 1, 0, 1, 1);

        QWidget::setTabOrder(msg, log);

        retranslateUi(ChatWidget);
        QObject::connect(msg, SIGNAL(editingFinished()), ChatWidget, SLOT(sendMessage()));

        QMetaObject::connectSlotsByName(ChatWidget);
    } // setupUi

    void retranslateUi(QWidget *ChatWidget)
    {
        ChatWidget->setWindowTitle(QApplication::translate("ChatWidget", "ChatWidget", 0));
    } // retranslateUi

};

namespace Ui {
    class ChatWidget: public Ui_ChatWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATWIDGET_H
