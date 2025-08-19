#pragma once

#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class DownloaderPeer : public QObject {
    Q_OBJECT
    QML_ELEMENT

    std::shared_ptr<rtc::PeerConnection> _peer;
    std::shared_ptr<rtc::DataChannel> _channel;

public:
    DownloaderPeer();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
