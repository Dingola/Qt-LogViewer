/********************************************************************************
** Form generated from reading UI file 'LogFilterBarWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGFILTERBARWIDGET_H
#define UI_LOGFILTERBARWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

QT_BEGIN_NAMESPACE

class Ui_LogFilterBarWidget
{
public:
    QVBoxLayout *verticalLayout;
    LogFilterWidget *logFilterWidget;
    SearchBarWidget *searchBarWidget;

    void setupUi(QWidget *LogFilterBarWidget)
    {
        if (LogFilterBarWidget->objectName().isEmpty())
            LogFilterBarWidget->setObjectName("LogFilterBarWidget");
        LogFilterBarWidget->resize(488, 209);
        verticalLayout = new QVBoxLayout(LogFilterBarWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        logFilterWidget = new LogFilterWidget(LogFilterBarWidget);
        logFilterWidget->setObjectName("logFilterWidget");

        verticalLayout->addWidget(logFilterWidget);

        searchBarWidget = new SearchBarWidget(LogFilterBarWidget);
        searchBarWidget->setObjectName("searchBarWidget");

        verticalLayout->addWidget(searchBarWidget);


        retranslateUi(LogFilterBarWidget);

        QMetaObject::connectSlotsByName(LogFilterBarWidget);
    } // setupUi

    void retranslateUi(QWidget *LogFilterBarWidget)
    {
        LogFilterBarWidget->setWindowTitle(QCoreApplication::translate("LogFilterBarWidget", "FilterBarWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogFilterBarWidget: public Ui_LogFilterBarWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGFILTERBARWIDGET_H
