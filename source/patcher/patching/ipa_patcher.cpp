#include "std_include.hpp"
#include "ipa_patcher.hpp"
#include <filesystem>


namespace Patcher {

class IPAPatcherPrivate {
public:
    IPAPatcher* q;
    QString gameServerUrl;
    QString dlcServerUrl;

    explicit IPAPatcherPrivate(IPAPatcher* patcher) : q(patcher) {}

    bool decompileApp(const QString& inputFile);
    bool recompileApp(const QString& inputFile);
    bool replaceUrls(const QString& gameServerUrl, const QString& dlcServerUrl);
    bool updatePlist(const QString& plistPath);
    bool updateBinary(const QString& binaryPath);
    bool patchIPA(const QString& ipaPath, const QString& gameServerUrl, const QString& dlcServerUrl);
};

IPAPatcher::IPAPatcher(QObject* parent)
    : QObject(parent)
    , d(new IPAPatcherPrivate(this))
{
}

IPAPatcher::~IPAPatcher()
{
    delete d;
}

bool IPAPatcher::checkDependencies()
{
    emit progressUpdated(0, "Checking dependencies...");
    emit log("Checking for required dependencies...");

    QProcess powershellCheck;
    powershellCheck.start("powershell", {"-Command", "echo 'PowerShell check'"});
    
    if (!powershellCheck.waitForStarted(5000)) {
        emit error("PowerShell not found or could not be started");
        emit log("ERROR: PowerShell not found or could not be started");
        return false;
    }
    
    if (!powershellCheck.waitForFinished(5000)) {
        powershellCheck.kill();
        emit error("PowerShell check timed out");
        emit log("ERROR: PowerShell check timed out");
        return false;
    }
    
    if (powershellCheck.exitCode() != 0) {
        emit error("PowerShell check failed with exit code: " + QString::number(powershellCheck.exitCode()));
        emit log("ERROR: PowerShell check failed with exit code: " + QString::number(powershellCheck.exitCode()));
        return false;
    }
    
    emit log("PowerShell check passed successfully");
    emit progressUpdated(100, "Dependencies verified successfully!");
    emit log("All dependencies verified successfully!");
    return true;
}

bool IPAPatcherPrivate::updatePlist(const QString& plistPath)
{
    q->emit log("Updating Info.plist...");

    QFile file(plistPath);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        q->emit error("Failed to open Info.plist");
        return false;
    }

    QString content = file.readAll();

