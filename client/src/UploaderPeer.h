#pragma once

#include <QFile>
#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class UploaderPeer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl selectedFile READ selectedFile WRITE setSelectedFile NOTIFY selectedFileChanged)
    QML_ELEMENT

    using DataChannel = std::shared_ptr<rtc::DataChannel>;

    QUrl _selectedFile;
    std::shared_ptr<rtc::PeerConnection> _peer = nullptr;
    std::vector<DataChannel> _channels{};

public:
    [[nodiscard]] QUrl selectedFile() const;
    void setSelectedFile(QUrl url);

    Q_INVOKABLE void startFileNegotiation();

signals:
    void selectedFileChanged();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
