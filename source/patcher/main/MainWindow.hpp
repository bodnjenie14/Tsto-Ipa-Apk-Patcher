#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QLabel>
#include "patching/patcher.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onBrowseClicked();
    void onPatchClicked();
    void onCheckDependenciesClicked();
    void onProgressUpdated(int progress, const QString& status);
    void onLogMessage(const QString& message);
    void onError(const QString& message);
    void onDarkModeToggled();
    void onCreditsClicked();

private:
    void setupUi();
    void applyTheme(bool darkMode);
    void showCreditsDialog();

    QLineEdit* filePathEdit;
    QLineEdit* gameServerEdit;
    QLineEdit* dlcServerEdit;
    QPushButton* browseButton;
    QPushButton* patchButton;
    QPushButton* checkDependenciesButton;
    QPushButton* darkModeButton;
    QPushButton* creditsButton;
    QProgressBar* progressBar;
    QTextEdit* consoleOutput;
    QLabel* statusLabel;
    
    Patcher::AppPatcher* patcher;
    bool isDarkMode = false;
};