    QRegularExpression serverRegex("<key>MayhemServerURL</key>\\s*<string>(.*?)</string>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch serverMatch = serverRegex.match(content);
    
    QString newServerUrl = gameServerUrl.trimmed();
    if (newServerUrl.endsWith('/')) {
        newServerUrl.chop(1);
    }
    
    if (serverMatch.hasMatch()) {
        content.replace(serverMatch.captured(0), 
            QString("<key>MayhemServerURL</key><string>%1</string>").arg(newServerUrl));
        q->emit log("Updated MayhemServerURL: " + newServerUrl);
    } else {
        q->emit log("Key 'MayhemServerURL' not found.");
        
        QRegularExpression versionRegex("<key>CFBundleVersion</key>\\s*<string>(.*?)</string>");
        QRegularExpressionMatch versionMatch = versionRegex.match(content);
        if (versionMatch.hasMatch()) {
            QString insertText = versionMatch.captured(0) + 
                QString("\n\t<key>MayhemServerURL</key>\n\t<string>%1</string>").arg(newServerUrl);
            content.replace(versionMatch.captured(0), insertText);
            q->emit log("Added MayhemServerURL: " + newServerUrl);
        }
    }

    QRegularExpression dlcRegex("<key>DLCLocation</key>\\s*<string>(.*?)</string>", QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch dlcMatch = dlcRegex.match(content);
    
    QString newDlcUrl = dlcServerUrl.trimmed();
    if (newDlcUrl.endsWith('/')) {
        newDlcUrl.chop(1);
    }
    newDlcUrl += "/static/";
    
    if (dlcMatch.hasMatch()) {
        content.replace(dlcMatch.captured(0), 
            QString("<key>DLCLocation</key><string>%1</string>").arg(newDlcUrl));
        q->emit log("Updated DLCLocation: " + newDlcUrl);
    } else {
        q->emit log("Key 'DLCLocation' not found.");
        
        QRegularExpression mayhemRegex("<key>MayhemServerURL</key>\\s*<string>(.*?)</string>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch mayhemMatch = mayhemRegex.match(content);
        if (mayhemMatch.hasMatch()) {
            QString insertText = mayhemMatch.captured(0) + 
                QString("\n\t<key>DLCLocation</key>\n\t<string>%1</string>").arg(newDlcUrl);
            content.replace(mayhemMatch.captured(0), insertText);
            q->emit log("Added DLCLocation: " + newDlcUrl);
        }
    }

    file.seek(0);
    file.write(content.toUtf8());
    file.resize(file.pos());
    file.close();

    return true;
}

bool IPAPatcherPrivate::updateBinary(const QString& binaryPath) {
    q->emit log("Updating binary file...");

    QFile file(binaryPath);
    if (!file.open(QIODevice::ReadWrite)) {
        q->emit error("Failed to open binary file: " + binaryPath);
        return false;
    }

    QByteArray content = file.readAll();
    
    q->emit log("\n=== Binary Patching Summary ===");
    q->emit log("Binary file: " + binaryPath);
    q->emit log("File size: " + QString::number(content.size()) + " bytes");
    
    QList<QByteArray> oldUrls = {
        "http://oct2018-4-35-0-uam5h44a.tstodlc.eamobile.com/netstorage/gameasset/direct/simpsons/",
        "https://syn-dir.sn.eamobile.com"
    };
    
    QString newDlcUrl = dlcServerUrl.trimmed();
    if (newDlcUrl.endsWith('/')) {
        newDlcUrl.chop(1);
    }
    newDlcUrl += "/static/";
    
    QString newGameUrl = gameServerUrl.trimmed();
    
    QList<QByteArray> newUrls = {
        newDlcUrl.toUtf8(),
        newGameUrl.toUtf8()
    };
    
    //binary replacements
    for (int i = 0; i < oldUrls.size(); i++) {
        QByteArray oldUrlBytes = oldUrls[i];
        QByteArray newUrlBytes = newUrls[i];
        
        q->emit log(QString("\nURL %1:").arg(i + 1));
        q->emit log("  Old URL: " + QString::fromUtf8(oldUrlBytes));
        q->emit log("  Old URL length: " + QString::number(oldUrlBytes.length()) + " bytes");
        q->emit log("  New URL: " + QString::fromUtf8(newUrlBytes));
        q->emit log("  New URL length: " + QString::number(newUrlBytes.length()) + " bytes");
        
        // Check URL lengths and pad if necessary
        if (newUrlBytes.length() < oldUrlBytes.length()) {
            q->emit log("  Padding new URL with '/' characters");
            // Pad the new URL with '/' to match length (like in Python)
            while (newUrlBytes.length() < oldUrlBytes.length()) {
                newUrlBytes.append('/');
            }
            q->emit log("  Padded URL: " + QString::fromUtf8(newUrlBytes));
            q->emit log("  Final URL length: " + QString::number(newUrlBytes.length()) + " bytes");
        } else if (newUrlBytes.length() > oldUrlBytes.length()) {
            q->emit log("  ERROR: New URL is longer than old URL by " + 
                      QString::number(newUrlBytes.length() - oldUrlBytes.length()) + " bytes");
            q->emit error("New URL is too long: " + QString::fromUtf8(newUrlBytes));
            file.close();
            return false;
        }
        
        if (content.contains(oldUrlBytes)) {
            content.replace(oldUrlBytes, newUrlBytes);
            q->emit log("  Successfully replaced URL in binary");
        } else {
            q->emit log("  URL not found in binary");
        }
    }
    
    file.seek(0);
    file.write(content);
    file.resize(file.pos());
    file.close();
    
    q->emit log("Binary file updated successfully");
    return true;
}

bool IPAPatcherPrivate::decompileApp(const QString& inputFile)
{
    q->emit log("Decompiling IPA...");
    
    QDir dir("decipa");
    if (dir.exists()) {
        dir.removeRecursively();
    }
    QDir().mkdir("decipa");

    QString tempZipPath = QDir::temp().filePath("temp_ipa.zip");
    q->emit log("Creating temporary zip at: " + tempZipPath);
    
    if (QFile::exists(tempZipPath)) {
        QFile::remove(tempZipPath);
    }

    if (!QFile::copy(inputFile, tempZipPath)) {
        q->emit error("Failed to create temporary zip file");
        return false;
    }

    QProcess process;
    process.setWorkingDirectory(QDir::currentPath());
    process.setProgram("powershell");
    process.setArguments(QStringList() 
        << "-NoProfile"
        << "-Command"
        << QString("$ErrorActionPreference = 'Stop'; Expand-Archive -LiteralPath '%1' -DestinationPath decipa -Force")
            .arg(QDir::toNativeSeparators(tempZipPath))
    );

    q->emit log("\nExecuting command: " + process.program() + " " + process.arguments().join(" "));
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();

    if (!process.waitForStarted(30000)) {
        q->emit error("Failed to start IPA decompilation");
        QFile::remove(tempZipPath);
        return false;
    }

    while (process.state() != QProcess::NotRunning) {
        if (process.waitForReadyRead(1000)) {
            QString output = QString::fromUtf8(process.readAll()).trimmed();
            if (!output.isEmpty()) {
                q->emit log(output);
            }
        }
    }

    process.waitForFinished();
    
    QFile::remove(tempZipPath);

    if (process.exitCode() != 0) {
        q->emit error("IPA decompilation failed");
        return false;
    }

    q->emit log("IPA decompiled successfully");
    return true;
}

bool IPAPatcherPrivate::recompileApp(const QString& inputFile)
{
    q->emit log("Recompiling IPA...");

    QFileInfo fi(inputFile);
    QString outputName = fi.baseName() + "-patched.ipa";

    QString tempZipPath = QDir::temp().filePath("temp_ipa.zip");
    
    if (QFile::exists(tempZipPath)) {
        QFile::remove(tempZipPath);
    }
    if (QFile::exists(outputName)) {
        QFile::remove(outputName);
    }

    QProcess process;
    process.setWorkingDirectory(QDir::currentPath());
    process.setProgram("powershell");
    process.setArguments(QStringList() 
        << "-NoProfile"
        << "-Command"
        << QString("$ErrorActionPreference = 'Stop'; Compress-Archive -Path 'decipa/*' -DestinationPath '%1' -Force; Move-Item -Path '%1' -Destination '%2' -Force")
            .arg(QDir::toNativeSeparators(tempZipPath))
            .arg(QDir::toNativeSeparators(outputName))
    );

    q->emit log("\nExecuting command: " + process.program() + " " + process.arguments().join(" "));
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();

    if (!process.waitForStarted(30000)) {
        q->emit error("Failed to start IPA recompilation");
        return false;
    }

    while (process.state() != QProcess::NotRunning) {
        if (process.waitForReadyRead(1000)) {
            QString output = QString::fromUtf8(process.readAll()).trimmed();
            if (!output.isEmpty()) {
                q->emit log(output);
            }
        }
    }

    process.waitForFinished();
    
    QFile::remove(tempZipPath);

    if (process.exitCode() != 0) {
        q->emit error("IPA recompilation failed");
        return false;
    }

    q->emit log("IPA recompiled successfully");
    return true;
}

bool IPAPatcherPrivate::replaceUrls(const QString& gameServerUrl, const QString& dlcServerUrl)
{
    this->gameServerUrl = gameServerUrl;
    this->dlcServerUrl = dlcServerUrl;

    QDirIterator it("decipa", QDir::Dirs | QDir::NoDotAndDotDot);
    QString payloadPath;
    while (it.hasNext()) {
        QString dirPath = it.next();
        if (dirPath.endsWith("Payload", Qt::CaseInsensitive)) {
            payloadPath = dirPath;
            break;
        }
    }

    if (payloadPath.isEmpty()) {
        q->emit error("Payload directory not found in IPA");
        return false;
    }

    QDirIterator appIt(payloadPath, QDir::Dirs | QDir::NoDotAndDotDot);
    QString appPath;
    while (appIt.hasNext()) {
        QString dirPath = appIt.next();
        if (dirPath.endsWith(".app", Qt::CaseInsensitive)) {
            appPath = dirPath;
            break;
        }
    }

    if (appPath.isEmpty()) {
        q->emit error(".app directory not found in IPA");
        return false;
    }

    QString plistPath = appPath + "/Info.plist";
    if (!updatePlist(plistPath)) {
        return false;
    }

    QFileInfo appInfo(appPath);
    QString binaryPath = appPath + "/" + appInfo.baseName();
    if (!updateBinary(binaryPath)) {
        return false;
    }

    return true;
}

bool IPAPatcherPrivate::patchIPA(const QString& ipaPath, const QString& gameServerUrl, const QString& dlcServerUrl)
{
    this->gameServerUrl = gameServerUrl;
    this->dlcServerUrl = dlcServerUrl;

    q->emit progressUpdated(0, "Starting IPA patching process...");
    q->emit log("Starting IPA patching process...");
    q->emit log("IPA Path: " + ipaPath);
    q->emit log("Game Server: " + gameServerUrl);
    q->emit log("DLC Server: " + dlcServerUrl);

    q->emit progressUpdated(10, "Decompiling IPA...");
    if (!decompileApp(ipaPath)) {
        return false;
    }


    QDirIterator it("decipa", QDir::Dirs | QDir::NoDotAndDotDot);
    QString payloadPath;
    while (it.hasNext()) {
        QString dirPath = it.next();
        if (dirPath.endsWith("Payload", Qt::CaseInsensitive)) {
            payloadPath = dirPath;
            break;
        }
    }

    if (payloadPath.isEmpty()) {
        q->emit error("Payload directory not found in IPA");
        return false;
    }

    QDirIterator appIt(payloadPath, QDir::Dirs | QDir::NoDotAndDotDot);
    QString appPath;
    while (appIt.hasNext()) {
        QString dirPath = appIt.next();
        if (dirPath.endsWith(".app", Qt::CaseInsensitive)) {
            appPath = dirPath;
            break;
        }
    }

    if (appPath.isEmpty()) {
        q->emit error(".app directory not found in IPA");
        return false;
    }

    q->emit progressUpdated(40, "Updating Info.plist...");
    QString plistPath = appPath + "/Info.plist";
    if (!QFile::exists(plistPath)) {
        q->emit error("Info.plist not found at: " + plistPath);
        return false;
    }
    if (!updatePlist(plistPath)) {
        return false;
    }

    q->emit progressUpdated(60, "Updating binary...");
    QFileInfo appInfo(appPath);
    QString binaryPath = appPath + "/" + appInfo.baseName();
    if (!QFile::exists(binaryPath)) {
        q->emit error("Binary not found at: " + binaryPath);
        return false;
    }
    if (!updateBinary(binaryPath)) {
        return false;
    }

    q->emit progressUpdated(80, "Recompiling IPA...");
    if (!recompileApp(ipaPath)) {
        return false;
    }

    q->emit progressUpdated(100, "IPA patching completed successfully!");
    q->emit log("IPA patching completed successfully");
    return true;
}

void IPAPatcher::patchIPA(const QString& ipaPath, const QString& gameServerUrl, const QString& dlcServerUrl)
{
    d->patchIPA(ipaPath, gameServerUrl, dlcServerUrl);
}

} 
