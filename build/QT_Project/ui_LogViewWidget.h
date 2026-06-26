/********************************************************************************
** Form generated from reading UI file 'LogViewWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGVIEWWIDGET_H
#define UI_LOGVIEWWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"

QT_BEGIN_NAMESPACE

class Ui_LogViewWidget
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayoutTop;
    QToolButton *filesInViewToolButton;
    LogFilterWidget *logFilterWidget;
    LogTableView *logTableView;

    void setupUi(QWidget *LogViewWidget)
    {
        if (LogViewWidget->objectName().isEmpty())
            LogViewWidget->setObjectName("LogViewWidget");
        LogViewWidget->resize(274, 240);
        verticalLayout = new QVBoxLayout(LogViewWidget);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayoutTop = new QHBoxLayout();
        horizontalLayoutTop->setObjectName("horizontalLayoutTop");
        filesInViewToolButton = new QToolButton(LogViewWidget);
        filesInViewToolButton->setObjectName("filesInViewToolButton");

        horizontalLayoutTop->addWidget(filesInViewToolButton);

        logFilterWidget = new LogFilterWidget(LogViewWidget);
        logFilterWidget->setObjectName("logFilterWidget");

        horizontalLayoutTop->addWidget(logFilterWidget);


        verticalLayout->addLayout(horizontalLayoutTop);

        logTableView = new LogTableView(LogViewWidget);
        logTableView->setObjectName("logTableView");

        verticalLayout->addWidget(logTableView);


        retranslateUi(LogViewWidget);

        QMetaObject::connectSlotsByName(LogViewWidget);
    } // setupUi

    void retranslateUi(QWidget *LogViewWidget)
    {
        LogViewWidget->setWindowTitle(QCoreApplication::translate("LogViewWidget", "LogViewWidget", nullptr));
        filesInViewToolButton->setText(QCoreApplication::translate("LogViewWidget", "Files..", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogViewWidget: public Ui_LogViewWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGVIEWWIDGET_H
