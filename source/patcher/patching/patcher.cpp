#include "std_include.hpp"
#include "patcher.hpp"
#include "apk_patcher.hpp"
#include "ipa_patcher.hpp"

namespace Patcher {

class AppPatcherPrivate {
public:
    AppPatcher* q;
    APKPatcher* apkPatcher;
    IPAPatcher* ipaPatcher;

    explicit AppPatcherPrivate(AppPatcher* patcher) 
        : q(patcher)
        , apkPatcher(new APKPatcher(patcher))
        , ipaPatcher(new IPAPatcher(patcher))
    {
        QObject::connect(apkPatcher, &APKPatcher::progressUpdated,
                        q, &AppPatcher::progressUpdated);
        QObject::connect(apkPatcher, &APKPatcher::error,
                        q, &AppPatcher::error);
        QObject::connect(apkPatcher, &APKPatcher::log,
                        q, &AppPatcher::log);

        QObject::connect(ipaPatcher, &IPAPatcher::progressUpdated,
                        q, &AppPatcher::progressUpdated);
        QObject::connect(ipaPatcher, &IPAPatcher::error,
                        q, &AppPatcher::error);
        QObject::connect(ipaPatcher, &IPAPatcher::log,
                        q, &AppPatcher::log);
    }

    ~AppPatcherPrivate() {
        delete apkPatcher;
        delete ipaPatcher;
    }
};

AppPatcher::AppPatcher(QObject* parent)
    : QObject(parent)
    , d(new AppPatcherPrivate(this))
{
}

AppPatcher::~AppPatcher()
{
    delete d;
}

bool AppPatcher::checkDependencies()
{
    return d->apkPatcher->checkDependencies() && 
           d->ipaPatcher->checkDependencies();
}

void AppPatcher::patchAPK(const QString& apkPath, const QString& gameServerUrl, const QString& dlcServerUrl)
{
    d->apkPatcher->patchAPK(apkPath, gameServerUrl, dlcServerUrl);
}

void AppPatcher::patchIPA(const QString& ipaPath, const QString& gameServerUrl, const QString& dlcServerUrl)
{
    d->ipaPatcher->patchIPA(ipaPath, gameServerUrl, dlcServerUrl);
}

} 
