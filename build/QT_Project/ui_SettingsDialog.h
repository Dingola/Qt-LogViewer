/********************************************************************************
** Form generated from reading UI file 'SettingsDialog.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QListWidget *listWidgetCategories;
    QStackedWidget *stackedWidgetContent;
    QWidget *pageGeneral;
    QVBoxLayout *verticalLayout_6;
    QLabel *pageGeneralTitle;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *labelLang;
    QComboBox *comboBoxLang;
    QSpacerItem *horizontalSpacer_3;
    QLabel *labelLangDesc;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_5;
    QLabel *labelConfig;
    QHBoxLayout *horizontalLayout_4;
    QLabel *labelConfigLocation;
    QLineEdit *lineEditConfigLocation;
    QToolButton *toolButtonConfigLocation;
    QLabel *labelConfigDesc;
    QSpacerItem *verticalSpacer_2;
    QWidget *pageAppearance;
    QVBoxLayout *verticalLayout_4;
    QLabel *pageAppearanceTitle;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelTheme;
    QComboBox *comboBoxTheme;
    QSpacerItem *horizontalSpacer_2;
    QLabel *labelThemeDesc;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayoutButtons;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonCancel;
    QPushButton *pushButtonApply;
    QPushButton *pushButtonOk;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName("SettingsDialog");
        SettingsDialog->resize(625, 391);
        verticalLayout_3 = new QVBoxLayout(SettingsDialog);
        verticalLayout_3->setSpacing(10);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        listWidgetCategories = new QListWidget(SettingsDialog);
        listWidgetCategories->setObjectName("listWidgetCategories");

        horizontalLayout->addWidget(listWidgetCategories);

        stackedWidgetContent = new QStackedWidget(SettingsDialog);
        stackedWidgetContent->setObjectName("stackedWidgetContent");
        pageGeneral = new QWidget();
        pageGeneral->setObjectName("pageGeneral");
        verticalLayout_6 = new QVBoxLayout(pageGeneral);
        verticalLayout_6->setObjectName("verticalLayout_6");
        pageGeneralTitle = new QLabel(pageGeneral);
        pageGeneralTitle->setObjectName("pageGeneralTitle");

        verticalLayout_6->addWidget(pageGeneralTitle);

        groupBox_2 = new QGroupBox(pageGeneral);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName("verticalLayout");
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        labelLang = new QLabel(groupBox_2);
        labelLang->setObjectName("labelLang");

        horizontalLayout_3->addWidget(labelLang);

        comboBoxLang = new QComboBox(groupBox_2);
        comboBoxLang->setObjectName("comboBoxLang");

        horizontalLayout_3->addWidget(comboBoxLang);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_3);

        labelLangDesc = new QLabel(groupBox_2);
        labelLangDesc->setObjectName("labelLangDesc");
        labelLangDesc->setWordWrap(true);

        verticalLayout->addWidget(labelLangDesc);


        verticalLayout_6->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(pageGeneral);
        groupBox_3->setObjectName("groupBox_3");
        verticalLayout_5 = new QVBoxLayout(groupBox_3);
        verticalLayout_5->setObjectName("verticalLayout_5");
        labelConfig = new QLabel(groupBox_3);
        labelConfig->setObjectName("labelConfig");

        verticalLayout_5->addWidget(labelConfig);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        labelConfigLocation = new QLabel(groupBox_3);
        labelConfigLocation->setObjectName("labelConfigLocation");

        horizontalLayout_4->addWidget(labelConfigLocation);

        lineEditConfigLocation = new QLineEdit(groupBox_3);
        lineEditConfigLocation->setObjectName("lineEditConfigLocation");
        lineEditConfigLocation->setReadOnly(true);

        horizontalLayout_4->addWidget(lineEditConfigLocation);

        toolButtonConfigLocation = new QToolButton(groupBox_3);
        toolButtonConfigLocation->setObjectName("toolButtonConfigLocation");

        horizontalLayout_4->addWidget(toolButtonConfigLocation);


        verticalLayout_5->addLayout(horizontalLayout_4);

        labelConfigDesc = new QLabel(groupBox_3);
        labelConfigDesc->setObjectName("labelConfigDesc");
        labelConfigDesc->setWordWrap(true);

        verticalLayout_5->addWidget(labelConfigDesc);


        verticalLayout_6->addWidget(groupBox_3);

        verticalSpacer_2 = new QSpacerItem(20, 222, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout_6->addItem(verticalSpacer_2);

        stackedWidgetContent->addWidget(pageGeneral);
        pageAppearance = new QWidget();
        pageAppearance->setObjectName("pageAppearance");
        verticalLayout_4 = new QVBoxLayout(pageAppearance);
        verticalLayout_4->setObjectName("verticalLayout_4");
        pageAppearanceTitle = new QLabel(pageAppearance);
        pageAppearanceTitle->setObjectName("pageAppearanceTitle");

        verticalLayout_4->addWidget(pageAppearanceTitle);

        groupBox = new QGroupBox(pageAppearance);
        groupBox->setObjectName("groupBox");
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName("verticalLayout_2");
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        labelTheme = new QLabel(groupBox);
        labelTheme->setObjectName("labelTheme");

        horizontalLayout_2->addWidget(labelTheme);

        comboBoxTheme = new QComboBox(groupBox);
        comboBoxTheme->setObjectName("comboBoxTheme");

        horizontalLayout_2->addWidget(comboBoxTheme);

        horizontalSpacer_2 = new QSpacerItem(802, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_2);

        labelThemeDesc = new QLabel(groupBox);
        labelThemeDesc->setObjectName("labelThemeDesc");
        labelThemeDesc->setWordWrap(true);

        verticalLayout_2->addWidget(labelThemeDesc);


        verticalLayout_4->addWidget(groupBox);

        verticalSpacer = new QSpacerItem(20, 222, QSizePolicy::Expanding, QSizePolicy::Minimum);

        verticalLayout_4->addItem(verticalSpacer);

        stackedWidgetContent->addWidget(pageAppearance);

        horizontalLayout->addWidget(stackedWidgetContent);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 3);

        verticalLayout_3->addLayout(horizontalLayout);

        horizontalLayoutButtons = new QHBoxLayout();
        horizontalLayoutButtons->setObjectName("horizontalLayoutButtons");
        horizontalLayoutButtons->setContentsMargins(-1, -1, 6, -1);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayoutButtons->addItem(horizontalSpacer);

        pushButtonCancel = new QPushButton(SettingsDialog);
        pushButtonCancel->setObjectName("pushButtonCancel");

        horizontalLayoutButtons->addWidget(pushButtonCancel);

        pushButtonApply = new QPushButton(SettingsDialog);
        pushButtonApply->setObjectName("pushButtonApply");

        horizontalLayoutButtons->addWidget(pushButtonApply);

        pushButtonOk = new QPushButton(SettingsDialog);
        pushButtonOk->setObjectName("pushButtonOk");

        horizontalLayoutButtons->addWidget(pushButtonOk);


        verticalLayout_3->addLayout(horizontalLayoutButtons);


        retranslateUi(SettingsDialog);

        stackedWidgetContent->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QCoreApplication::translate("SettingsDialog", "Dialog", nullptr));
        pageGeneralTitle->setText(QCoreApplication::translate("SettingsDialog", "General Settings", nullptr));
        groupBox_2->setTitle(QString());
        labelLang->setText(QCoreApplication::translate("SettingsDialog", "Language", nullptr));
        labelLangDesc->setText(QCoreApplication::translate("SettingsDialog", "Select your preferred language for the application interface. Changes will take effect immediately upon clicking Apply or OK.", nullptr));
        groupBox_3->setTitle(QString());
        labelConfig->setText(QCoreApplication::translate("SettingsDialog", "Configuration", nullptr));
        labelConfigLocation->setText(QCoreApplication::translate("SettingsDialog", "Location", nullptr));
        toolButtonConfigLocation->setText(QCoreApplication::translate("SettingsDialog", "...", nullptr));
        labelConfigDesc->setText(QCoreApplication::translate("SettingsDialog", "View the location where application settings are stored. You can open the folder to back up, inspect, or reset configuration files.", nullptr));
        pageAppearanceTitle->setText(QCoreApplication::translate("SettingsDialog", "Appearance", nullptr));
        groupBox->setTitle(QString());
        labelTheme->setText(QCoreApplication::translate("SettingsDialog", "Theme", nullptr));
        labelThemeDesc->setText(QCoreApplication::translate("SettingsDialog", "Choose between a light or dark interface, or match your system default. The UI updates instantly upon saving.", nullptr));
        pushButtonCancel->setText(QCoreApplication::translate("SettingsDialog", "Cancel", nullptr));
        pushButtonApply->setText(QCoreApplication::translate("SettingsDialog", "Apply", nullptr));
        pushButtonOk->setText(QCoreApplication::translate("SettingsDialog", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
