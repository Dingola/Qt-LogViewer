/**
 * @file TranslatorTest.cpp
 * @brief Implements tests for the Translator service (loading, defaults, and enumeration).
 */

#include "Qt-LogViewer/Services/TranslatorTest.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QMap>
#include <QSignalSpy>
#include <QStringList>

#include "Qt-LogViewer/Services/Translator.h"

void TranslatorTest::SetUp()
{
    m_translator = new Translator();
    QApplication::processEvents();
}

void TranslatorTest::TearDown()
{
    delete m_translator;
    m_translator = nullptr;
}

/**
 * @test Default state returns current language code from the default QLocale and empty available
 * lists when the translations directory is missing.
 */
TEST_F(TranslatorTest, DefaultStateAndAvailableLanguagesWhenFolderMissing)
{
    ASSERT_NE(m_translator, nullptr);

    // Default QLocale name is typically non-empty (e.g., "en_US")
    const QString current = m_translator->get_current_language_code();
    EXPECT_FALSE(current.isEmpty());

    // When translations directory doesn't exist or has no files, no languages should be listed
    const QStringList codes = m_translator->get_available_language_codes();
    const QStringList names = m_translator->get_available_language_names();
    const QMap<QString, QString> map = m_translator->get_language_code_name_map();

    EXPECT_TRUE(codes.isEmpty());
    EXPECT_TRUE(names.isEmpty());
    EXPECT_TRUE(map.isEmpty());
}

/**
 * @test Enumeration: when app_*.qm files exist in translations dir, codes/names/map reflect them.
 *
 * Creates the translations directory alongside the test binary and touches minimal app_*.qm files.
 * Note: Files do not need to be valid .qm for enumeration; only file names are parsed.
 */
TEST_F(TranslatorTest, EnumeratesAvailableLanguagesFromAppQmFiles)
{
    // Build expected translations path like Translator does
    const QString base_dir = QCoreApplication::applicationDirPath();
    const QString translations_dir = base_dir + QStringLiteral("/translations");
    QDir dir(translations_dir);

    // Ensure a clean directory
    if (!dir.exists())
    {
        EXPECT_TRUE(QDir().mkpath(translations_dir));
    }

    // Create minimal files: app_de.qm and app_en.qm
    const QString de_file = translations_dir + QStringLiteral("/app_de.qm");
    const QString en_file = translations_dir + QStringLiteral("/app_en.qm");

    QFile f_de(de_file);
    QFile f_en(en_file);
    // Remove pre-existing files to avoid interference
    if (f_de.exists()) EXPECT_TRUE(f_de.remove());
    if (f_en.exists()) EXPECT_TRUE(f_en.remove());
    EXPECT_TRUE(f_de.open(QIODevice::WriteOnly));
    f_de.write("dummy");  // content doesn't matter for enumeration
    f_de.close();
    EXPECT_TRUE(f_en.open(QIODevice::WriteOnly));
    f_en.write("dummy");
    f_en.close();

    // Recreate Translator to pick up the directory created now
    delete m_translator;
    m_translator = new Translator();

    const QStringList codes = m_translator->get_available_language_codes();
    EXPECT_FALSE(codes.isEmpty());
    // Order is not guaranteed; check presence
    EXPECT_TRUE(codes.contains(QStringLiteral("de")));
    EXPECT_TRUE(codes.contains(QStringLiteral("en")));

    const QStringList names = m_translator->get_available_language_names();
    // Names come from QLocale(language). E.g., "German", "English"
    EXPECT_FALSE(names.isEmpty());
    EXPECT_TRUE(
        names.contains(QLocale::languageToString(QLocale(QStringLiteral("de")).language())));
    EXPECT_TRUE(
        names.contains(QLocale::languageToString(QLocale(QStringLiteral("en")).language())));

    const QMap<QString, QString> map = m_translator->get_language_code_name_map();
    EXPECT_EQ(map.value(QStringLiteral("de")),
              QLocale::languageToString(QLocale(QStringLiteral("de")).language()));
    EXPECT_EQ(map.value(QStringLiteral("en")),
              QLocale::languageToString(QLocale(QStringLiteral("en")).language()));

    // Cleanup created files
    QFile::remove(de_file);
    QFile::remove(en_file);
}

/**
 * @test Edge case: only Qt translator present (qt_en.qm exists, app_en.qm missing); load should
 * fail, fallback attempted, and languageChanged not emitted.
 */
