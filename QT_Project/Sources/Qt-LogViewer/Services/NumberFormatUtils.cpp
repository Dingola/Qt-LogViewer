#include "Qt-LogViewer/Services/NumberFormatUtils.h"

/**
 * @brief Formats a number with abbreviated suffixes (e.g., 1.2K, 3M).
 *        Handles negative numbers and rounds consistently.
 * @param value The number to format.
 * @return Abbreviated string representation.
 */
auto NumberFormatUtils::format_number_abbreviated(double value) -> QString
{
    constexpr const char* suffixes[] = {"", "K", "M", "B", "T"};
    constexpr int num_suffixes = sizeof(suffixes) / sizeof(suffixes[0]);

    double abs_value = std::abs(value);
    int suffix_index = 0;
    int decimals = 0;
    QString number_str;

    if (abs_value < 1000.0)
    {
        decimals = (abs_value < 10.0 && abs_value != static_cast<int>(abs_value))
                       ? 1
                       : (abs_value != static_cast<int>(abs_value) ? 1 : 0);
        number_str = QString::number(value, 'f', decimals);
    }
    else
    {
        while (abs_value >= 1000.0 && suffix_index < num_suffixes - 1)
        {
            abs_value /= 1000.0;
            ++suffix_index;
        }

        decimals = (abs_value < 10.0 && suffix_index > 0) ? 1 : 0;
        number_str = QString::number(abs_value, 'f', decimals);

        if (value < 0.0)
        {
            number_str.prepend('-');
        }

        number_str += suffixes[suffix_index];
    }

    // Remove trailing .0 for whole numbers
    if (number_str.endsWith(".0"))
    {
        number_str.chop(2);
    }

    return number_str;
}

/**
 * @brief Formats an integer with abbreviated suffixes (e.g., 1.2K, 3M).
 * @param value The integer to format.
 * @return Abbreviated string representation.
 */
auto NumberFormatUtils::format_number_abbreviated(int value) -> QString
{
    return format_number_abbreviated(static_cast<double>(value));
}
