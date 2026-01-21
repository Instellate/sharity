#pragma once

#include <QFile>
#include <QFuture>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class UploaderPeer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl selectedFile READ selectedFile WRITE setSelectedFile NOTIFY selectedFileChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(qint64 amountUploaded READ amountUploaded NOTIFY amountUploadedChanged)
    Q_PROPERTY(qint64 speed READ speed NOTIFY speedChanged)
    QML_ELEMENT

    using DataChannel = std::shared_ptr<rtc::DataChannel>;

    QUrl _selectedFile;
    std::shared_ptr<rtc::PeerConnection> _peer = nullptr;

    QTimer *_timer;
    QAtomicInteger<qint64> _uploadedSinceTick;

    QFuture<void> _streamFuture;
    qint64 _amountUploaded = 0;
    qint64 _fileSize = 0;
    qint64 _speed = 0;

    void startRtcNegotiation();
    void handleFileUpload(const DataChannel &channel);

public:
    explicit UploaderPeer(QObject *parent = nullptr);

    ~UploaderPeer() override;

    [[nodiscard]] qint64 amountUploaded() const;
    [[nodiscard]] qint64 fileSize() const;
    [[nodiscard]] qint64 speed() const;

    [[nodiscard]] QUrl selectedFile() const;
    void setSelectedFile(const QUrl &url);

    [[nodiscard]] QString fileName() const;

    Q_INVOKABLE void startFileNegotiation() const;
    Q_INVOKABLE void close();

signals:
    void selectedFileChanged();
    void fileUploaded();
    void amountUploadedChanged();
    void fileSizeChanged();
    void speedChanged();
    void fileNameChanged();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
    void tick();
};
