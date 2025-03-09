#pragma once
#include "std_include.hpp"
#include <QtCore/QObject>
#include <QtCore/QString>

namespace Patcher {

class APKPatcherPrivate;

class APKPatcher : public QObject {
    Q_OBJECT

public:
    explicit APKPatcher(QObject* parent = nullptr);
    virtual ~APKPatcher();

    bool checkDependencies();
    void patchAPK(const QString& apkPath,
        const QString& gameServerUrl = QString(),
        const QString& dlcServerUrl = QString());

signals:
    void progressUpdated(int progress, const QString& status);
    void error(const QString& message);
    void log(const QString& message);

private:
    QString javaHomeDir_; 
    APKPatcherPrivate* d;
    Q_DISABLE_COPY(APKPatcher)
};
} 
