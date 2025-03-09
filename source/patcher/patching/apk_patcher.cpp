#include "std_include.hpp"
#include "apk_patcher.hpp"
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QDirIterator>
#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>
#include <filesystem>

namespace Patcher {

struct APKPatcherPrivate {
    APKPatcher* q;
    QString gameServerUrl;
    QString dlcServerUrl;

    explicit APKPatcherPrivate(APKPatcher* patcher) : q(patcher) {}

    bool decompileApp(const QString& inputFile);
    bool recompileApp(const QString& inputFile);
    bool replaceUrls(const QString& gameServerUrl, const QString& dlcServerUrl);
    bool patchAPK(const QString& apkPath, const QString& newGameServerUrl, const QString& newDlcServerUrl);
};

APKPatcher::APKPatcher(QObject* parent)
    : QObject(parent)
    , d(new APKPatcherPrivate(this))
{
}

APKPatcher::~APKPatcher()
{
    delete d;
}

bool APKPatcher::checkDependencies()
{
    emit progressUpdated(0, "Checking dependencies...");
    emit log("Checking for required dependencies...");

    QDir apktoolDir("sdktools/apktool");
    QString apktoolJar;
    QStringList jarFiles = apktoolDir.entryList({"*.jar"});
    
    if (jarFiles.isEmpty()) {
        QDir buildApktoolDir("build/sdktools/apktool");
        jarFiles = buildApktoolDir.entryList({"*.jar"});
        
        if (!jarFiles.isEmpty()) {
            apktoolJar = buildApktoolDir.absoluteFilePath(jarFiles.first());
        }
    } else {
        apktoolJar = apktoolDir.absoluteFilePath(jarFiles.first());
    }
    
    if (apktoolJar.isEmpty()) {
        emit error("apktool.jar not found in sdktools/apktool or build/sdktools/apktool directory");
        emit log("ERROR: apktool.jar not found in sdktools/apktool or build/sdktools/apktool directory");
        return false;
    }
    
    emit log("Found apktool.jar at: " + apktoolJar);

    QString apktoolBat = "sdktools/apktool/apktool.bat";
    if (!QFile::exists(apktoolBat)) {
        apktoolBat = "build/sdktools/apktool/apktool.bat";
        if (!QFile::exists(apktoolBat)) {
            emit error("apktool.bat not found in sdktools/apktool or build/sdktools/apktool directory");
            emit log("ERROR: apktool.bat not found in sdktools/apktool or build/sdktools/apktool directory");
            return false;
        }
    }
    
    emit log("Found apktool.bat at: " + apktoolBat);

    QString debugKeystore = "sdktools/debug.keystore";
    if (!QFile::exists(debugKeystore)) {
        debugKeystore = "build/sdktools/debug.keystore";
        if (!QFile::exists(debugKeystore)) {
            emit error("debug.keystore not found in sdktools/ or build/sdktools/ directory");
            emit log("ERROR: debug.keystore not found in sdktools/ or build/sdktools/ directory");
            return false;
        }
    }
    
    emit log("Found debug.keystore at: " + debugKeystore);

    QProcess javaCheck;
    javaCheck.start("java", QStringList() << "-version");
    if (!javaCheck.waitForStarted(5000) || !javaCheck.waitForFinished(5000)) {
        emit error("Java not found. Please install Java SDK (version 11 or higher)");
        emit log("ERROR: Java not found. Please install Java SDK (version 11 or higher)");
        return false;
    }

    QString javaOutput = QString::fromUtf8(javaCheck.readAllStandardError());
    QRegularExpression versionRegex(QStringLiteral("version \"([\\d.]+)"));
    QRegularExpressionMatch match = versionRegex.match(javaOutput);
    
    if (match.hasMatch()) {
        QString version = match.captured(1);
        emit log("Found Java version: " + version);
        
        QStringList versionParts = version.split('.');
        if (!versionParts.isEmpty()) {
            bool ok;
            int majorVersion = versionParts[0].toInt(&ok);
            if (ok && majorVersion < 11) {
                emit error("Java version too old. Please install Java SDK 11 or higher");
                emit log("ERROR: Java version too old. Please install Java SDK 11 or higher");
                return false;
            }
        }
    } else {
        emit error("Could not determine Java version. Please ensure Java SDK 11 or higher is installed");
        emit log("ERROR: Could not determine Java version. Please ensure Java SDK 11 or higher is installed");
        return false;
    }

    QByteArray javaHomeBytes = qgetenv("JAVA_HOME");
    QString javaHome = QString::fromLocal8Bit(javaHomeBytes);
    if (javaHome.isEmpty()) {
        emit log("WARNING: JAVA_HOME environment variable is not set");
        emit log("It's recommended to set JAVA_HOME to your Java SDK installation directory");
    } else {
        emit log("JAVA_HOME is set to: " + javaHome);
    }

    QProcess jarsignerCheck;
    jarsignerCheck.start("jarsigner", QStringList() << "-help");
    if (!jarsignerCheck.waitForStarted(5000)) {
        emit log("WARNING: jarsigner not found in PATH. Will attempt to find it in other locations.");
        
        QString jarsignerPath;
        if (!javaHome.isEmpty()) {
            jarsignerPath = javaHome + "/bin/jarsigner.exe";
            if (QFile::exists(jarsignerPath)) {
                emit log("Found jarsigner at: " + jarsignerPath);
            } else {
                jarsignerPath.clear();
            }
        }
        
        if (jarsignerPath.isEmpty()) {
            QProcess whereJava;
            whereJava.start("where", QStringList() << "java");
            if (whereJava.waitForStarted(5000) && whereJava.waitForFinished(5000)) {
                QString javaPath = QString::fromLocal8Bit(whereJava.readAllStandardOutput()).trimmed();
                if (!javaPath.isEmpty()) {
                    QFileInfo javaFileInfo(javaPath);
                    QString javaBinDir = javaFileInfo.absolutePath();
                    jarsignerPath = javaBinDir + "/jarsigner.exe";
                    if (QFile::exists(jarsignerPath)) {
                        emit log("Found jarsigner at: " + jarsignerPath);
                    } else {
                        jarsignerPath.clear();
                    }
                }
            }
        }
        
        if (jarsignerPath.isEmpty()) {
            QStringList commonJavaPaths = {
                "C:/Program Files/Java/jdk*/bin/jarsigner.exe",
                "C:/Program Files (x86)/Java/jdk*/bin/jarsigner.exe"
            };
            
            for (const QString& pathPattern : commonJavaPaths) {
                QStringList matches;
                QStringList parts = pathPattern.split('*');
                if (parts.size() == 2) {
                    QString basePath = parts[0];
                    QString endPath = parts[1];
                    QDir baseDir(basePath);
                    QStringList entries = baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    for (const QString& entry : entries) {
                        QString fullPath = basePath + entry + endPath;
                        if (QFile::exists(fullPath)) {
                            matches.append(fullPath);
                        }
                    }
                }
                
                if (!matches.isEmpty()) {
                    jarsignerPath = matches.first();
                    emit log("Found jarsigner at: " + jarsignerPath);
                    break;
                }
            }
        }
        

        if (jarsignerPath.isEmpty()) {
            jarsignerPath = "sdktools/jarsigner.exe";
            if (!QFile::exists(jarsignerPath)) {
                jarsignerPath = "build/sdktools/jarsigner.exe";
                if (!QFile::exists(jarsignerPath)) {
                    emit log("WARNING: jarsigner not found. APK signing might fail.");
                } else {
                    emit log("Found bundled jarsigner at: " + jarsignerPath);
                }
            } else {
                emit log("Found bundled jarsigner at: " + jarsignerPath);
            }
        }
    } else {
        jarsignerCheck.waitForFinished(5000);
        emit log("jarsigner found in PATH");
    }

    emit progressUpdated(100, "Dependencies verified successfully!");
    emit log("All dependencies verified successfully!");
    return true;
}

bool APKPatcherPrivate::replaceUrls(const QString& gameServerUrl, const QString& dlcServerUrl)
{
    q->emit log("\n=== URL Replacement Summary ===");
    q->emit log("Game Server URL: " + gameServerUrl);
    q->emit log("DLC Server URL: " + dlcServerUrl);
    
    QStringList textExtensions = {".xml", ".smali", ".txt"};
    QMap<QString, QString> replacements;
    replacements["https://prod.simpsons-ea.com"] = gameServerUrl;
    replacements["https://syn-dir.sn.eamobile.com"] = gameServerUrl;


    q->emit log("\nSearching for URLs to replace:");
    for (auto it = replacements.begin(); it != replacements.end(); ++it) {
        q->emit log("  " + it.key() + " -> " + it.value());
    }

    QDirIterator it("tappedout", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        
        bool isTextFile = false;
        for (const auto& ext : textExtensions) {
            if (filePath.endsWith(ext)) {
                isTextFile = true;
                break;
            }
        }
        
        if (!isTextFile) continue;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            q->emit log("WARNING: Could not open file: " + filePath);
            continue;
        }

        QString content = file.readAll();
        bool modified = false;

        for (auto it = replacements.begin(); it != replacements.end(); ++it) {
            if (content.contains(it.key())) {
                content.replace(it.key(), it.value());
                q->emit log("Replaced '" + it.key() + "' with '" + it.value() + "' in " + filePath);
                modified = true;
            }
        }

        if (modified) {
            file.seek(0);
            file.write(content.toUtf8());
            file.resize(file.pos());
        }
        
        file.close();
    }

