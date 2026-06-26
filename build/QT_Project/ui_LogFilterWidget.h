/********************************************************************************
** Form generated from reading UI file 'LogFilterWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGFILTERWIDGET_H
#define UI_LOGFILTERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LogFilterWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QComboBox *comboBoxApp;
    QWidget *logLevelBar;

    void setupUi(QWidget *LogFilterWidget)
    {
        if (LogFilterWidget->objectName().isEmpty())
            LogFilterWidget->setObjectName("LogFilterWidget");
        LogFilterWidget->resize(179, 41);
        horizontalLayout = new QHBoxLayout(LogFilterWidget);
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        comboBoxApp = new QComboBox(LogFilterWidget);
        comboBoxApp->setObjectName("comboBoxApp");
        comboBoxApp->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        comboBoxApp->setMinimumContentsLength(1);

        horizontalLayout->addWidget(comboBoxApp);

        logLevelBar = new QWidget(LogFilterWidget);
        logLevelBar->setObjectName("logLevelBar");

        horizontalLayout->addWidget(logLevelBar);


        retranslateUi(LogFilterWidget);

        QMetaObject::connectSlotsByName(LogFilterWidget);
    } // setupUi

    void retranslateUi(QWidget *LogFilterWidget)
    {
        LogFilterWidget->setWindowTitle(QCoreApplication::translate("LogFilterWidget", "FilterWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogFilterWidget: public Ui_LogFilterWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGFILTERWIDGET_H
