//
// Created by instellate on 2026-01-20.
//

#include "certificate.h"

#include <QDir>
#include <QStandardPaths>

// USE THIS ONLY ON ANDROID
QString getCaCertificate() {
    QDir appConfig{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
    QFile previousCaCert{appConfig.filePath("cacert.pem")};
    if (appConfig.exists("cacert.pem")) {
        appConfig.remove("cacert.pem");
    }

    if (!appConfig.exists()) {
        appConfig.cdUp();
        // ReSharper disable once CppExpressionWithoutSideEffects
        appConfig.mkpath("settings");
        appConfig.cd("settings");
    }

    QFile caCert{":/resources/cacert.pem"};
    caCert.copy(appConfig.filePath("cacert.pem"));

    if (!appConfig.exists("cacert.pem")) {
        qFatal() << "cacerts.pem does not exist";
    }

    qputenv("SSL_CERT_FILE", appConfig.filePath("cacert.pem").toUtf8());

    return appConfig.filePath("cacert.pem");
}

