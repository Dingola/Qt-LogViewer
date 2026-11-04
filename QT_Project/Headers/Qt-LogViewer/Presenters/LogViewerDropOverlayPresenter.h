#pragma once

#include <QObject>

class DropOverlayWidget;
struct LogViewerDropValidationResult;

/**
 * @file LogViewerDropOverlayPresenter.h
 * @brief Declares Qt-LogViewer-specific content mapping for the reusable drop
 * overlay.
 */

/**
 * @brief Identifies the session that will receive accepted log files.
 */
enum class LogViewerDropDestination
{
    NewSession,
    CurrentSession
};

/**
 * @brief Describes the concrete application destination under the drag pointer.
 */
struct LogViewerDropTarget {
        /** @brief Whether the drop creates a session or extends the current one. */
        LogViewerDropDestination destination{LogViewerDropDestination::NewSession};
};

/**
 * @class LogViewerDropOverlayPresenter
 * @brief Maps validated log and session drops onto generic overlay
 * presentations.
 */
class LogViewerDropOverlayPresenter: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the presenter for one reusable overlay.
         * @param overlay Presentation component controlled by this presenter.
         * @param parent Optional QObject parent.
         */
        explicit LogViewerDropOverlayPresenter(DropOverlayWidget& overlay,
                                               QObject* parent = nullptr);

        /**
         * @brief Presents validation results and their effective application
         * destination.
         * @param validation Validated and classified dragged files.
         * @param target Session destination selected from current application state.
         */
        auto show(const LogViewerDropValidationResult& validation,
                  const LogViewerDropTarget& target) -> void;

        /** @brief Hides the controlled overlay. */
        auto hide() -> void;

    private:
        DropOverlayWidget& m_overlay;
};
