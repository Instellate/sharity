#pragma once

#include <QFile>
#include <QObject>
#include <QJsonObject>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class UploaderPeer : public QObject {
    Q_OBJECT
    QML_ELEMENT

    QFile _uploadingFile;
    std::shared_ptr<rtc::PeerConnection> _peer;
    std::shared_ptr<rtc::DataChannel> _channel = nullptr;

public:
    UploaderPeer();

    private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
