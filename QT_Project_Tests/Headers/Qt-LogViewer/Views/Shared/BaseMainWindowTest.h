#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/AppPreferencesInterface.h"
#include "Qt-LogViewer/Views/Shared/BaseMainWindow.h"

/**
 * @brief Google Mock for AppPreferencesInterface.
 */
class MockPreferences: public QObject, public AppPreferencesInterface
{
        Q_OBJECT
        Q_INTERFACES(AppPreferencesInterface)

    public:
        MOCK_METHOD(QString, get_theme, (), (override));
        MOCK_METHOD(void, set_theme, (const QString& value), (override));
        MOCK_METHOD(QString, get_language_code, (), (override));
        MOCK_METHOD(void, set_language_code, (const QString& language_code), (override));
        MOCK_METHOD(QString, get_language_name, (), (override));
        MOCK_METHOD(void, set_language_name, (const QString& language_name), (override));
        MOCK_METHOD(QByteArray, get_mainwindow_geometry, (), (override));
        MOCK_METHOD(void, set_mainwindow_geometry, (const QByteArray& geometry), (override));
        MOCK_METHOD(QByteArray, get_mainwindow_state, (), (override));
        MOCK_METHOD(void, set_mainwindow_state, (const QByteArray& state), (override));
        MOCK_METHOD(int, get_mainwindow_windowstate, (), (override));
        MOCK_METHOD(void, set_mainwindow_windowstate, (int state), (override));

    signals:
        void languageCodeChanged(const QString& language_code) override;
        void languageNameChanged(const QString& language_name) override;
        void themeChanged(const QString& theme_name) override;
};

/**
 * @brief Test subclass to expose protected methods for testing.
 */
class TestableBaseMainWindow: public BaseMainWindow
{
        Q_OBJECT

    public:
        using BaseMainWindow::BaseMainWindow;
        using BaseMainWindow::restore_window_settings;
        using BaseMainWindow::save_window_settings;
};

/**
 * @file BaseMainWindowTest.h
 * @brief Test fixture for BaseMainWindow.
 */
class BaseMainWindowTest: public ::testing::Test
{
    protected:
        BaseMainWindowTest() = default;
        ~BaseMainWindowTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a temporary QSS file with given content.
         * @param content The QSS content to write.
         * @return The file path of the created temporary file.
         */
        [[nodiscard]] QString create_temp_qss(const QString& content);

        TestableBaseMainWindow* m_window = nullptr;
        MockPreferences* m_mock_prefs = nullptr;
        QString m_temp_qss_path;
};