    q->emit log("\n=== Binary Patching Summary ===");
    q->emit log("Starting binary patching for DLC URL...");

    QByteArray originalUrl = "http://oct2018-4-35-0-uam5h44a.tstodlc.eamobile.com/netstorage/gameasset/direct/simpsons/";
    q->emit log("Original DLC URL length: " + QString::number(originalUrl.length()) + " bytes");

    QString newUrl = dlcServerUrl.trimmed();
    if (newUrl.endsWith('/')) {
        newUrl.chop(1);
    }
    newUrl += "/static/";
    QByteArray newUrlBytes = newUrl.toUtf8();
    q->emit log("New DLC URL: " + newUrl);
    q->emit log("New DLC URL length: " + QString::number(newUrlBytes.length()) + " bytes");

    // Pad with "./" pairs if needed
    int paddingNeeded = originalUrl.length() - newUrlBytes.length();
    if (paddingNeeded > 0) {
        q->emit log("Adding " + QString::number(paddingNeeded) + " bytes of padding");
        while (newUrlBytes.length() < originalUrl.length() - 1) {
            newUrlBytes.append("./");
        }
        if (newUrlBytes.length() < originalUrl.length()) {
            newUrlBytes.append('/');
        }
        q->emit log("Final padded URL: " + QString::fromUtf8(newUrlBytes));
        q->emit log("Final URL length: " + QString::number(newUrlBytes.length()) + " bytes");
    } else if (paddingNeeded < 0) {
        q->emit log("ERROR: New URL is too long by " + QString::number(-paddingNeeded) + " bytes");
        q->emit error("New DLC URL is too long");
        return false;
    }

