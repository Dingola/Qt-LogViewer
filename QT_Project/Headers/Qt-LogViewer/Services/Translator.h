#pragma once

#include <QLocale>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTranslator>

/**
 * @file Translator.h
 * @brief Provides translation functionality for the application.
 */
class Translator: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a Translator object.
         *
         * Initializes the translation system and sets up the translations directory.
         *
         * @param parent The parent QObject, or nullptr.
         */
        explicit Translator(QObject* parent = nullptr);

        /**
         * @brief Destroys the Translator object.
         */
        ~Translator() override = default;

        // NOLINTBEGIN(modernize-use-trailing-return-type)

        /**
         * @brief Loads the language translations for the specified language code.
         *
         * Loads both Qt and application translation files for the given language code.
         *
         * @param language The language code to load (e.g. "en", "de").
         * @return True if the translations were loaded successfully, false otherwise.
         */
        Q_INVOKABLE bool load_translation(const QString& language_code);

        /**
         * @brief Loads the language translations for the specified locale.
         *
         * Loads both Qt and application translation files for the given locale.
         *
         * @param locale The QLocale to load translations for.
         * @return True if the translations were loaded successfully, false otherwise.
         */
        Q_INVOKABLE bool load_translation(const QLocale& locale);

        /**
         * @brief Loads the default language translations.
         *
         * Loads both Qt and application translation files for the default language.
         *
         * @return True if the translations were loaded successfully, false otherwise.
         */
        Q_INVOKABLE bool load_default_translation();

        /**
         * @brief Returns the current language code.
         *
         * @return The current language code (e.g. "en", "de").
         */
        [[nodiscard]] Q_INVOKABLE QString get_current_language_code() const;

        /**
         * @brief Returns a list of available language codes found in the translations directory.
         * @return A QStringList of language codes (e.g. "en", "de").
         */
        [[nodiscard]] Q_INVOKABLE QStringList get_available_language_codes() const;

        /**
         * @brief Returns a list of available language names found in the translations directory.
         * @return A QStringList of language names (e.g. "English", "German").
         */
        [[nodiscard]] Q_INVOKABLE QStringList get_available_language_names() const;

        /**
         * @brief Returns a map of language codes to language names found in the translations
         * directory.
         * @return A QMap mapping language code to language name.
         */
        [[nodiscard]] Q_INVOKABLE QMap<QString, QString> get_language_code_name_map() const;

        // NOLINTEND(modernize-use-trailing-return-type)

    private:
        /**
         * @brief Removes installed translators if they are not empty.
         *
         * Uninstalls any previously installed Qt or application translators.
         */
        auto remove_none_empty_translators() -> void;

        /**
         * @brief Loads the translations for the specified locale and filename into the given
         * translator.
         *
         * Attempts to load the translation file for the given locale and filename.
         *
         * @param locale The QLocale to load translations for.
         * @param filename The base filename of the translation file.
         * @param translator The QTranslator object to load the translations into.
         * @return True if the translations were loaded successfully, false otherwise.
         */
        [[nodiscard]] auto load(const QLocale& locale, const QString& filename,
                                QTranslator& translator) -> bool;

    signals:
        /**
         * @brief Emitted when the language is changed.
         */
        void languageChanged();

    private:
        QTranslator m_qt_translator;
        QTranslator m_app_translator;
        QString m_translations_path;
        QLocale m_current_locale;
};
