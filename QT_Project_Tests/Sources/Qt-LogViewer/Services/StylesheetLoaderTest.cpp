#include "Qt-LogViewer/Services/StylesheetLoaderTest.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>
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

    // Load with a different theme name to ensure property updates even if theme not found
    ASSERT_TRUE(m_loader->load_stylesheet(file_path, "NonExistent"));
    EXPECT_EQ(m_loader->get_current_theme_name(), "NonExistent");

    QFile::remove(file_path);
}

/**
 * @brief Indirectly tests resolve_variable recursion and replacement loop,
 *        including inner captures and repeated matching until no @Var remains.
 */
TEST_F(StylesheetLoaderTest, ResolveVariableRecursiveChain)
{
    QString qss = R"(
@Variables[Name="Chain"] {
    @A: @B;
    @B: @C;
    @C: #010203;
}
QWidget { color: @A; border-color: @B; outline-color: @C; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Chain"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_TRUE(applied.contains("#010203"));
    EXPECT_EQ(applied.count("#010203"), 3);
    EXPECT_FALSE(applied.contains("@A"));
    EXPECT_FALSE(applied.contains("@B"));
    EXPECT_FALSE(applied.contains("@C"));
}

/**
 * @brief Indirectly tests resolve_variable cycle prevention via seen-set.
 *        A cycle should resolve to empty strings for involved variables.
 */
TEST_F(StylesheetLoaderTest, ResolveVariableCyclePrevention)
{
    QString qss = R"(
@Variables[Name="Cycle"] {
    @A: @B;
    @B: @A;
}
QWidget { color: @A; border-color: @B; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Cycle"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_FALSE(applied.contains("@A"));
    EXPECT_FALSE(applied.contains("@B"));
    EXPECT_FALSE(applied.contains("#"));
}

/**
 * @brief Exercises substitute_variables iteration over multiple keys with varied lengths
 *        to ensure length-descending sort avoids partial replacements.
 */
TEST_F(StylesheetLoaderTest, SubstituteVariablesMultipleKeysIteration)
{
    QString qss = R"(
@Variables[Name="LenSort"] {
    @Color: #111111;
    @ColorPrimary: #222222;
    @ColorPrimaryDark: #333333;
}
QWidget { 
    background: @ColorPrimaryDark; 
    border: 1px solid @ColorPrimary; 
    color: @Color; 
}
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "LenSort"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_TRUE(applied.contains("#333333"));
    EXPECT_TRUE(applied.contains("#222222"));
    EXPECT_TRUE(applied.contains("#111111"));
    EXPECT_EQ(applied.count("@"), 0);
}

/**
 * @brief Ensures theme parsing loop covers empty theme names exclusion and default block detection.
 */
TEST_F(StylesheetLoaderTest, ParseAvailableThemesLoopAndDefaultDetection)
{
    QString qss = R"(
@Variables[Name="A"] { @X: 1; }
@Variables[Name=""] { @Y: 2; }  /* should be ignored due to empty name */
@Variables[Name="B"] { @Z: 3; }
@Variables { @D: 4; }            /* triggers Default */
QWidget { background: @X; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "A"));
    QStringList themes = m_loader->get_available_themes();
    EXPECT_TRUE(themes.contains("A"));
    EXPECT_TRUE(themes.contains("B"));
    EXPECT_TRUE(themes.contains("Default"));
    EXPECT_EQ(themes.size(), 3);
}

/**
 * @brief Exercises parse_variables_block loop by providing many variables in one block.
 */
TEST_F(StylesheetLoaderTest, ParseVariablesBlockManyEntries)
{
    QString qss = R"(
@Variables[Name="Many"] {
    @V1: #111;
    @V2: #222;
    @V3: #333;
    @V4: #444;
    @V5: #555;
}
QWidget { 
    color: @V1; 
    background: @V2; 
    border-color: @V3; 
    outline-color: @V4; 
    selection-color: @V5;
}
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Many"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_TRUE(applied.contains("#111"));
    EXPECT_TRUE(applied.contains("#222"));
    EXPECT_TRUE(applied.contains("#333"));
    EXPECT_TRUE(applied.contains("#444"));
    EXPECT_TRUE(applied.contains("#555"));
    EXPECT_EQ(applied.count("@V"), 0);
}

/**
 * @brief Exercises extract_variables_block conditional matches indirectly:
 *  - First branch: named block found.
 *  - Second branch: named not found but default exists.
 *  - No block: neither named nor default found -> variables remain unresolved.
 */
TEST_F(StylesheetLoaderTest, ExtractVariablesBlockBranchingViaLoad)
{
    // Named present
    QString qss_named = R"(
@Variables[Name="Named"] { @C: #abc; }
QWidget { color: @C; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss_named, "Named"));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("#abc"));

    // Named missing, default present -> default used
    QString qss_default = R"(
@Variables { @C: #def; }
QWidget { color: @C; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss_default, "Unknown"));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("#def"));

    // No variables block at all -> unresolved remains
    QString qss_none = R"(
