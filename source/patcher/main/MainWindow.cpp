#include "std_include.hpp"
#include "MainWindow.hpp"
#include "style.hpp"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , patcher(new Patcher::AppPatcher(this))
{
    setupUi();

    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(101));  
    if (hIcon) {
        SendMessage((HWND)this->winId(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage((HWND)this->winId(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

    connect(patcher, &Patcher::AppPatcher::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(patcher, &Patcher::AppPatcher::log, this, &MainWindow::onLogMessage);
    connect(patcher, &Patcher::AppPatcher::error, this, &MainWindow::onError);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    // Create central widget and layout
    auto* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);
    auto* mainLayout = new QVBoxLayout(centralWidget);

    // Title label
    auto* titleLabel = new QLabel("TSTO Patcher", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // File selection group
    auto* fileGroup = new QGroupBox("File Selection", this);
    auto* fileLayout = new QHBoxLayout(fileGroup);
    filePathEdit = new QLineEdit(this);
    browseButton = new QPushButton("Browse", this);
    fileLayout->addWidget(new QLabel("APK/IPA File:", this));
    fileLayout->addWidget(filePathEdit);
    fileLayout->addWidget(browseButton);
    mainLayout->addWidget(fileGroup);

    // Server URLs group
    auto* serverGroup = new QGroupBox("Server Configuration", this);
    auto* serverLayout = new QGridLayout(serverGroup);
    gameServerEdit = new QLineEdit(this);
    dlcServerEdit = new QLineEdit(this);
    serverLayout->addWidget(new QLabel("Game Server URL:", this), 0, 0);
    serverLayout->addWidget(gameServerEdit, 0, 1);
    serverLayout->addWidget(new QLabel("DLC Server URL:", this), 1, 0);
    serverLayout->addWidget(dlcServerEdit, 1, 1);
    mainLayout->addWidget(serverGroup);

    // Console output group
    auto* consoleGroup = new QGroupBox("Console Output", this);
    auto* consoleLayout = new QVBoxLayout(consoleGroup);
    consoleOutput = new QTextEdit(this);
    consoleOutput->setReadOnly(true);
    consoleOutput->setFont(QFont("Courier New", 10));
    consoleLayout->addWidget(consoleOutput);
    mainLayout->addWidget(consoleGroup);

    // Progress group
    auto* progressGroup = new QGroupBox("Progress", this);
    auto* progressLayout = new QVBoxLayout(progressGroup);
    progressBar = new QProgressBar(this);
    progressBar->setTextVisible(true);
    progressLayout->addWidget(progressBar);
    statusLabel = new QLabel("Ready", this);
    statusLabel->setObjectName("statusLabel");
    progressLayout->addWidget(statusLabel);
    mainLayout->addWidget(progressGroup);

    // Button layout
    auto* buttonLayout = new QHBoxLayout();
    checkDependenciesButton = new QPushButton("Check Dependencies", this);
    patchButton = new QPushButton("Patch File", this);
    buttonLayout->addWidget(checkDependenciesButton);
    buttonLayout->addWidget(patchButton);
    mainLayout->addLayout(buttonLayout);

    auto* settingsLayout = new QHBoxLayout();
    darkModeButton = new QPushButton(isDarkMode ? "Light Mode" : "Dark Mode", this);
    darkModeButton->setObjectName("darkModeButton");
    
    creditsButton = new QPushButton("Credits", this);
    creditsButton->setObjectName("creditsButton");
    
    settingsLayout->addWidget(darkModeButton);
    settingsLayout->addWidget(creditsButton);
    mainLayout->addLayout(settingsLayout);

    setWindowTitle("TSTO Patcher");
    resize(800, 600);

    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    connect(patchButton, &QPushButton::clicked, this, &MainWindow::onPatchClicked);
    connect(checkDependenciesButton, &QPushButton::clicked, this, &MainWindow::onCheckDependenciesClicked);
    connect(darkModeButton, &QPushButton::clicked, this, &MainWindow::onDarkModeToggled);
    connect(creditsButton, &QPushButton::clicked, this, &MainWindow::onCreditsClicked);

    applyTheme(isDarkMode);
}

void MainWindow::applyTheme(bool darkMode)
{
    isDarkMode = darkMode;
    darkModeButton->setText(isDarkMode ? "Light Mode" : "Dark Mode");
    setStyleSheet(Style::getStyleSheet(darkMode));
}

void MainWindow::onDarkModeToggled()
{
    applyTheme(!isDarkMode);
}

void MainWindow::onCreditsClicked()
{
    showCreditsDialog();
}

void MainWindow::showCreditsDialog()
{
    QMessageBox creditsBox(this);
    creditsBox.setWindowTitle("Credits");
    creditsBox.setIcon(QMessageBox::Information);

    QString creditsText = "<div style='background-color: #333; color: #f0f0f0; padding: 15px; border-radius: 5px;'>";
    
    creditsText += "<h2 style='color: #FFC107; text-align: center; margin-top: 0;'>TSTO Patcher Credits</h2>";
    
    creditsText += "<div style='border-bottom: 1px solid #555; margin: 10px 0;'></div>";
    
    creditsText += "<div style='margin: 15px 0;'>";
    creditsText += "<p style='font-weight: bold; color: #03A9F4; margin-bottom: 5px;'>IPA Patching Code</p>";
    creditsText += "<p style='margin-left: 15px; margin-top: 0;'>damar1st</p>";
    creditsText += "</div>";
    
    creditsText += "<div style='border-bottom: 1px solid #555; margin: 10px 0;'></div>";
    
    creditsText += "<div style='margin: 15px 0;'>";
    creditsText += "<p style='font-weight: bold; color: #03A9F4; margin-bottom: 5px;'>Testers</p>";
    creditsText += "<p style='margin-left: 15px; margin-top: 0;'>Rudeboy<br>Tapper<br>Avariss</p>";
    creditsText += "</div>";
    
    creditsText += "<div style='border-bottom: 1px solid #555; margin: 10px 0;'></div>";
    
    creditsText += "<p style='text-align: center; color: #FFC107; margin-top: 15px;'>Thank you for your contributions!</p>";
    
    creditsText += "</div>";

    creditsBox.setText(creditsText);
    creditsBox.setTextFormat(Qt::RichText);
    creditsBox.setStandardButtons(QMessageBox::Ok);
    
    QList<QAbstractButton*> buttons = creditsBox.buttons();
    for (QAbstractButton* button : buttons) {
        if (creditsBox.buttonRole(button) == QMessageBox::AcceptRole) {
            button->setStyleSheet(
                "QPushButton {"
                "  background-color: #0099cc;"
                "  color: white;"
                "  border: none;"
                "  padding: 5px 15px;"
                "  border-radius: 3px;"
                "}"
                "QPushButton:hover {"
                "  background-color: #00aadd;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #0088bb;"
                "}"
            );
        }
    }

    creditsBox.setMinimumWidth(350);
    
    creditsBox.exec();
}

void MainWindow::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select APK/IPA File",
        QString(),
        "Game Files (*.apk *.ipa)"
    );

    if (!filePath.isEmpty()) {
        filePathEdit->setText(filePath);
    }
}

void MainWindow::onPatchClicked()
{
    QString filePath = filePathEdit->text();
    QString gameServerUrl = gameServerEdit->text();
    QString dlcServerUrl = dlcServerEdit->text();

    if (filePath.isEmpty() || gameServerUrl.isEmpty() || dlcServerUrl.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill in all fields");
        return;
    }

    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "Error", "File not found: " + filePath);
        return;
    }

    consoleOutput->clear();
    progressBar->setValue(0);
    statusLabel->setText("Starting patch process...");
    patchButton->setEnabled(false);
    checkDependenciesButton->setEnabled(false);

    QString extension = QFileInfo(filePath).suffix().toLower();
    if (extension == "apk") {
        patcher->patchAPK(filePath, gameServerUrl, dlcServerUrl);
    } else if (extension == "ipa") {
        patcher->patchIPA(filePath, gameServerUrl, dlcServerUrl);
    } else {
        QMessageBox::warning(this, "Error", "Unsupported file type. Please select an APK or IPA file.");
        patchButton->setEnabled(true);
        checkDependenciesButton->setEnabled(true);
    }
}

void MainWindow::onCheckDependenciesClicked()
{
    consoleOutput->clear();
    progressBar->setValue(0);
    statusLabel->setText("Checking dependencies...");
    patchButton->setEnabled(false);
    checkDependenciesButton->setEnabled(false);

    if (patcher->checkDependencies()) {
        patchButton->setEnabled(true);
    }
    checkDependenciesButton->setEnabled(true);
}

void MainWindow::onProgressUpdated(int progress, const QString& status)
{
    progressBar->setValue(progress);
    statusLabel->setText(status);

    if (progress == 100) {
        patchButton->setEnabled(true);
        checkDependenciesButton->setEnabled(true);
    }
}

void MainWindow::onLogMessage(const QString& message)
{
    consoleOutput->append(message);
}

void MainWindow::onError(const QString& message)
{
    QMessageBox::critical(this, "Error", message);
    patchButton->setEnabled(true);
    checkDependenciesButton->setEnabled(true);
}
