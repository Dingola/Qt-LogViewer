/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/Shared/PaginationWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    LogFilterBarWidget *logFilterBarWidget;
    QVBoxLayout *verticalLayoutViewLog;
    LogTabWidget *tabWidgetLog;
    PaginationWidget *paginationWidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(422, 261);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        logFilterBarWidget = new LogFilterBarWidget(centralwidget);
        logFilterBarWidget->setObjectName("logFilterBarWidget");

        verticalLayout->addWidget(logFilterBarWidget);

        verticalLayoutViewLog = new QVBoxLayout();
        verticalLayoutViewLog->setSpacing(0);
        verticalLayoutViewLog->setObjectName("verticalLayoutViewLog");
        tabWidgetLog = new LogTabWidget(centralwidget);
        tabWidgetLog->setObjectName("tabWidgetLog");

        verticalLayoutViewLog->addWidget(tabWidgetLog);

        paginationWidget = new PaginationWidget(centralwidget);
        paginationWidget->setObjectName("paginationWidget");
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(paginationWidget->sizePolicy().hasHeightForWidth());
        paginationWidget->setSizePolicy(sizePolicy);

        verticalLayoutViewLog->addWidget(paginationWidget);


        verticalLayout->addLayout(verticalLayoutViewLog);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 422, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidgetLog->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
