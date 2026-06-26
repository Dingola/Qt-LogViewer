/********************************************************************************
** Form generated from reading UI file 'PaginationWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGINATIONWIDGET_H
#define UI_PAGINATIONWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PaginationWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer_3;
    QToolButton *buttonPrev;
    QWidget *pageButtonsWidget;
    QToolButton *buttonNext;
    QSpacerItem *horizontalSpacer;
    QToolButton *buttonJumpPrev;
    QLineEdit *lineEditJumpTo;
    QToolButton *buttonJumpNext;
    QSpacerItem *horizontalSpacer_4;
    QComboBox *comboBoxItemsPerPage;
    QLabel *labelItemsPerPage;

    void setupUi(QWidget *PaginationWidget)
    {
        if (PaginationWidget->objectName().isEmpty())
            PaginationWidget->setObjectName("PaginationWidget");
        PaginationWidget->resize(549, 40);
        horizontalLayout = new QHBoxLayout(PaginationWidget);
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(16, -1, 16, -1);
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_3);

        buttonPrev = new QToolButton(PaginationWidget);
        buttonPrev->setObjectName("buttonPrev");
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/Resources/left-arrow.svg"), QSize(), QIcon::Normal, QIcon::Off);
        buttonPrev->setIcon(icon);

        horizontalLayout->addWidget(buttonPrev);

        pageButtonsWidget = new QWidget(PaginationWidget);
        pageButtonsWidget->setObjectName("pageButtonsWidget");

        horizontalLayout->addWidget(pageButtonsWidget);

        buttonNext = new QToolButton(PaginationWidget);
        buttonNext->setObjectName("buttonNext");
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/Resources/right-arrow.svg"), QSize(), QIcon::Normal, QIcon::Off);
        buttonNext->setIcon(icon1);

        horizontalLayout->addWidget(buttonNext);

        horizontalSpacer = new QSpacerItem(24, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        buttonJumpPrev = new QToolButton(PaginationWidget);
        buttonJumpPrev->setObjectName("buttonJumpPrev");
        buttonJumpPrev->setIcon(icon);

        horizontalLayout->addWidget(buttonJumpPrev);

        lineEditJumpTo = new QLineEdit(PaginationWidget);
        lineEditJumpTo->setObjectName("lineEditJumpTo");
        lineEditJumpTo->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(lineEditJumpTo);

        buttonJumpNext = new QToolButton(PaginationWidget);
        buttonJumpNext->setObjectName("buttonJumpNext");
        buttonJumpNext->setIcon(icon1);

        horizontalLayout->addWidget(buttonJumpNext);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_4);

        comboBoxItemsPerPage = new QComboBox(PaginationWidget);
        comboBoxItemsPerPage->setObjectName("comboBoxItemsPerPage");

        horizontalLayout->addWidget(comboBoxItemsPerPage);

        labelItemsPerPage = new QLabel(PaginationWidget);
        labelItemsPerPage->setObjectName("labelItemsPerPage");

        horizontalLayout->addWidget(labelItemsPerPage);


        retranslateUi(PaginationWidget);

        QMetaObject::connectSlotsByName(PaginationWidget);
    } // setupUi

    void retranslateUi(QWidget *PaginationWidget)
    {
        PaginationWidget->setWindowTitle(QCoreApplication::translate("PaginationWidget", "Form", nullptr));
        buttonPrev->setText(QCoreApplication::translate("PaginationWidget", "...", nullptr));
        buttonNext->setText(QCoreApplication::translate("PaginationWidget", "...", nullptr));
        buttonJumpPrev->setText(QCoreApplication::translate("PaginationWidget", "...", nullptr));
        buttonJumpNext->setText(QCoreApplication::translate("PaginationWidget", "...", nullptr));
        labelItemsPerPage->setText(QCoreApplication::translate("PaginationWidget", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PaginationWidget: public Ui_PaginationWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGINATIONWIDGET_H
