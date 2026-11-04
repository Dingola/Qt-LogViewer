#include "Qt-LogViewer/Presenters/LogViewerDropOverlayPresenter.h"

#include <QApplication>
#include <QIcon>
#include <QPalette>
#include <QStyle>

#include "Qt-LogViewer/Services/LogViewerDropValidator.h"
#include "Qt-LogViewer/Views/Shared/DropOverlayWidget.h"

/**
 * @file LogViewerDropOverlayPresenter.cpp
 * @brief Implements Qt-LogViewer-specific content mapping for the reusable drop
 * overlay.
 */

/**
 * @brief Constructs the presenter for one reusable overlay.
 * @param overlay Presentation component controlled by this presenter.
 * @param parent Optional QObject parent.
 */
LogViewerDropOverlayPresenter::LogViewerDropOverlayPresenter(DropOverlayWidget& overlay,
                                                             QObject* parent)
    : QObject(parent), m_overlay(overlay)
{}

/**
 * @brief Presents validation results and their effective application
 * destination.
 * @param validation Validated and classified dragged files.
 * @param target Session destination selected from current application state.
 */
auto LogViewerDropOverlayPresenter::show(const LogViewerDropValidationResult& validation,
                                         const LogViewerDropTarget& target) -> void
{
    DropOverlayPresentation presentation;
    const int accepted_count = validation.accepted_paths.size();
    const int rejected_count = validation.rejected_entries.size();

    if (validation.content_type == LogViewerDropContentType::SessionFile)
    {
        presentation.icon = QIcon(QStringLiteral(":/Resources/Icons/session.svg"));
        presentation.icon_tint = m_overlay.palette().color(QPalette::WindowText);
        presentation.title = tr("Open session");
        presentation.description = tr("Replace the current workspace with this saved session.");
        presentation.state =
            rejected_count > 0 ? DropOverlayVisualState::Warning : DropOverlayVisualState::Accepted;
    }
    else if (validation.content_type == LogViewerDropContentType::LogFiles)
    {
        presentation.icon = QIcon(QStringLiteral(":/Resources/Icons/file.svg"));
        presentation.icon_tint = m_overlay.palette().color(QPalette::WindowText);
        presentation.title = accepted_count == 1 ? tr("Add one log file")
                                                 : tr("Add %n log files", nullptr, accepted_count);
        if (target.destination == LogViewerDropDestination::CurrentSession)
        {
            presentation.description = tr("Add to the current session.");
        }
        else if (accepted_count == 1)
        {
            presentation.description = tr("Create a new session and add the log file.");
        }
        else
        {
            presentation.description = tr("Create a new session and add the log files.");
        }
        presentation.detail = tr("Import into a log view remains a separate step.");
        presentation.state =
            rejected_count > 0 ? DropOverlayVisualState::Warning : DropOverlayVisualState::Accepted;
    }
    else
    {
        presentation.icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
        presentation.title = tr("These items cannot be dropped here");
        presentation.description = tr("Use readable local log files or one valid session file.");
        presentation.state = DropOverlayVisualState::Rejected;
    }

    if (rejected_count > 0)
    {
        if (!presentation.detail.isEmpty())
        {
            presentation.detail.append(QLatin1Char('\n'));
        }
        presentation.detail.append(
            tr("%n invalid, unsupported, duplicate, or loaded item(s) will be "
               "skipped.",
               nullptr, rejected_count));
    }

    m_overlay.set_presentation(presentation);
    m_overlay.show_overlay();
}

/** @brief Hides the controlled overlay. */
auto LogViewerDropOverlayPresenter::hide() -> void
{
    m_overlay.hide_overlay();
}
