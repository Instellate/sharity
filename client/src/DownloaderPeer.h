#pragma once

#include <QFile>
#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>
#include <qtimer.h>
#include <rtc/rtc.hpp>

class DownloaderPeer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString downloadState READ downloadState NOTIFY downloadStateChanged)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(qint64 amountDownloaded READ amountDownloaded NOTIFY amountDownloadedChanged)
    Q_PROPERTY(qint64 speed READ speed NOTIFY speedChanged)
    QML_ELEMENT

    using DataChannel = std::shared_ptr<rtc::DataChannel>;

    std::shared_ptr<rtc::PeerConnection> _peer;
    std::vector<DataChannel> _channels;

    QTimer *_timer;
    QAtomicInteger<qint64> _downloadedSinceTick;

    QString _state = "Waiting";
    QString _expectedLabel;
    qint64 _fileSize = 0;
    qint64 _amountDownloaded = 0;
    qint64 _speed = 0;

    void handleDataChannel(const DataChannel &channel);

public:
    explicit DownloaderPeer(QObject *parent = nullptr);

    ~DownloaderPeer() override;

    [[nodiscard]] QString downloadState();
    [[nodiscard]] qint64 fileSize() const;
    [[nodiscard]] qint64 amountDownloaded() const;
    [[nodiscard]] qint64 speed() const;

    Q_INVOKABLE void close();

signals:
    void downloadStateChanged();
    void fileSizeChanged();
    void amountDownloadedChanged();
    void speedChanged();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
    void tick();
};
