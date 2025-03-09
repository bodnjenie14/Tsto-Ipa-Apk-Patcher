#pragma once
#include "std_include.hpp"
#include <QtCore/QObject>
#include <QtCore/QString>

namespace Patcher {

class IPAPatcherPrivate;

class IPAPatcher : public QObject {
    Q_OBJECT

public:
    explicit IPAPatcher(QObject* parent = nullptr);
    virtual ~IPAPatcher();

    bool checkDependencies();
    void patchIPA(const QString& ipaPath,
        const QString& gameServerUrl = QString(),
        const QString& dlcServerUrl = QString());

signals:
    void progressUpdated(int progress, const QString& status);
    void error(const QString& message);
    void log(const QString& message);

private:
    IPAPatcherPrivate* d;
    Q_DISABLE_COPY(IPAPatcher)
};
} 
