#pragma once

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @class DialogHeaderWidget
 * @brief Custom header widget for dialogs, including title and close button.
 *
 * This widget provides a customizable header for dialogs, featuring a title label
 * and a close button. The appearance should be styled via QSS. Emits a signal when
 * the close button is pressed.
 */
class DialogHeaderWidget: public QFrame
{
        Q_OBJECT
        Q_PROPERTY(QString close_icon READ close_icon WRITE set_close_icon)
        Q_PROPERTY(QColor close_icon_color READ close_icon_color WRITE set_close_icon_color)

    public:
        /**
         * @brief Returns the close icon resource path.
         * @return Current stylesheet property value.
         */
        [[nodiscard]] auto close_icon() const -> QString;

        /**
         * @brief Sets the close icon resource path.
         * @param value New stylesheet property value.
         */
        auto set_close_icon(const QString& value) -> void;

        /**
         * @brief Constructs a DialogHeaderWidget.
         * @param title The title to display in the header.
         * @param parent The parent widget.
         */
        explicit DialogHeaderWidget(const QString& title, QWidget* parent = nullptr);

        /**
         * @brief Returns the current title.
         * @return The current title text.
         */
        [[nodiscard]] auto get_title() const -> QString;

        /**
         * @brief Sets the title of the header.
         * @param title The new title to display.
         */
        auto set_title(const QString& title) -> void;

        /**
         * @brief Returns the color used to render the title-bar close SVG.
         * @return Current close-icon color.
         */
        [[nodiscard]] auto close_icon_color() const -> QColor;

        /**
         * @brief Recolors the close SVG to match the main window's title-bar theme.
         * @param color Icon color supplied by the stylesheet.
         */
        auto set_close_icon_color(const QColor& color) -> void;

    signals:
        /**
         * @brief Emitted when the close button is pressed.
         */
        void close_requested();

    private:
        QString m_close_icon;  ///< close icon resource path.
        QColor m_close_icon_color = Qt::white;
        QLabel* m_title_label;        ///< Label displaying the header title.
        QPushButton* m_close_button;  ///< Button to close the dialog.
        QHBoxLayout* m_layout;        ///< Layout for header elements.
};
