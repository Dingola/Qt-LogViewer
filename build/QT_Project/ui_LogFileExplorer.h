/********************************************************************************
** Form generated from reading UI file 'LogFileExplorer.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGFILEEXPLORER_H
#define UI_LOGFILEEXPLORER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "Qt-LogViewer/Views/Shared/TreeView.h"

QT_BEGIN_NAMESPACE

class Ui_LogFileExplorer
{
public:
    QVBoxLayout *verticalLayout;
    TreeView *treeView;

    void setupUi(QWidget *LogFileExplorer)
    {
        if (LogFileExplorer->objectName().isEmpty())
            LogFileExplorer->setObjectName("LogFileExplorer");
        LogFileExplorer->resize(274, 210);
        verticalLayout = new QVBoxLayout(LogFileExplorer);
        verticalLayout->setObjectName("verticalLayout");
        treeView = new TreeView(LogFileExplorer);
        treeView->setObjectName("treeView");

        verticalLayout->addWidget(treeView);


        retranslateUi(LogFileExplorer);

        QMetaObject::connectSlotsByName(LogFileExplorer);
    } // setupUi

    void retranslateUi(QWidget *LogFileExplorer)
    {
        LogFileExplorer->setWindowTitle(QCoreApplication::translate("LogFileExplorer", "LogFileExplorer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LogFileExplorer: public Ui_LogFileExplorer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGFILEEXPLORER_H
