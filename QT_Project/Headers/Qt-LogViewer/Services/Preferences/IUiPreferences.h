#pragma once

#include <QtPlugin>

#include "ILanguagePreferences.h"
#include "IMainWindowStatePersistence.h"
#include "IThemePreferences.h"

/**
 * @class IUiPreferences
 * @brief Aggregated interface for UI-related preferences.
 *
 * This interface combines language, theme, and window state persistence preferences.
 */
class IUiPreferences: public ILanguagePreferences,
                      public IMainWindowStatePersistence,
                      public IThemePreferences
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~IUiPreferences() = default;
};

Q_DECLARE_INTERFACE(IUiPreferences, "de.adrianhelbig.IUiPreferences")