    QDirIterator soIt("tappedout", {"*.so"}, QDir::Files, QDirIterator::Subdirectories);
    while (soIt.hasNext()) {
        QString filePath = soIt.next();
        q->emit log("Processing .so file: " + filePath);

        QFile file(filePath);
        if (!file.open(QIODevice::ReadWrite)) {
            q->emit log("WARNING: Could not open .so file: " + filePath);
            continue;
        }

        QByteArray content = file.readAll();
        int offset = content.indexOf(originalUrl);

        if (offset >= 0) {
            q->emit log("Found DLC URL at offset " + QString::number(offset));
            q->emit log("Replacing with padded URL...");
            
            content.replace(offset, originalUrl.length(), newUrlBytes);
            file.seek(0);
            file.write(content);
            q->emit log("Successfully patched DLC URL");
        } else {
            q->emit log("DLC URL not found in this file");
        }
        
        file.close();
    }

    return true;
}

bool APKPatcherPrivate::decompileApp(const QString& inputFile)
{
    q->emit log("Starting decompilation...");
    q->emit log("Current directory: " + QDir::currentPath());
    q->emit log("Input APK: " + inputFile);

    // Check if input file exists
    if (!QFile::exists(inputFile)) {
        q->emit log("ERROR: Input APK not found: " + inputFile);
        q->emit error("Input APK not found");
        return false;
    }

    QDir().mkdir("tappedout");

    QProcess process;
    process.setWorkingDirectory(QDir::currentPath());
    process.setProgram("java");
    QDir apktoolDir("sdktools/apktool");
    QString apktoolJar = apktoolDir.absoluteFilePath(apktoolDir.entryList({"*.jar"}).first());
    process.setArguments({"-jar", apktoolJar, "d", inputFile, "-f", "-o", "tappedout"});

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    QByteArray javaHomeBytes = qgetenv("JAVA_HOME");
    QString javaHome = QString::fromLocal8Bit(javaHomeBytes);
    if (javaHome.isEmpty()) {
        QProcess javaPathProcess;
        javaPathProcess.start("where", QStringList() << "java");
        if (javaPathProcess.waitForFinished()) {
            QString javaPath = QString::fromUtf8(javaPathProcess.readAllStandardOutput()).trimmed();
            if (!javaPath.isEmpty()) {
                QFileInfo javaFileInfo(javaPath.split("\n").first());
                QString javaBinDir = javaFileInfo.absolutePath();
                QString possibleJavaHome = QDir(javaBinDir).absolutePath();
                
                if (possibleJavaHome.endsWith("bin", Qt::CaseInsensitive)) {
                    possibleJavaHome = QDir(possibleJavaHome).absolutePath() + "/..";
                    possibleJavaHome = QDir(possibleJavaHome).canonicalPath();
                    
                    q->emit log("Temporarily setting JAVA_HOME to: " + possibleJavaHome);
                    env.insert("JAVA_HOME", possibleJavaHome);
                    
                    QString path = env.value("PATH");
                    env.insert("PATH", javaBinDir + ";" + path);
                }
            }
        }
    }
    
    env.insert("SOURCE_OUTPUT", "./tappedout");
    env.insert("APK_FILE", inputFile);
    env.insert("DLC_URL", dlcServerUrl);
    env.insert("GAMESERVER_URL", gameServerUrl);
    env.insert("DIRECTOR_URL", gameServerUrl);
    process.setProcessEnvironment(env);

    q->emit log("\nExecuting command: java -jar " + apktoolJar + " d " + inputFile + " -f -o tappedout");
    
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();

    if (!process.waitForStarted(30000)) {
        q->emit log("ERROR: Failed to start java process");
        q->emit log("Error details: " + process.errorString());
        q->emit error("Failed to start decompilation process");
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

    if (process.exitCode() != 0) {
        QString errorOutput = QString::fromUtf8(process.readAllStandardError());
        q->emit log("ERROR: Process failed with code " + QString::number(process.exitCode()));
        q->emit log("Error output: " + errorOutput);
        q->emit error("Decompilation failed");
        return false;
    }

    q->emit log("Decompilation completed successfully");
    return true;
}

bool APKPatcherPrivate::recompileApp(const QString& inputFile)
{
    q->emit log("Starting APK recompilation...");

    QProcess buildProcess;
    buildProcess.setWorkingDirectory(QDir::currentPath());
    buildProcess.setProgram("java");
    QDir apktoolDir("sdktools/apktool");
    QString apktoolJar = apktoolDir.absoluteFilePath(apktoolDir.entryList({"*.jar"}).first());
    buildProcess.setArguments({"-jar", apktoolJar, "b", "tappedout", "-o", "unsigned.apk"});
    
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    QByteArray javaHomeBytes = qgetenv("JAVA_HOME");
    QString javaHome = QString::fromLocal8Bit(javaHomeBytes);
    if (javaHome.isEmpty()) {
        QProcess javaPathProcess;
        javaPathProcess.start("where", QStringList() << "java");
        if (javaPathProcess.waitForFinished()) {
            QString javaPath = QString::fromUtf8(javaPathProcess.readAllStandardOutput()).trimmed();
            if (!javaPath.isEmpty()) {
                QFileInfo javaFileInfo(javaPath.split("\n").first());
                QString javaBinDir = javaFileInfo.absolutePath();
                QString possibleJavaHome = QDir(javaBinDir).absolutePath();
                
                if (possibleJavaHome.endsWith("bin", Qt::CaseInsensitive)) {
                    possibleJavaHome = QDir(possibleJavaHome).absolutePath() + "/..";
                    possibleJavaHome = QDir(possibleJavaHome).canonicalPath();
                    
                    q->emit log("Temporarily setting JAVA_HOME to: " + possibleJavaHome);
                    env.insert("JAVA_HOME", possibleJavaHome);
                    
                    QString path = env.value("PATH");
                    env.insert("PATH", javaBinDir + ";" + path);
                }
            }
        }
    }
    
    buildProcess.setProcessEnvironment(env);
    buildProcess.setProcessChannelMode(QProcess::MergedChannels);
    buildProcess.start();

    if (!buildProcess.waitForStarted(30000)) {
        q->emit error("Failed to start APK build process");
        return false;
    }

    while (buildProcess.state() != QProcess::NotRunning) {
        if (buildProcess.waitForReadyRead(1000)) {
            QString output = QString::fromUtf8(buildProcess.readAll()).trimmed();
            if (!output.isEmpty()) {
                q->emit log(output);
            }
        }
    }

    buildProcess.waitForFinished();
    if (buildProcess.exitCode() != 0) {
        q->emit error("APK build failed");
        return false;
    }

    QFileInfo fi(inputFile);
    QString outputName = fi.baseName() + "-patched.apk";

    q->emit log("Signing APK...");
    QProcess signProcess;
    signProcess.setWorkingDirectory(QDir::currentPath());
    
    QString jarsignerPath;
    
    QProcess jarsignerCheckProcess;
    jarsignerCheckProcess.start("jarsigner", QStringList() << "-help");
    if (jarsignerCheckProcess.waitForStarted() && jarsignerCheckProcess.waitForFinished()) {
        if (jarsignerCheckProcess.exitCode() == 0) {
            jarsignerPath = "jarsigner";
            q->emit log("Found jarsigner in system PATH");
        }
    }
    
    if (jarsignerPath.isEmpty()) {
        QString javaHome = env.value("JAVA_HOME");
        if (!javaHome.isEmpty()) {
            QString possiblePath = QDir(javaHome).filePath("bin/jarsigner");
            if (QFile::exists(possiblePath + ".exe")) {
                possiblePath += ".exe";
            }
            
            if (QFile::exists(possiblePath)) {
                jarsignerPath = possiblePath;
                q->emit log("Found jarsigner in JAVA_HOME/bin: " + jarsignerPath);
            }
        }
    }
    
    if (jarsignerPath.isEmpty()) {
        QProcess javaPathProcess;
        javaPathProcess.start("where", QStringList() << "java");
        if (javaPathProcess.waitForFinished()) {
            QString javaPath = QString::fromUtf8(javaPathProcess.readAllStandardOutput()).trimmed();
            if (!javaPath.isEmpty()) {
                QFileInfo javaFileInfo(javaPath.split("\n").first());
                QString javaBinDir = javaFileInfo.absolutePath();
                
                QString possiblePath = QDir(javaBinDir).filePath("jarsigner.exe");
                if (QFile::exists(possiblePath)) {
                    jarsignerPath = possiblePath;
                    q->emit log("Found jarsigner in same directory as java: " + jarsignerPath);
                }
            }
        }
    }
    
    if (jarsignerPath.isEmpty()) {
        QStringList commonJavaLocations = {
            "C:/Program Files/Java/jdk*/bin/jarsigner.exe",
            "C:/Program Files (x86)/Java/jdk*/bin/jarsigner.exe",
            "C:/Program Files/Java/jre*/bin/jarsigner.exe",
            "C:/Program Files (x86)/Java/jre*/bin/jarsigner.exe"
        };
        
        for (const auto& pattern : commonJavaLocations) {
            QStringList matches;
            QDirIterator it("C:/Program Files", QDirIterator::Subdirectories);
            while (it.hasNext()) {
                QString path = it.next();
                if (path.contains("jarsigner.exe", Qt::CaseInsensitive)) {
                    matches.append(path);
                }
            }
            
            if (!matches.isEmpty()) {
                jarsignerPath = matches.first();
                q->emit log("Found jarsigner in common Java location: " + jarsignerPath);
                break;
            }
        }
    }
    
    if (jarsignerPath.isEmpty()) {
        QString bundledJarsigner = "sdktools/jarsigner.exe";
        if (QFile::exists(bundledJarsigner)) {
            jarsignerPath = bundledJarsigner;
            q->emit log("Using bundled jarsigner: " + jarsignerPath);
        }
    }
    
    if (jarsignerPath.isEmpty()) {
        q->emit log("ERROR: Could not find jarsigner executable");
        q->emit log("Please make sure Java JDK is installed and either:");
        q->emit log("1. jarsigner is in your system PATH");
        q->emit log("2. JAVA_HOME environment variable is set correctly");
        q->emit error("jarsigner not found. Please install Java JDK and ensure it's in your PATH");
        return false;
    }
    
    signProcess.setProgram(jarsignerPath);
    
    QString keystorePath = "sdktools/debug.keystore";
    if (!QFile::exists(keystorePath)) {
        q->emit log("WARNING: debug.keystore not found at: " + keystorePath);
        
        if (QFile::exists("debug.keystore")) {
            keystorePath = "debug.keystore";
            q->emit log("Found debug.keystore in current directory");
        } else {
            q->emit error("debug.keystore not found");
            return false;
        }
    }
    
    q->emit log("Using keystore: " + keystorePath);
    signProcess.setArguments({
        "-verbose",
        "-keystore", keystorePath,
        "-storepass", "android",
        "-keypass", "android",
        "unsigned.apk",
        "androiddebugkey"
    });

    signProcess.setProcessEnvironment(env);
    signProcess.setProcessChannelMode(QProcess::MergedChannels);
    signProcess.start();

    if (!signProcess.waitForStarted(30000)) {
        QString errorMsg = "Failed to start APK signing process: " + signProcess.errorString();
        q->emit log("ERROR: " + errorMsg);
        q->emit log("Command attempted: " + jarsignerPath + " -verbose -keystore " + keystorePath + 
                   " -storepass android -keypass android unsigned.apk androiddebugkey");
        q->emit error(errorMsg);
        return false;
    }

    while (signProcess.state() != QProcess::NotRunning) {
        if (signProcess.waitForReadyRead(1000)) {
            QString output = QString::fromUtf8(signProcess.readAll()).trimmed();
            if (!output.isEmpty()) {
                q->emit log(output);
            }
        }
    }

    signProcess.waitForFinished();
    if (signProcess.exitCode() != 0) {
        q->emit error("APK signing failed");
        return false;
    }

    if (QFile::exists(outputName)) {
        QFile::remove(outputName);
    }
    if (!QFile::rename("unsigned.apk", outputName)) {
        if (!QFile::copy("unsigned.apk", outputName)) {
            q->emit error("Failed to rename signed APK to output");
            return false;
        }
        QFile::remove("unsigned.apk");
    }

    q->emit log("APK recompilation and signing completed successfully");
    return true;
}

bool APKPatcherPrivate::patchAPK(const QString& apkPath, const QString& newGameServerUrl, const QString& newDlcServerUrl)
{
    bool success = true;
    q->emit log("Starting APK patching process...");
    q->emit log("APK Path: " + apkPath);
    q->emit log("Game Server: " + newGameServerUrl);
    q->emit log("DLC Server: " + newDlcServerUrl);

    gameServerUrl = newGameServerUrl;
    dlcServerUrl = newDlcServerUrl;

    if (!q->checkDependencies()) {
        success = false;
    }

    if (success) {
        q->emit progressUpdated(20, "Decompiling APK...");
        if (!decompileApp(apkPath)) {
            success = false;
        }
    }

    if (success) {
        q->emit progressUpdated(50, "Replacing URLs...");
        if (!replaceUrls(gameServerUrl, dlcServerUrl)) {
            success = false;
        }
    }

    if (success) {
        q->emit progressUpdated(80, "Recompiling APK...");
        if (!recompileApp(apkPath)) {
            success = false;
        }
    }

    if (success) {
        q->emit progressUpdated(100, "APK patched successfully!");
        q->emit log("\n=== Final URL Summary ===");
        q->emit log("Game Server URL: " + gameServerUrl);
        QString finalDlcUrl = dlcServerUrl.trimmed();
        if (finalDlcUrl.endsWith('/')) {
            finalDlcUrl.chop(1);
        }
        finalDlcUrl += "/static/";
        QByteArray paddedUrl = finalDlcUrl.toUtf8();
        while (paddedUrl.length() < 89 - 1) {  
            paddedUrl.append("./");
        }
        if (paddedUrl.length() < 89) {
            paddedUrl.append('/');
        }
        q->emit log("DLC Server URL (with padding): " + QString::fromUtf8(paddedUrl));
        q->emit log("\nAPK patched successfully!");
    } else {
        q->emit progressUpdated(100, "Failed to patch APK");
    }
    return success;
}

void APKPatcher::patchAPK(const QString& apkPath, const QString& gameServerUrl, const QString& dlcServerUrl)
{
    if (d->patchAPK(apkPath, gameServerUrl, dlcServerUrl)) {
        emit progressUpdated(100, "APK patched successfully!");
    } else {
        emit progressUpdated(100, "Failed to patch APK");
    }
}

} 
