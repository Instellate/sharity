#pragma once

#include <qatomic.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <rtc/common.hpp>
#include <rtc/websocket.hpp>
#include <variant>
#include <vodozemac.h>

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <nlohmann/adl_serializer.hpp>
#include <qqmlintegration.h>

class WebSocket : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList stunServers READ stunServers NOTIFY stunServersChanged)
    Q_PROPERTY(QString publicKey READ publicKey)
    Q_PROPERTY(bool established READ established NOTIFY establishedChanged)
    Q_PROPERTY(bool encrypted READ encrypted NOTIFY encryptedChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(bool sasEstablished READ sasEstablished NOTIFY sasEstablishedChanged)
    Q_PROPERTY(QString sasDecimals READ sasDecimals)
    Q_PROPERTY(QString sasEmojis READ sasEmojis)
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
    bool _sasEstablished = false;

    vodozemac::olm::Account _account;
    std::optional<vodozemac::olm::Session> _session = std::nullopt;
    vodozemac::sas::Sas _sas;

    void sendEncrypted(const std::string &message);

    void onMessage(std::variant<rtc::binary, rtc::string> &&message);

    void handleConnected(rtc::string &&message);
    void handleInitialEstablishment();

    // The uploader handling the tokens from the downloader
    void handleDownloaderRequest(rtc::binary &&binary);

    void handleUploaderResponse(rtc::binary &&binary);

    void handleEncryptedMessage(std::u8string &&message);

    void handleSasToken(nlohmann::json &&json);

public:
    explicit WebSocket(QObject *parent = nullptr);

    Q_INVOKABLE void open(const QString &url, QString publicKey);
    Q_INVOKABLE void open(const QString &url);

    [[nodiscard]] QStringList stunServers() const;
    [[nodiscard]] QString publicKey() const;
    [[nodiscard]] bool encrypted() const;
    [[nodiscard]] bool established() const;
    [[nodiscard]] bool connected() const;
    [[nodiscard]] bool sasEstablished() const;
    [[nodiscard]] QString sasEmojis() const;
    [[nodiscard]] QString sasDecimals() const;

signals:
    void stunServersChanged();
    void establishedChanged();
    void encryptedChanged();
    void closed();
    void connectedChanged();
    void sasEstablishedChanged();
};