TEST_F(TranslatorTest, LoadFailsWhenOnlyQtTranslatorPresent)
{
    const QString base_dir = QCoreApplication::applicationDirPath();
    const QString translations_dir = base_dir + QStringLiteral("/translations");
    QDir dir(translations_dir);
    ASSERT_TRUE(dir.exists());  // According to project setup, qt_en.qm is present in this folder

    // Ensure app_en.qm does not exist (simulate missing app translator)
    const QString app_file = translations_dir + QStringLiteral("/app_en.qm");
    QFile f_app(app_file);
    if (f_app.exists())
    {
        EXPECT_TRUE(f_app.remove());
    }

    // Recreate translator to ensure fresh state
    delete m_translator;
    m_translator = new Translator();

    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    const QString before = m_translator->get_current_language_code();
    const bool ok = m_translator->load_translation(QStringLiteral("en"));
    EXPECT_FALSE(ok);
    EXPECT_EQ(spy.count(), 0);
    const QString after = m_translator->get_current_language_code();
    EXPECT_EQ(before, after);
}

/**
 * @test Edge case: only one translator file present; load should fail and not change current
 * language.
 *
 * Places only app_de.qm (no qt_de.qm) and verifies load_translation("de") returns false.
 */
TEST_F(TranslatorTest, LoadFailsWhenOnlyOneTranslatorFilePresent)
{
    const QString base_dir = QCoreApplication::applicationDirPath();
    const QString translations_dir = base_dir + QStringLiteral("/translations");
    QDir dir(translations_dir);
    if (!dir.exists())
    {
        EXPECT_TRUE(QDir().mkpath(translations_dir));
    }

    // Ensure directory has only app_de.qm, but no qt_de.qm
    const QString app_file = translations_dir + QStringLiteral("/app_de.qm");
    const QString qt_file = translations_dir + QStringLiteral("/qt_de.qm");
    QFile f_app(app_file);
    QFile f_qt(qt_file);
    if (f_app.exists()) EXPECT_TRUE(f_app.remove());
    if (f_qt.exists()) EXPECT_TRUE(f_qt.remove());
    EXPECT_TRUE(f_app.open(QIODevice::WriteOnly));
    f_app.write("dummy");
    f_app.close();

    // Recreate translator to ensure fresh state
    delete m_translator;
    m_translator = new Translator();

    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    const QString before = m_translator->get_current_language_code();
    const bool ok = m_translator->load_translation(QStringLiteral("de"));
    EXPECT_FALSE(ok);
    EXPECT_EQ(spy.count(), 0);
    const QString after = m_translator->get_current_language_code();
    EXPECT_EQ(before, after);

    // Cleanup
    QFile::remove(app_file);
    QFile::remove(qt_file);
}

/**
 * @test Loading an explicit language code fails when translations are missing and does not emit
 * languageChanged. Current language is updated only on success.
 */
TEST_F(TranslatorTest, LoadTranslationByCodeFailsWithoutResources)
{
    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    const QString before = m_translator->get_current_language_code();

    const bool ok = m_translator->load_translation(QStringLiteral("de"));
    EXPECT_FALSE(ok);

    // No signal emission expected on failure
    EXPECT_EQ(spy.count(), 0);

    // Current language code remains unchanged on failure
    const QString after = m_translator->get_current_language_code();
    EXPECT_EQ(before, after);
}

/**
 * @test Loading by QLocale fails without resources; attempting default also fails without changing
 * current language or emitting languageChanged.
 */
TEST_F(TranslatorTest, LoadTranslationByLocaleAndDefaultFallbackFailWithoutResources)
{
    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    const QString before = m_translator->get_current_language_code();

    // Try an arbitrary locale
    const bool ok_locale = m_translator->load_translation(QLocale(QStringLiteral("fr_FR")));
    EXPECT_FALSE(ok_locale);

    // Try default translation explicitly
    const bool ok_default = m_translator->load_default_translation();
    EXPECT_FALSE(ok_default);

    EXPECT_EQ(spy.count(), 0);

    const QString after = m_translator->get_current_language_code();
    EXPECT_EQ(before, after);
}

/**
 * @test Repeated failed loads do not emit languageChanged; count stays 0 across attempts.
 */
TEST_F(TranslatorTest, RepeatedFailedLoadsDoNotEmitLanguageChanged)
{
    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    // Attempt multiple failed loads (no resources or partial resources)
    EXPECT_FALSE(m_translator->load_translation(QStringLiteral("es")));
    EXPECT_FALSE(m_translator->load_translation(QLocale(QStringLiteral("it_IT"))));
    EXPECT_FALSE(m_translator->load_default_translation());

    EXPECT_EQ(spy.count(), 0);
}