QWidget { color: @C; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss_none, ""));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("@C"));
}

/**
 * @brief Covers load_stylesheet open-branch and unresolved warning detection indirectly.
 */
TEST_F(StylesheetLoaderTest, LoadStylesheetWarnsOnUnresolvedVariables)
{
    QString qss = R"(
@Variables[Name="Test"] { @Color: #444444; }
QWidget { background: @Color; color: @MissingVar; }
)";
    QString path = create_temp_qss(qss);
    ASSERT_FALSE(path.isEmpty());

    // Triggers file open branch and unresolved warning path.
    ASSERT_TRUE(m_loader->load_stylesheet(path, "Test"));
    QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#444444"));
    EXPECT_TRUE(applied.contains("@MissingVar"));  // still unresolved

    QFile::remove(path);
}

/**
 * @brief Covers iteration over m_variables when resolving them in load_stylesheet.
 */
TEST_F(StylesheetLoaderTest, LoadStylesheetIteratesOverVariablesForResolution)
{
    QString qss = R"(
@Variables[Name="Iter"] {
    @Base: #101010;
    @Accent: @Base;
    @Primary: @Accent;
}
QWidget { background: @Primary; border-color: @Accent; color: @Base; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Iter"));
    QString applied = m_loader->get_current_stylesheet();

    EXPECT_EQ(applied.count("#101010"), 3);
    EXPECT_EQ(applied.count("@"), 0);
}

/**
 * @brief Ensures set_theme switches themes correctly and exercises re-parsing.
 */
TEST_F(StylesheetLoaderTest, SetThemeSwitchesNamedAndDefault)
{
    QString qss = R"(
@Variables[Name="Dark"] { @Color: #000; }
@Variables[Name="Light"] { @Color: #fff; }
@Variables { @Color: #abc; }
QWidget { color: @Color; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Dark"));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("#000"));

    ASSERT_TRUE(m_loader->set_theme("Light"));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("#fff"));

    ASSERT_TRUE(m_loader->set_theme(""));
    EXPECT_TRUE(m_loader->get_current_stylesheet().contains("#abc"));
}

/**
 * @brief Validates auto-reload: enabling watcher, modifying file, and observing re-application.
 *        Uses bounded polling to remain deterministic.
 */
TEST_F(StylesheetLoaderTest, AutoReloadUpdatesAppliedStylesheetOnFileChange)
{
    // Start with Dark
    QString initial_qss = R"(
@Variables[Name="Dark"] { @Color: #000000; }
QWidget { color: @Color; }
)";
    QString path = create_temp_qss(initial_qss);
    ASSERT_FALSE(path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(path, "Dark"));
    ASSERT_TRUE(m_loader->enable_auto_reload(true));

    QString applied_before = m_loader->get_current_stylesheet();
    ASSERT_TRUE(applied_before.contains("#000000"));

    // Update same theme "Dark" to a new 6-digit hex value for robust matching
    QString updated_qss = R"(
@Variables[Name="Dark"] { @Color: #FFFFFF; }
QWidget { color: @Color; }
)";
    {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        QTextStream out(&f);
        out << updated_qss;
        out.flush();
        f.close();
    }

    // Poll for auto-reload effect (bounded time)
    QElapsedTimer timer;
    timer.start();
    const qint64 timeout_ms = 3000;
    bool seen_update = false;

    while (timer.elapsed() < timeout_ms)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        const QString applied_now = m_loader->get_current_stylesheet();

        if (applied_now.contains("#FFFFFF"))
        {
            seen_update = true;
            break;
        }

        QThread::msleep(10);
    }

    EXPECT_TRUE(seen_update) << "Auto-reload did not apply updated stylesheet within timeout.";

    QFile::remove(path);
}

/**
 * @brief Tests that auto-reload reapplies the same theme block on file changes.
 */
TEST_F(StylesheetLoaderTest, AutoReloadSameThemeReappliesOnFileChange)
{
    QString initial_qss = R"(
@Variables[Name="Dark"] { @Color: #000; }
QWidget { color: @Color; }
)";
    QString path = create_temp_qss(initial_qss);
    ASSERT_FALSE(path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(path, "Dark"));
    ASSERT_TRUE(m_loader->enable_auto_reload(true));
    ASSERT_TRUE(m_loader->get_current_stylesheet().contains("#000"));

    // Update same theme "Dark"
    QString updated_qss = R"(
@Variables[Name="Dark"] { @Color: #0f0; }
QWidget { color: @Color; }
)";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QTextStream out(&f);
    out << updated_qss;
    out.flush();
    f.close();

    QElapsedTimer timer;
    timer.start();
    bool seen_update = false;
    while (timer.elapsed() < 2000)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QString applied_now = m_loader->get_current_stylesheet();
        if (applied_now.contains("#0f0"))
        {
            seen_update = true;
            break;
        }
        QThread::msleep(10);
    }
    EXPECT_TRUE(seen_update);

    QFile::remove(path);
}

/**
 * @brief Tests that when the current theme block is removed, the default block is used on reload.
 */
TEST_F(StylesheetLoaderTest, AutoReloadDefaultBlockUsedIfThemeMissing)
{
    QString initial_qss = R"(
@Variables[Name="Dark"] { @Color: #000; }
@Variables { @Color: #123; } /* default */
QWidget { color: @Color; }
)";
    QString path = create_temp_qss(initial_qss);
    ASSERT_FALSE(path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(path, "Dark"));
    ASSERT_TRUE(m_loader->enable_auto_reload(true));
    ASSERT_TRUE(m_loader->get_current_stylesheet().contains("#000"));

    // Remove "Dark" block, keep default
    QString updated_qss = R"(
@Variables { @Color: #abc; }
QWidget { color: @Color; }
)";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
    QTextStream out(&f);
    out << updated_qss;
    out.flush();
    f.close();

    QElapsedTimer timer;
    timer.start();
    bool seen_update = false;
    while (timer.elapsed() < 2000)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QString applied_now = m_loader->get_current_stylesheet();
        if (applied_now.contains("#abc"))
        {
            seen_update = true;
            break;
        }
        QThread::msleep(10);
    }
    EXPECT_TRUE(seen_update);

    QFile::remove(path);
}

/**
 * @brief Tests that disabling auto-reload prevents updates on file changes.
 */
TEST_F(StylesheetLoaderTest, AutoReloadDisabledDoesNotUpdate)
{
    QString initial_qss = R"(
@Variables[Name="Dark"] { @Color: #000; }
QWidget { color: @Color; }
)";
    QString path = create_temp_qss(initial_qss);
    ASSERT_FALSE(path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(path, "Dark"));
    // Disabling should return false (no watcher path configured).
    EXPECT_FALSE(m_loader->enable_auto_reload(false));
    ASSERT_TRUE(m_loader->get_current_stylesheet().contains("#000"));

    // Change file but auto-reload disabled
    QString updated_qss = R"(
@Variables[Name="Dark"] { @Color: #0f0; }
QWidget { color: @Color; }
)";
    {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        QTextStream out(&f);
        out << updated_qss;
        out.flush();
        f.close();
    }

    // Process events and ensure not updated
    QElapsedTimer timer;
    timer.start();
    bool updated = false;
    while (timer.elapsed() < 500)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        if (m_loader->get_current_stylesheet().contains("#0f0"))
        {
            updated = true;
            break;
        }
        QThread::msleep(10);
    }
    EXPECT_FALSE(updated);

    QFile::remove(path);
}

/**
 * @brief Tests that rapid successive file changes result in only the last change being applied
 *        due to debounce timer behavior.
 */
TEST_F(StylesheetLoaderTest, AutoReloadDebounceAppliesLastWrite)
{
    QString initial_qss = R"(
@Variables[Name="Dark"] { @Color: #000000; }
QWidget { color: @Color; }
)";
    QString path = create_temp_qss(initial_qss);
    ASSERT_FALSE(path.isEmpty());

    ASSERT_TRUE(m_loader->load_stylesheet(path, "Dark"));
    ASSERT_TRUE(m_loader->enable_auto_reload(true));
    ASSERT_TRUE(m_loader->get_current_stylesheet().contains("#000000"));

    // Rapid writes: three quick changes to the same theme; use 6-digit hex values
    auto write_text = [&](const QString& text) {
        QFile f(path);
        EXPECT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        QTextStream out(&f);
        out << text;
        out.flush();
        f.close();
        // Let the OS and watcher register the change
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        QThread::msleep(20);
    };

    write_text(R"(
@Variables[Name="Dark"] { @Color: #000001; }
QWidget { color: @Color; }
)");
    write_text(R"(
@Variables[Name="Dark"] { @Color: #000002; }
QWidget { color: @Color; }
)");
    write_text(R"(
@Variables[Name="Dark"] { @Color: #000003; }
QWidget { color: @Color; }
)");

    // Wait for debounce; final content should be #000003
    QElapsedTimer timer;
    timer.start();
    const qint64 timeout_ms = 4000;
    bool applied_final = false;
    while (timer.elapsed() < timeout_ms)
    {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        const QString applied_now = m_loader->get_current_stylesheet();
        if (applied_now.contains("#000003"))
        {
            applied_final = true;
            break;
        }
        QThread::msleep(10);
    }
    EXPECT_TRUE(applied_final) << "Debounce did not apply the last write within timeout.";

    QFile::remove(path);
}

/**
 * @brief get_variables returns a copy of the resolved variables map (values are fully resolved).
 */
TEST_F(StylesheetLoaderTest, GetVariablesReturnsResolvedValues)
{
    const QString qss = R"(
@Variables[Name="Test"] {
    @Base: #112233;
    @Accent: @Base;
}
QWidget { color: @Accent; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Test"));

    const auto vars = m_loader->get_variables();
    ASSERT_TRUE(vars.contains("Base"));
    ASSERT_TRUE(vars.contains("Accent"));
    EXPECT_EQ(vars.value("Base"), "#112233");
    EXPECT_EQ(vars.value("Accent"), "#112233");  // resolved via recursion
}

/**
 * @brief has_variable reflects current state after load, set_variable and remove_variable.
 */
TEST_F(StylesheetLoaderTest, HasVariableReflectsAddAndRemove)
{
    const QString qss = R"(
@Variables[Name="Test"] {
    @Color: #abcdef;
}
QWidget { color: @Color; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Test"));
    EXPECT_TRUE(m_loader->has_variable("Color"));
    EXPECT_FALSE(m_loader->has_variable("Missing"));

    // add/override
    m_loader->set_variable("NewVar", "#010101");
    EXPECT_TRUE(m_loader->has_variable("NewVar"));

    // remove again
    EXPECT_TRUE(m_loader->remove_variable("NewVar"));
    EXPECT_FALSE(m_loader->has_variable("NewVar"));
}

/**
 * @brief remove_variable removes the entry and reapplies the stylesheet (placeholder remains).
 */
TEST_F(StylesheetLoaderTest, RemoveVariableRemovesAndTriggersReapply)
{
    const QString qss = R"(
@Variables[Name="Test"] {
    @Accent: #445566;
}
QWidget { color: @Accent; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Test"));

    // Sanity: value applied
    QString applied_before = m_loader->get_current_stylesheet();
    ASSERT_TRUE(applied_before.contains("#445566"));

    // Remove and verify
    EXPECT_TRUE(m_loader->remove_variable("Accent"));
    EXPECT_FALSE(m_loader->has_variable("Accent"));

    // After removal, unresolved placeholder should remain and previous color must be gone
    QString applied_after = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied_after.contains("@Accent"));
    EXPECT_FALSE(applied_after.contains("#445566"));

    // Removing again returns false (already removed)
    EXPECT_FALSE(m_loader->remove_variable("Accent"));
}

/**
 * @brief remove_variable on a non-existing variable returns false and leaves stylesheet unchanged.
 */
TEST_F(StylesheetLoaderTest, RemoveVariableNonExistingReturnsFalseAndNoChange)
{
    const QString qss = R"(
@Variables[Name="Test"] {
    @Color: #778899;
}
QWidget { color: @Color; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Test"));
    const QString applied_before = m_loader->get_current_stylesheet();

    EXPECT_FALSE(m_loader->remove_variable("DoesNotExist"));
    const QString applied_after = m_loader->get_current_stylesheet();

    EXPECT_EQ(applied_before, applied_after);
}

/**
 * @brief get_variables returns a copy; mutating the returned map must not affect internal state.
 */
TEST_F(StylesheetLoaderTest, GetVariablesReturnsCopyNotReference)
{
    const QString qss = R"(
@Variables[Name="Test"] {
    @Color: #aabbcc;
}
QWidget { color: @Color; }
)";
    ASSERT_TRUE(m_loader->load_stylesheet_from_data(qss, "Test"));

    auto vars_copy = m_loader->get_variables();
    vars_copy.insert("Injected", "should-not-appear");

    // Internal state unchanged
    EXPECT_FALSE(m_loader->has_variable("Injected"));

    // Stylesheet still uses only the real variables
    const QString applied = m_loader->get_current_stylesheet();
    EXPECT_TRUE(applied.contains("#aabbcc"));
    EXPECT_FALSE(applied.contains("should-not-appear"));
}
