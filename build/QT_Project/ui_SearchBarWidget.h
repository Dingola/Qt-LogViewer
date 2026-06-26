/********************************************************************************
** Form generated from reading UI file 'SearchBarWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEARCHBARWIDGET_H
#define UI_SEARCHBARWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SearchBarWidget
{
public:
    QGridLayout *gridLayout;
    QComboBox *comboBoxSearchArea;
    QHBoxLayout *horizontalLayout;
    QCheckBox *checkBoxLiveSearch;
    QCheckBox *checkBoxRegEx;
    QSpacerItem *horizontalSpacer;
    QLineEdit *lineEditSearch;
    QPushButton *pushButtonSearch;

    void setupUi(QWidget *SearchBarWidget)
    {
        if (SearchBarWidget->objectName().isEmpty())
            SearchBarWidget->setObjectName("SearchBarWidget");
        SearchBarWidget->resize(336, 70);
        gridLayout = new QGridLayout(SearchBarWidget);
        gridLayout->setObjectName("gridLayout");
        comboBoxSearchArea = new QComboBox(SearchBarWidget);
        comboBoxSearchArea->setObjectName("comboBoxSearchArea");

        gridLayout->addWidget(comboBoxSearchArea, 0, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        horizontalLayout->setContentsMargins(12, -1, -1, -1);
        checkBoxLiveSearch = new QCheckBox(SearchBarWidget);
        checkBoxLiveSearch->setObjectName("checkBoxLiveSearch");
        checkBoxLiveSearch->setChecked(true);

        horizontalLayout->addWidget(checkBoxLiveSearch);

        checkBoxRegEx = new QCheckBox(SearchBarWidget);
        checkBoxRegEx->setObjectName("checkBoxRegEx");

        horizontalLayout->addWidget(checkBoxRegEx);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        gridLayout->addLayout(horizontalLayout, 1, 1, 1, 1);

        lineEditSearch = new QLineEdit(SearchBarWidget);
        lineEditSearch->setObjectName("lineEditSearch");

        gridLayout->addWidget(lineEditSearch, 0, 1, 1, 1);

        pushButtonSearch = new QPushButton(SearchBarWidget);
        pushButtonSearch->setObjectName("pushButtonSearch");

        gridLayout->addWidget(pushButtonSearch, 0, 2, 1, 1);


        retranslateUi(SearchBarWidget);

        QMetaObject::connectSlotsByName(SearchBarWidget);
    } // setupUi

    void retranslateUi(QWidget *SearchBarWidget)
    {
        SearchBarWidget->setWindowTitle(QCoreApplication::translate("SearchBarWidget", "SearchBarWidget", nullptr));
#if QT_CONFIG(tooltip)
        checkBoxLiveSearch->setToolTip(QCoreApplication::translate("SearchBarWidget", "Trigger search while typing (without pressing Enter or Search).", nullptr));
#endif // QT_CONFIG(tooltip)
        checkBoxLiveSearch->setText(QCoreApplication::translate("SearchBarWidget", "Live Search", nullptr));
        checkBoxRegEx->setText(QCoreApplication::translate("SearchBarWidget", "Regex", nullptr));
        pushButtonSearch->setText(QCoreApplication::translate("SearchBarWidget", "Search", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SearchBarWidget: public Ui_SearchBarWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEARCHBARWIDGET_H
