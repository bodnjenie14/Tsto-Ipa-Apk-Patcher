#pragma once
#include "std_include.hpp"

namespace Patcher {

class AppPatcherPrivate;

class AppPatcher : public QObject {
    Q_OBJECT

public:
    explicit AppPatcher(QObject* parent = nullptr);
    virtual ~AppPatcher();

    bool checkDependencies();
    void patchAPK(const QString& apkPath,
        const QString& gameServerUrl = QString(),
        const QString& dlcServerUrl = QString());
    void patchIPA(const QString& ipaPath,
        const QString& gameServerUrl = QString(),
        const QString& dlcServerUrl = QString());

signals:
    void progressUpdated(int progress, const QString& status);
    void error(const QString& message);
    void log(const QString& message);

private:
    AppPatcherPrivate* d;
    Q_DISABLE_COPY(AppPatcher)
};

} 
