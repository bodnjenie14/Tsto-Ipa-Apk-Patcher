#include "std_include.hpp"
#include "style.hpp"


//me love qt due to using css for design :D 
namespace Style {

    // Light theme
    QString getLightStyleSheet() {
        return QString(R"(
            QMainWindow {
                background: #FEDE6E;  /* Softer Simpsons yellow */
            }

            QWidget#centralWidget {
                background: transparent;
            }

            #titleLabel {
                color: #FF7E00;  /* Homer's shirt orange */
                font-size: 32px;
                font-weight: bold;
                font-family: 'Arial Black', sans-serif;
                margin: 20px 0;
            }

            QPushButton {
                background: #009DDC;  /* Marge's hair blue */
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-weight: bold;
                font-size: 14px;
                min-height: 32px;
            }

            QPushButton:hover {
                background: #007AB3;
            }

            QPushButton:pressed {
                background: #006090;
            }

            QLineEdit {
                background-color: white;
                border: 2px solid #FF7E00;  /* Homer's shirt orange */
                border-radius: 6px;
                padding: 8px 12px;
                font-size: 13px;
                color: #333333;
            }

            QLineEdit:focus {
                border: 2px solid #FF0000;  /* Bart's shirt red */
            }

            QLineEdit::placeholder {
                color: #999999;
            }

            QGroupBox {
                background-color: #FFCF70;  /* Softer darker yellow */
                border: 2px solid #FF7E00;  /* Homer's shirt orange */
                border-radius: 8px;
                margin-top: 16px;
                padding: 16px;
                color: #663300;  /* Brown text */
            }

            QGroupBox::title {
                color: #FF0000;  /* Bart's shirt red */
                font-size: 14px;
                font-weight: bold;
                padding: 0 8px;
                subcontrol-origin: margin;
                subcontrol-position: top left;
                background: #FEDE6E;  /* Softer Simpsons yellow */
            }

            QLabel {
                color: #663300;  /* Brown text */
                font-size: 13px;
                font-weight: bold;
            }

            QProgressBar {
                border: 2px solid #FF7E00;  /* Homer's shirt orange */
                border-radius: 6px;
                text-align: center;
                background-color: white;
                color: #663300;
                font-weight: bold;
                height: 30px;
                margin: 15px 0;
                font-size: 16px;
            }

            QProgressBar::chunk {
                background-color: #009DDC;  /* Marge's hair blue */
                border-radius: 4px;
            }

            /* Status Label */
            #statusLabel {
                color: #FF0000;  /* Bart's shirt red */
                font-size: 14px;
                font-weight: bold;
                padding: 8px;
                background-color: #FFCF70;  /* Softer darker yellow */
                border-radius: 6px;
            }
        )");
    }

    // Dark theme
    QString getDarkStyleSheet() {
        return QString(R"(
            QMainWindow {
                background: #2D2D2D;  /* Dark background */
            }

            QWidget#centralWidget {
                background: transparent;
            }

            #titleLabel {
                color: #FF9E2C;  /* Brighter orange for dark mode */
                font-size: 32px;
                font-weight: bold;
                font-family: 'Arial Black', sans-serif;
                margin: 20px 0;
            }

            QPushButton {
                background: #0F84B5;  /* Darker blue */
                color: white;
                border: none;
                border-radius: 8px;
                padding: 8px 16px;
                font-weight: bold;
                font-size: 14px;
                min-height: 32px;
            }

            QPushButton:hover {
                background: #1A9FD9;
            }

            QPushButton:pressed {
                background: #0A6A94;
            }

            QLineEdit {
                background-color: #3D3D3D;
                border: 2px solid #FF9E2C;  /* Brighter orange */
                border-radius: 6px;
                padding: 8px 12px;
                font-size: 13px;
                color: #FFFFFF;
            }

            QLineEdit:focus {
                border: 2px solid #FF5252;  /* Bright red */
            }

            QLineEdit::placeholder {
                color: #AAAAAA;
            }

            QGroupBox {
                background-color: #3D3D3D;  /* Darker gray */
                border: 2px solid #FF9E2C;  /* Brighter orange */
                border-radius: 8px;
                margin-top: 16px;
                padding: 16px;
                color: #E0E0E0;  /* Light text */
            }

            QGroupBox::title {
                color: #FF5252;  /* Bright red */
                font-size: 14px;
                font-weight: bold;
                padding: 0 8px;
                subcontrol-origin: margin;
                subcontrol-position: top left;
                background: #2D2D2D;  /* Dark background */
            }

            QLabel {
                color: #E0E0E0;  /* Light text */
                font-size: 13px;
                font-weight: bold;
            }

            QProgressBar {
                border: 2px solid #FF9E2C;  /* Brighter orange */
                border-radius: 6px;
                text-align: center;
                background-color: #3D3D3D;
                color: #E0E0E0;
                font-weight: bold;
                height: 30px;
                margin: 15px 0;
                font-size: 16px;
            }

            QProgressBar::chunk {
                background-color: #0F84B5;  /* Darker blue */
                border-radius: 4px;
            }

            /* Status Label */
            #statusLabel {
                color: #FF5252;  /* Bright red */
                font-size: 14px;
                font-weight: bold;
                padding: 8px;
                background-color: #3D3D3D;  /* Darker gray */
                border-radius: 6px;
            }

            /* Scrollbar styling for dark mode */
            QScrollBar:vertical {
                border: none;
                background: #2D2D2D;
                width: 10px;
                margin: 0px;
            }

            QScrollBar::handle:vertical {
                background: #555555;
                min-height: 20px;
                border-radius: 5px;
            }

            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0px;
            }

            QScrollBar:horizontal {
                border: none;
                background: #2D2D2D;
                height: 10px;
                margin: 0px;
            }

            QScrollBar::handle:horizontal {
                background: #555555;
                min-width: 20px;
                border-radius: 5px;
            }

            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                width: 0px;
            }
        )");
    }

    QString getStyleSheet(bool darkMode) {
        if (darkMode) {
            return getDarkStyleSheet();
        } else {
            return getLightStyleSheet();
        }
    }

    QString getStyleSheet() {
        return getLightStyleSheet();
    }
}
