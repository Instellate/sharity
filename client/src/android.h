//
// Created by instellate on 2026-01-20.
//

#pragma once

#include <QtSystemDetection>

#ifdef Q_OS_ANDROID

#include <QFile>
#include <QJniObject>
#include <QString>
#include <QUrl>

// USE THIS ONLY ON ANDROID
QString getCaCertificate();

QJniObject &contentResolverInstance();

QJniObject getDownloadUri(const QString &fileName);

void urlDonePending(QJniObject uri);

class AndroidContentFile : public QFile {
    QJniObject _contentUri;
    bool _isPending = false;

    static int openFd(const QJniObject &uri);

public:
    AndroidContentFile() = default;

    bool openDownloadFile(const QString &fileName);

    bool openContentUri(const QUrl &url);

    void close() override;
};

#endif
