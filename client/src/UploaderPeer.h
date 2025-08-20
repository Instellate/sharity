#pragma once

#include <QFile>
#include <QFuture>
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

    QFuture<void> _streamFuture;

    void startRtcNegotiation();
    void handleFileUpload(const DataChannel &channel);

public:
    explicit UploaderPeer(QObject *parent = nullptr);

    ~UploaderPeer() override;

    [[nodiscard]] QUrl selectedFile() const;
    void setSelectedFile(const QUrl &url);

    Q_INVOKABLE void startFileNegotiation() const;

signals:
    void selectedFileChanged();
    void fileUploaded();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
