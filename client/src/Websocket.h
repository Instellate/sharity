#pragma once

#include <qtmetamacros.h>
#include <rtc/rtc.hpp>
#include <rtc/websocket.hpp>
#include <variant>
#include <vodozemac.h>

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <qqmlintegration.h>

class WebSocket : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList stunServers READ stunServers NOTIFY stunServersChanged)
    Q_PROPERTY(QString publicKey READ publicKey NOTIFY publicKeyChanged)
    Q_PROPERTY(bool established READ established NOTIFY establishedChanged)
    Q_PROPERTY(bool encrypted READ encrypted NOTIFY encryptedChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool isDownloader READ isDownloader NOTIFY isDownloaderChanged)
    Q_PROPERTY(bool connecting READ connecting NOTIFY connectingChanged)
    QML_ELEMENT
    QML_SINGLETON

    QMutex _mutex;

    // If this isn't nullopt the WebSocket is a downloader
    // If this is nullopt the webSocket is an uploader
    std::optional<vodozemac::Ed25519PublicKey> _publicKey = std::nullopt;
    rtc::WebSocket _ws;
    QStringList _stunServers;
    bool _established = false;
    bool _encrypted = false;
    bool _connecting = false;

    vodozemac::olm::Account _account;
    std::optional<vodozemac::olm::Session> _session = std::nullopt;

    void sendEncrypted(const QString &message);

    void onMessage(std::variant<rtc::binary, rtc::string> &&message);

    void handleConnected(rtc::string &&message);
    void handleInitialEstablishment();

    // The uploader handling the tokens from the downloader
    void handleDownloaderRequest(rtc::binary &&binary);

    void handleUploaderResponse(rtc::binary &&binary);

    void handleEncryptedMessage(std::u8string &&message);

public:
    explicit WebSocket(QObject *parent = nullptr);

    static WebSocket *instance();

    Q_INVOKABLE void open(const QString &url, QString publicKey);
    Q_INVOKABLE void open(const QString &url);

    Q_INVOKABLE void send(const QString &message);
    Q_INVOKABLE void close();

    [[nodiscard]] QStringList stunServers() const;
    [[nodiscard]] QString publicKey() const;
    [[nodiscard]] bool encrypted() const;
    [[nodiscard]] bool established() const;
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool isDownloader() const;
    [[nodiscard]] bool connecting() const;

signals:
    void stunServersChanged();
    void establishedChanged();
    void encryptedChanged();
    void connectedChanged();
    void publicKeyChanged();
    void isDownloaderChanged();
    void connectingChanged();

    void message(const QString &type, const QJsonObject &json);
};
