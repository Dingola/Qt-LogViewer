/********************************************************************************
** Form generated from reading UI file 'LogLevelFilterItemWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGLEVELFILTERITEMWIDGET_H
#define UI_LOGLEVELFILTERITEMWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LogLevelFilterItemWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBox;
    QLabel *labelCount;

    void setupUi(QWidget *LogLevelFilterItemWidget)
    {
        if (LogLevelFilterItemWidget->objectName().isEmpty())
            LogLevelFilterItemWidget->setObjectName("LogLevelFilterItemWidget");
        LogLevelFilterItemWidget->resize(100, 38);
        horizontalLayout = new QHBoxLayout(LogLevelFilterItemWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        checkBox = new QCheckBox(LogLevelFilterItemWidget);
        checkBox->setObjectName("checkBox");

        horizontalLayout->addWidget(checkBox);

        labelCount = new QLabel(LogLevelFilterItemWidget);
        labelCount->setObjectName("labelCount");

        horizontalLayout->addWidget(labelCount);


        retranslateUi(LogLevelFilterItemWidget);

        QMetaObject::connectSlotsByName(LogLevelFilterItemWidget);
    } // setupUi

    void retranslateUi(QWidget *LogLevelFilterItemWidget)
    {
        LogLevelFilterItemWidget->setWindowTitle(QCoreApplication::translate("LogLevelFilterItemWidget", "Form", nullptr));
        checkBox->setText(QCoreApplication::translate("LogLevelFilterItemWidget", "LogLevel", nullptr));
        labelCount->setText(QCoreApplication::translate("LogLevelFilterItemWidget", "0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogLevelFilterItemWidget: public Ui_LogLevelFilterItemWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGLEVELFILTERITEMWIDGET_H
