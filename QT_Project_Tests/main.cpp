#include <gtest/gtest.h>

#include <QApplication>

/**
 * @brief Initializes and runs all Google Test unit tests.
 *
 * This function sets up the Google Test framework and executes all registered tests.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return The result of the test run (0 if all tests pass, otherwise non-zero).
 */
auto main(int argc, char* argv[]) -> int
{
    testing::InitGoogleTest(&argc, argv);
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QtTest"));
    app.setOrganizationName(QStringLiteral("QtTemplate_Tests"));
    app.setOrganizationDomain(QStringLiteral("AdrianHelbig.de"));

    return RUN_ALL_TESTS();
}
