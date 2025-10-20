#pragma once

#include <QFile>
#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <qqmlintegration.h>
#include <rtc/rtc.hpp>

class DownloaderPeer : public QObject {
    Q_OBJECT
    Q_PROPERTY(State downloadState READ downloadState NOTIFY downloadStateChanged)
    Q_PROPERTY(qint64 fileSize READ fileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(qint64 amountDownloaded READ amountDownloaded NOTIFY amountDownloadedChanged)
    Q_PROPERTY(qint64 speed READ speed NOTIFY speedChanged)
    QML_ELEMENT

public:
    enum State {
        Waiting,
        Downloading,
        Downloaded
    };
    Q_ENUM(State);

private:
    using DataChannel = std::shared_ptr<rtc::DataChannel>;

    std::shared_ptr<rtc::PeerConnection> _peer;
    std::vector<DataChannel> _channels;

    QTimer *_timer;
    QAtomicInteger<qint64> _downloadedSinceTick;

    State _state = Waiting;
    QString _expectedLabel;
    qint64 _fileSize = 0;
    qint64 _amountDownloaded = 0;
    qint64 _speed = 0;

    void handleDataChannel(const DataChannel &channel);

public:
    explicit DownloaderPeer(QObject *parent = nullptr);

    ~DownloaderPeer() override;

    [[nodiscard]] State downloadState() const;
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