/**
 * @test Successful load when both qt_<code>.qm and app_<code>.qm exist (copied from qt_en.qm).
 *
 * Copies an existing valid qt_en.qm to create app_en.qm, then loads "en" successfully.
 */
TEST_F(TranslatorTest, LoadSucceedsWhenBothQtAndAppExistForEn)
{
    const QString base_dir = QCoreApplication::applicationDirPath();
    const QString translations_dir = base_dir + QStringLiteral("/translations");
    QDir dir(translations_dir);
    ASSERT_TRUE(dir.exists());

    // Source: existing valid qt_en.qm
    const QString qt_en_src = translations_dir + QStringLiteral("/qt_en.qm");
    ASSERT_TRUE(QFile::exists(qt_en_src));

    // Target app_en.qm (copy from qt_en.qm to simulate a valid app file)
    const QString app_en_dst = translations_dir + QStringLiteral("/app_en.qm");
    if (QFile::exists(app_en_dst)) EXPECT_TRUE(QFile::remove(app_en_dst));
    EXPECT_TRUE(QFile::copy(qt_en_src, app_en_dst));

    // Fresh translator
    delete m_translator;
    m_translator = new Translator();

    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    const bool ok = m_translator->load_translation(QStringLiteral("en"));
    EXPECT_TRUE(ok);
    EXPECT_GE(spy.count(), 1);

    const QString current = m_translator->get_current_language_code();
    // Depending on platform, current may be "en" or "en_US"; ensure it contains "en"
    EXPECT_TRUE(current.contains(QStringLiteral("en")));

    // Cleanup
    QFile::remove(app_en_dst);
}

/**
 * @test Switching locales removes previous translators and emits languageChanged per switch.
 *
 * Copies qt_en.qm to provide app_en.qm, app_de.qm, qt_de.qm, then loads "en" and "de" sequentially.
 */
TEST_F(TranslatorTest, SwitchLocalesEmitsOncePerSuccessfulLoadAndReplacesTranslators)
{
    const QString base_dir = QCoreApplication::applicationDirPath();
    const QString translations_dir = base_dir + QStringLiteral("/translations");
    QDir dir(translations_dir);
    ASSERT_TRUE(dir.exists());

    // Source valid file
    const QString qt_en_src = translations_dir + QStringLiteral("/qt_en.qm");
    ASSERT_TRUE(QFile::exists(qt_en_src));

    // Prepare targets: app_en.qm, app_de.qm, qt_de.qm
    const QString app_en_dst = translations_dir + QStringLiteral("/app_en.qm");
    const QString app_de_dst = translations_dir + QStringLiteral("/app_de.qm");
    const QString qt_de_dst = translations_dir + QStringLiteral("/qt_de.qm");

    // Clean possible leftovers
    if (QFile::exists(app_en_dst)) EXPECT_TRUE(QFile::remove(app_en_dst));
    if (QFile::exists(app_de_dst)) EXPECT_TRUE(QFile::remove(app_de_dst));
    if (QFile::exists(qt_de_dst)) EXPECT_TRUE(QFile::remove(qt_de_dst));

    // Copy qt_en.qm to create valid app_en.qm, app_de.qm and qt_de.qm (binary format suffices)
    EXPECT_TRUE(QFile::copy(qt_en_src, app_en_dst));
    EXPECT_TRUE(QFile::copy(qt_en_src, app_de_dst));
    EXPECT_TRUE(QFile::copy(qt_en_src, qt_de_dst));

    // Fresh translator
    delete m_translator;
    m_translator = new Translator();

    QSignalSpy spy(m_translator, &Translator::languageChanged);
    ASSERT_TRUE(spy.isValid());

    // Load "en"
    EXPECT_TRUE(m_translator->load_translation(QStringLiteral("en")));
    EXPECT_EQ(spy.count(), 1);
    const QString current_en = m_translator->get_current_language_code();
    EXPECT_TRUE(current_en.contains(QStringLiteral("en")));

    // Load "de" (should uninstall previous translators and install new ones)
    EXPECT_TRUE(m_translator->load_translation(QStringLiteral("de")));
    EXPECT_EQ(spy.count(), 2);
    const QString current_de = m_translator->get_current_language_code();
    EXPECT_TRUE(current_de.contains(QStringLiteral("de")));

    // Cleanup
    QFile::remove(app_en_dst);
    QFile::remove(app_de_dst);
    QFile::remove(qt_de_dst);
}
