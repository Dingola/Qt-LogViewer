/********************************************************************************
** Form generated from reading UI file 'StartPageWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STARTPAGEWIDGET_H
#define UI_STARTPAGEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StartPageWidget
{
public:
    QGridLayout *gridLayout;
    QLabel *welcomeLabel;
    QSpacerItem *verticalSpacer_3;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_4;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QPushButton *openFileButton;
    QPushButton *openSessionButton;
    QPushButton *reopenSessionButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QListView *recentFilesView;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *openSelectedFileButton;
    QPushButton *clearRecentFilesButton;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QListView *recentSessionsView;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *openSelectedSessionButton;
    QPushButton *deleteSessionButton;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QWidget *StartPageWidget)
    {
        if (StartPageWidget->objectName().isEmpty())
            StartPageWidget->setObjectName("StartPageWidget");
        StartPageWidget->resize(1466, 763);
        gridLayout = new QGridLayout(StartPageWidget);
        gridLayout->setObjectName("gridLayout");
        welcomeLabel = new QLabel(StartPageWidget);
        welcomeLabel->setObjectName("welcomeLabel");
        welcomeLabel->setAlignment(Qt::AlignBottom|Qt::AlignLeading|Qt::AlignLeft);

        gridLayout->addWidget(welcomeLabel, 0, 1, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 99, QSizePolicy::Minimum, QSizePolicy::Preferred);

        gridLayout->addItem(verticalSpacer_3, 0, 2, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(270, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_3, 1, 0, 1, 1);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(12);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        horizontalLayout_4->setSizeConstraint(QLayout::SetMinimumSize);
        groupBox_3 = new QGroupBox(StartPageWidget);
        groupBox_3->setObjectName("groupBox_3");
        groupBox_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(24, -1, 24, -1);
        openFileButton = new QPushButton(groupBox_3);
        openFileButton->setObjectName("openFileButton");

        verticalLayout_3->addWidget(openFileButton);

        openSessionButton = new QPushButton(groupBox_3);
        openSessionButton->setObjectName("openSessionButton");

        verticalLayout_3->addWidget(openSessionButton);

        reopenSessionButton = new QPushButton(groupBox_3);
        reopenSessionButton->setObjectName("reopenSessionButton");

        verticalLayout_3->addWidget(reopenSessionButton);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);


        horizontalLayout_4->addWidget(groupBox_3);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(12);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(-1, 0, -1, -1);
        groupBox = new QGroupBox(StartPageWidget);
        groupBox->setObjectName("groupBox");
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName("verticalLayout");
        recentFilesView = new QListView(groupBox);
        recentFilesView->setObjectName("recentFilesView");
        recentFilesView->setAlternatingRowColors(true);
        recentFilesView->setSelectionMode(QAbstractItemView::SingleSelection);
        recentFilesView->setSpacing(4);
        recentFilesView->setUniformItemSizes(true);

        verticalLayout->addWidget(recentFilesView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        openSelectedFileButton = new QPushButton(groupBox);
        openSelectedFileButton->setObjectName("openSelectedFileButton");

        horizontalLayout->addWidget(openSelectedFileButton);

        clearRecentFilesButton = new QPushButton(groupBox);
        clearRecentFilesButton->setObjectName("clearRecentFilesButton");

        horizontalLayout->addWidget(clearRecentFilesButton);


        verticalLayout->addLayout(horizontalLayout);


        horizontalLayout_3->addWidget(groupBox);

        groupBox_2 = new QGroupBox(StartPageWidget);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        recentSessionsView = new QListView(groupBox_2);
        recentSessionsView->setObjectName("recentSessionsView");
        recentSessionsView->setAlternatingRowColors(true);
        recentSessionsView->setSelectionMode(QAbstractItemView::SingleSelection);
        recentSessionsView->setSpacing(4);
        recentSessionsView->setUniformItemSizes(true);

        verticalLayout_2->addWidget(recentSessionsView);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        openSelectedSessionButton = new QPushButton(groupBox_2);
        openSelectedSessionButton->setObjectName("openSelectedSessionButton");

        horizontalLayout_2->addWidget(openSelectedSessionButton);

        deleteSessionButton = new QPushButton(groupBox_2);
        deleteSessionButton->setObjectName("deleteSessionButton");

        horizontalLayout_2->addWidget(deleteSessionButton);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout_3->addWidget(groupBox_2);


        horizontalLayout_4->addLayout(horizontalLayout_3);

        horizontalLayout_4->setStretch(0, 1);
        horizontalLayout_4->setStretch(1, 4);

        gridLayout->addLayout(horizontalLayout_4, 1, 1, 1, 2);

        horizontalSpacer_4 = new QSpacerItem(270, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 1, 3, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 186, QSizePolicy::Minimum, QSizePolicy::Preferred);

        gridLayout->addItem(verticalSpacer_2, 2, 2, 1, 1);

        gridLayout->setRowStretch(0, 2);
        gridLayout->setRowStretch(1, 15);
        gridLayout->setRowStretch(2, 5);
        gridLayout->setColumnStretch(0, 3);
        gridLayout->setColumnStretch(1, 25);
        gridLayout->setColumnStretch(3, 5);

        retranslateUi(StartPageWidget);

        QMetaObject::connectSlotsByName(StartPageWidget);
    } // setupUi

    void retranslateUi(QWidget *StartPageWidget)
    {
        StartPageWidget->setWindowTitle(QCoreApplication::translate("StartPageWidget", "Form", nullptr));
        welcomeLabel->setText(QCoreApplication::translate("StartPageWidget", "Welcome to Qt-LogViewer", nullptr));
        groupBox_3->setTitle(QString());
        openFileButton->setText(QCoreApplication::translate("StartPageWidget", "Open Log File...", nullptr));
        openSessionButton->setText(QCoreApplication::translate("StartPageWidget", "Open Session...", nullptr));
        reopenSessionButton->setText(QCoreApplication::translate("StartPageWidget", "Reopen Last Session", nullptr));
        groupBox->setTitle(QCoreApplication::translate("StartPageWidget", "Recent Files", nullptr));
        openSelectedFileButton->setText(QCoreApplication::translate("StartPageWidget", "Open Selected", nullptr));
        clearRecentFilesButton->setText(QCoreApplication::translate("StartPageWidget", "Clear", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("StartPageWidget", "Recent Sessions", nullptr));
        openSelectedSessionButton->setText(QCoreApplication::translate("StartPageWidget", "Open Session", nullptr));
        deleteSessionButton->setText(QCoreApplication::translate("StartPageWidget", "Delete", nullptr));
    } // retranslateUi

};

namespace Ui {
    class StartPageWidget: public Ui_StartPageWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STARTPAGEWIDGET_H
