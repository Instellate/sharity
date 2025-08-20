#pragma once

#include <QFile>
#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class DownloaderPeer : public QObject {
    Q_OBJECT
    QML_ELEMENT

    using DataChannel = std::shared_ptr<rtc::DataChannel>;

    std::shared_ptr<rtc::PeerConnection> _peer;
    std::vector<DataChannel> _channels;
    QString _expectedLabel;

    void handleDataChannel(const DataChannel &channel);

public:
    explicit DownloaderPeer(QObject *parent = nullptr);

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
