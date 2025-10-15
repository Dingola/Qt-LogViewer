#pragma once

#include <QString>

/**
 * @class NumberFormatUtils
 * @brief Utility class for formatting numbers for UI display.
 */
class NumberFormatUtils
{
    public:
        /**
         * @brief Formats a number with abbreviated suffixes (e.g., 1.2K, 3M).
         *
         * Converts large numbers into a more readable format using suffixes for
         * thousands (K), millions (M), billions (B), and trillions (T).
         * For values less than 10 with a suffix, one decimal place is shown.
         *
         * @param value The number to format.
         * @return Abbreviated string representation.
         */
        [[nodiscard]] static auto format_number_abbreviated(double value) -> QString;

        /**
         * @brief Formats an integer with abbreviated suffixes (e.g., 1.2K, 3M).
         *
         * Converts large integer values into a more readable format using suffixes for
         * thousands (K), millions (M), billions (B), and trillions (T).
         * For values less than 10 with a suffix, one decimal place is shown.
         *
         * @param value The integer to format.
         * @return Abbreviated string representation.
         */
        [[nodiscard]] static auto format_number_abbreviated(int value) -> QString;
};
