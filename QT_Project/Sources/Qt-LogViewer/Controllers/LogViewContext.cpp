/**
 * @file LogViewContext.cpp
 * @brief Implements the per-view model, loaded-file and filter-state container.
 */

#include "Qt-LogViewer/Controllers/LogViewContext.h"

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs an empty log view context.
 * @param parent QObject parent.
 */
LogViewContext::LogViewContext(QObject* parent)
    : QObject(parent), m_model(new LogModel(this)), m_loaded_files(), m_filter_state()
{}

/**
 * @brief Default destructor.
 *
 * Components are parented to this QObject and will be deleted automatically.
 */
LogViewContext::~LogViewContext() = default;

/**
 * @brief Returns the underlying LogModel instance.
 * @return Pointer to `LogModel`. Never nullptr after construction.
 */
auto LogViewContext::get_model() const -> LogModel*
{
    auto* result = m_model;
    return result;
}

/**
 * @brief Appends a batch of log entries to the underlying model.
 * @param entries The batch of `LogEntry` objects to append.
 */
auto LogViewContext::append_entries(const QVector<LogEntry>& entries) -> void
{
    if (m_model != nullptr)
    {
        m_model->add_entries(entries);
    }
}

/**
 * @brief Replaces the entries in the underlying model.
 * @param entries Entries stored by the model.
 */
auto LogViewContext::replace_entries(const QVector<LogEntry>& entries) -> void
{
    if (m_model != nullptr)
    {
        m_model->set_entries(entries);
    }
}

/**
 * @brief Removes all entries that belong to the given file path from the model.
 * @param file_path Absolute file path whose entries should be removed.
 */
auto LogViewContext::remove_entries_by_file_path(const QString& file_path) -> void
{
    if (m_model != nullptr)
    {
        m_model->remove_entries_by_file_path(file_path);
    }
}

/**
 * @brief Returns a copy of all entries currently stored in the model.
 * @return A `QVector<LogEntry>` with all current entries.
 */
auto LogViewContext::get_entries() const -> QVector<LogEntry>
{
    QVector<LogEntry> result;

    if (m_model != nullptr)
    {
        result = m_model->get_entries();
    }

    return result;
}

/**
 * @brief Sets the loaded-files list for this view.
 *
 * The provided list replaces any existing content.
 *
 * @param files List of `LogFileInfo` objects to set as loaded for this view.
 */
auto LogViewContext::set_loaded_files(const QList<LogFileInfo>& files) -> void
{
    m_loaded_files = files;
}

/**
 * @brief Adds a single file to the loaded-files list, avoiding duplicates by file path.
 * @param file_info The `LogFileInfo` to add.
 */
auto LogViewContext::add_loaded_file(const LogFileInfo& file_info) -> void
{
    bool exists = false;

    for (const auto& loaded_file: m_loaded_files)
    {
        if (loaded_file.get_file_path() == file_info.get_file_path())
        {
            exists = true;
        }
    }

    if (!exists)
    {
        m_loaded_files.append(file_info);
    }
}

/**
 * @brief Returns the list of loaded files for this view.
 * @return A copy of the `QList<LogFileInfo>` representing loaded files.
 */
auto LogViewContext::get_loaded_files() const -> QList<LogFileInfo>
{
    auto result = m_loaded_files;
    return result;
}

/**
 * @brief Returns all absolute file paths for the loaded files in this view.
 * @return A `QVector<QString>` of absolute file paths.
 */
auto LogViewContext::get_file_paths() const -> QVector<QString>
{
    QVector<QString> result;

    for (const auto& info: m_loaded_files)
    {
        result.append(info.get_file_path());
    }

    return result;
}

/**
 * @brief Clears the loaded-files list for this view.
 */
auto LogViewContext::clear_loaded_files() -> void
{
    m_loaded_files.clear();
}

/**
 * @brief Returns the filter state of this view.
 * @return Current filter and file-visibility state.
 */
auto LogViewContext::get_filter_state() const -> FilterState
{
    return m_filter_state;
}

/**
 * @brief Replaces the filter state of this view.
 * @param state New filter and file-visibility state.
 */
auto LogViewContext::set_filter_state(const FilterState& state) -> void
{
    m_filter_state = state;
}
