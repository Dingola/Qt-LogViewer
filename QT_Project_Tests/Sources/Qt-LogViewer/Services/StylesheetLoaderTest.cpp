#include "Qt-LogViewer/Services/StylesheetLoaderTest.h"

#include <QApplication>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include <QWidget>

/**
 * @brief Sets up the test fixture for each test.
 */
void StylesheetLoaderTest::SetUp()
{
    m_loader = new StylesheetLoader();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void StylesheetLoaderTest::TearDown()
{
    delete m_loader;
    m_loader = nullptr;
}

/**
 * @brief Helper to create a temporary QSS file with given content.
 * @param content The QSS content to write.
 * @return The file path of the created temporary file.
 */
auto StylesheetLoaderTest::create_temp_qss(const QString& content) -> QString
{
    QString result;
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(false);

    if (temp_file.open())
    {
        QTextStream out(&temp_file);
        out << content;
        out.flush();
        temp_file.close();
        result = temp_file.fileName();
    }

    return result;
}

/**
 * @brief Tests loading a stylesheet with variables and correct substitution.
 */
TEST_F(StylesheetLoaderTest, LoadsStylesheetWithVariables)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @ColorPrimary: #123456;
    @ColorSecondary: #abcdef;
}
QWidget { background: @ColorPrimary; color: @ColorSecondary; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    bool loaded = m_loader->load_stylesheet(file_path, "Test");
    EXPECT_TRUE(loaded);

    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#123456"));
    EXPECT_TRUE(applied.contains("#abcdef"));
    EXPECT_FALSE(applied.contains("@ColorPrimary"));
    EXPECT_FALSE(applied.contains("@ColorSecondary"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that variables are replaced only exactly (not as substrings).
 */
TEST_F(StylesheetLoaderTest, VariableReplacementIsExact)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @Color: #111111;
    @ColorExtra: #222222;
}
QWidget { background: @Color; border: 1px solid @ColorExtra; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Test"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#111111"));
    EXPECT_TRUE(applied.contains("#222222"));
    EXPECT_FALSE(applied.contains("@Color"));
    EXPECT_FALSE(applied.contains("@ColorExtra"));

    QFile::remove(file_path);
}

/**
 * @brief Tests fallback to unnamed @Variables block if theme not found.
 */
