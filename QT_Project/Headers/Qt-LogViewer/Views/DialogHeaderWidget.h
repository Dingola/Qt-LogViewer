#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

/**
 * @class DialogHeaderWidget
 * @brief Custom header widget for dialogs, including title and close button.
 *
 * This widget provides a customizable header for dialogs, featuring a title label
 * and a close button. The appearance should be styled via QSS. Emits a signal when
 * the close button is pressed.
 */
class DialogHeaderWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a DialogHeaderWidget.
         * @param title The title to display in the header.
         * @param parent The parent widget.
         */
        explicit DialogHeaderWidget(const QString& title, QWidget* parent = nullptr);

        /**
         * @brief Sets the title of the header.
         * @param title The new title to display.
         */
        auto set_title(const QString& title) -> void;

    signals:
        /**
         * @brief Emitted when the close button is pressed.
         */
        void close_requested();

    private:
        QLabel* m_title_label;        ///< Label displaying the header title.
        QPushButton* m_close_button;  ///< Button to close the dialog.
        QHBoxLayout* m_layout;        ///< Layout for header elements.
};