TEST_F(StylesheetLoaderTest, FallbackToUnnamedVariablesBlock)
{
    QString qss = R"(
@Variables {
    @Color: #333333;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "NonExistentTheme"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#333333"));
    EXPECT_FALSE(applied.contains("@Color"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that unresolved variables are warned about.
 */
TEST_F(StylesheetLoaderTest, WarnsOnUnresolvedVariables)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @Color: #444444;
}
QWidget { background: @Color; color: @MissingVar; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    testing::internal::CaptureStderr();
    m_loader->load_stylesheet(file_path, "Test");
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("Unresolved variable"), std::string::npos);

    QFile::remove(file_path);
}

/**
 * @brief Tests set_variable overrides and reapplies the stylesheet.
 */
TEST_F(StylesheetLoaderTest, SetVariableOverridesAndReapplies)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @Color: #555555;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Test"));
    m_loader->set_variable("Color", "#abcdef");
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#abcdef"));
    EXPECT_FALSE(applied.contains("#555555"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that loading a non-existent file fails.
 */
TEST_F(StylesheetLoaderTest, LoadNonExistentFileFails)
{
    bool loaded = m_loader->load_stylesheet(":/nonexistent.qss", "Test");
    EXPECT_FALSE(loaded);
}

/**
 * @brief Tests that loading a stylesheet with no variables still works.
 */
TEST_F(StylesheetLoaderTest, LoadsStylesheetWithoutVariables)
{
    QString qss = R"(
QWidget { background: #999999; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    bool loaded = m_loader->load_stylesheet(file_path, "");
    EXPECT_TRUE(loaded);

    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#999999"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that loading a stylesheet with multiple themes works correctly.
 */
TEST_F(StylesheetLoaderTest, LoadsCorrectThemeBlock)
{
    QString qss = R"(
@Variables[Name="Dark"] {
    @Color: #111111;
}
@Variables[Name="Light"] {
    @Color: #eeeeee;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Light"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#eeeeee"));
    EXPECT_FALSE(applied.contains("#111111"));

    QFile::remove(file_path);
}

TEST_F(StylesheetLoaderTest, VariableIsReplacedEverywhere)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @Color: #123456;
}
QWidget { background: @Color; border: 1px solid @Color; color: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Test"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_EQ(applied.count("#123456"), 3);
    EXPECT_FALSE(applied.contains("@Color"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that a named theme ignores an unnamed @Variables block.
 */
TEST_F(StylesheetLoaderTest, NamedThemeIgnoresUnnamedBlock)
{
    QString qss = R"(
@Variables {
    @Color: #000000;
}
@Variables[Name="Blue"] {
    @Color: #0000ff;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Blue"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#0000ff"));
    EXPECT_FALSE(applied.contains("#000000"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that an empty variables block does not crash the loader.
 */
TEST_F(StylesheetLoaderTest, EmptyVariablesBlockDoesNotCrash)
{
    QString qss = R"(
@Variables[Name="Empty"] {
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Empty"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("@Color"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that a malformed variables block does not crash the loader.
 */
TEST_F(StylesheetLoaderTest, MalformedVariablesBlockDoesNotCrash)
{
    QString qss = R"(
@Variables[Name="Broken"] 
    @Color: #ff00ff;
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Broken"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("@Color"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that a variable with whitespace and special characters is handled correctly.
 */
TEST_F(StylesheetLoaderTest, VariableWithWhitespaceAndSpecialChars)
{
    QString qss = R"(
@Variables[Name="Test"] {
    @MyVar:   #a1b2c3  ;
}
QWidget { background: @MyVar; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Test"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#a1b2c3"));
    EXPECT_FALSE(applied.contains("@MyVar"));

    QFile::remove(file_path);
}

/**
 * @brief Tests that get_available_themes returns all theme names from the stylesheet.
 */
TEST_F(StylesheetLoaderTest, GetAvailableThemesListsAllThemes)
{
    QString qss = R"(
@Variables[Name="Dark"] {
    @Color: #111;
}
@Variables[Name="Light"] {
    @Color: #eee;
}
@Variables {
    @Color: #abc;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Dark"));
    QStringList themes = m_loader->get_available_themes();
    EXPECT_TRUE(themes.contains("Dark"));
    EXPECT_TRUE(themes.contains("Light"));
    EXPECT_TRUE(themes.contains("Default"));
    EXPECT_EQ(themes.size(), 3);

    QFile::remove(file_path);
}

/**
 * @brief Tests that get_available_themes returns only named themes if no unnamed @Variables block
 * exists.
 */
TEST_F(StylesheetLoaderTest, GetAvailableThemesWithoutDefaultBlock)
{
    QString qss = R"(
@Variables[Name="Dark"] {
    @Color: #111;
}
@Variables[Name="Light"] {
    @Color: #eee;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Dark"));
    QStringList themes = m_loader->get_available_themes();
    EXPECT_TRUE(themes.contains("Dark"));
    EXPECT_TRUE(themes.contains("Light"));
    EXPECT_FALSE(themes.contains("Default"));
    EXPECT_EQ(themes.size(), 2);

    QFile::remove(file_path);
}

/**
 * @brief Tests that get_current_theme_name returns the last loaded theme.
 */
TEST_F(StylesheetLoaderTest, GetCurrentThemeNameReturnsCorrectTheme)
{
    QString qss = R"(
@Variables[Name="Blue"] {
    @Color: #00f;
}
QWidget { background: @Color; }
)";
    QString file_path = create_temp_qss(qss);
    ASSERT_FALSE(file_path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "Blue"));
    EXPECT_EQ(m_loader->get_current_theme_name(), "Blue");

    // Load with a different theme name
    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "NonExistent"));
    EXPECT_EQ(m_loader->get_current_theme_name(), "NonExistent");

    QFile::remove(file_path);
}
