#include "Websocket.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QUrlQuery>
#include <QtLogging>
#include <exception>
#include <optional>
#include <qrandom.h>
#include <string>
#include <variant>

void WebSocket::sendEncrypted(const QString &message) {
    auto bytes = this->_session->encrypt(message.toStdString()).bytes;
    auto bytesPtr = reinterpret_cast<std::byte *>(bytes.data());

    rtc::binary binary{bytesPtr, bytesPtr + bytes.size()};
    this->_ws.send(binary);
}

void WebSocket::onMessage(std::variant<rtc::binary, rtc::string> &&message) {
    QMutexLocker locker{&this->_mutex};

    if (!this->_established) {
        if (!std::holds_alternative<rtc::string>(message)) {
            qWarning() << "Got binary when expected string";
            return;
        }

        auto str = std::get<rtc::string>(std::move(message));
        handleConnected(std::move(str));
    } else if (!this->_encrypted) {
        if (!std::holds_alternative<rtc::binary>(message)) {
            qWarning() << "Got string when expected binary";
            return;
        }

        auto binary = std::get<rtc::binary>(std::move(message));
        if (!this->_publicKey.has_value()) {
            handleDownloaderRequest(std::move(binary));
        } else {
            handleUploaderResponse(std::move(binary));
        }
    } else if (!std::holds_alternative<rtc::binary>(message)) {
        qWarning() << "Got string when expected binary";
        return;
    }

    auto binary = std::get<rtc::binary>(std::move(message));
    const auto binaryPtr = reinterpret_cast<uint8_t *>(binary.data());
    std::vector<uint8_t> buff{binaryPtr, binaryPtr + binary.size()};

    std::vector<uint8_t> encryptedMessageBytes{binaryPtr, binaryPtr + binary.size()};
    rust::Vec<uint8_t> encryptedMessageRustVec;
    std::ranges::copy(encryptedMessageBytes, std::back_inserter(encryptedMessageRustVec));
    vodozemac::olm::OlmMessage olmMessage;
    olmMessage.bytes = encryptedMessageRustVec;

    auto messageRustBytes = this->_session.value().decrypt(olmMessage);
    std::u8string decryptedMessage{reinterpret_cast<const char8_t *>(messageRustBytes.data()), messageRustBytes.size()};
    handleEncryptedMessage(std::move(decryptedMessage));
}

void WebSocket::handleConnected(rtc::string &&message) {
    const auto jsonDoc = QJsonDocument::fromJson(QByteArray{message.data(), static_cast<qsizetype>(message.size())});
    if (jsonDoc.isNull()) {
        qWarning() << "Got invalid JSON";
        return;
    }
    const QJsonObject json = jsonDoc.object();

    if (!json.contains("type") || !json["type"].isString()) {
        qWarning() << "Got a json object from unestablished channel that does not contain property "
                      "\"type\"";
        return;
    }

    const QString type = json["type"].toString();
    if (type != "connected") {
        qWarning() << "Got unknown type from websocket:" << type;
        return;
    }

    const QStringList stunServers = json["stun_servers"].toVariant().toStringList();
    this->_stunServers = stunServers;
    emit stunServersChanged();
    if (this->_publicKey.has_value()) {
        handleInitialEstablishment();
    }
    this->_established = true;
    emit establishedChanged();
}

void WebSocket::handleInitialEstablishment() {
    std::vector<uint8_t> buffer;

    vodozemac::Curve25519PublicKey identityKey = this->_account.curve25519Key();
    vodozemac::Curve25519PublicKey oneTimeKey = this->_account.generateOneTimeKey();
    buffer.reserve(identityKey.bytes.size() + oneTimeKey.bytes.size());

    buffer.insert(buffer.end(), identityKey.bytes.begin(), identityKey.bytes.end());
    buffer.insert(buffer.end(), oneTimeKey.bytes.begin(), oneTimeKey.bytes.end());

    // No idea how send handles provided data, but I do not want segfaults
    auto *bufferPtr = reinterpret_cast<std::byte *>(buffer.data());
    rtc::binary bytes{bufferPtr, bufferPtr + buffer.size()};
    this->_ws.send(std::move(bytes));
}

void WebSocket::handleDownloaderRequest(rtc::binary &&binary) {
    if (binary.size() != 64) {
        qWarning() << "Expected 64 bytes from downloader but got" << binary.size() << "bytes";
        this->_ws.close();
        return;
    }

    std::array<uint8_t, 32> keyBuffer{};
    for (size_t i = 0; i < 32; ++i) {
        keyBuffer[i] = static_cast<uint8_t>(binary[i]);
    }
    vodozemac::Curve25519PublicKey identityKey;
    identityKey.bytes = keyBuffer;
    keyBuffer.fill(0);

    for (size_t i = 0; i < 32; ++i) {
        keyBuffer[i] = static_cast<uint8_t>(binary[i + 32]);
    }
    vodozemac::Curve25519PublicKey oneTimeKey;
    oneTimeKey.bytes = keyBuffer;

    auto session = this->_account.createOutboundSession(2, identityKey, oneTimeKey);

    QByteArray randomBuffer{};
    randomBuffer.resize(16);
    const auto random = QRandomGenerator::system();
    random->generate(randomBuffer.begin(), randomBuffer.end());

    const QString randomString = randomBuffer.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    auto message = session.encrypt(randomString.toStdString());
    this->_session = std::move(session);

    const auto binaryPtr = reinterpret_cast<uint8_t *>(binary.data());
    std::vector<uint8_t> buff{binaryPtr, binaryPtr + binary.size()};
    std::array<uint8_t, 32> myIdentityKey = this->_account.curve25519Key().bytes;
    buff.insert(buff.end(), myIdentityKey.begin(), myIdentityKey.end());
    buff.insert(buff.end(), message.bytes.begin(), message.bytes.end());

    auto signature = this->_account.sign(buff);
    buff.insert(buff.end(), signature.begin(), signature.end());

    const auto buffPtr = reinterpret_cast<std::byte *>(buff.data());
    rtc::binary response{buffPtr, buffPtr + buff.size()};
    qInfo() << "Sending a response with size" << response.size();
    this->_ws.send(std::move(response));

    this->_encrypted = true;
    emit encryptedChanged();
}

void WebSocket::handleUploaderResponse(rtc::binary &&binary) {
    if (binary.size() <= 168) {
        qWarning() << "Got a response that is less or equal to 168";
        this->_ws.close();
        return;
    }
    const auto binaryPtr = reinterpret_cast<uint8_t *>(binary.data());

    std::array<uint8_t, 64> signature{};
    for (size_t i = 0; i < 64; ++i) {
        signature[i] = static_cast<uint8_t>(binary[i + binary.size() - 64]);
    }

    std::vector<uint8_t> signedMessage{binaryPtr, binaryPtr + binary.size() - 64};
    const rust::Slice<const uint8_t> messageSlice{signedMessage.data(), signedMessage.size()};
    const rust::Slice<const uint8_t> signatureSlice{signature.data(), signature.size()};

    try {
        this->_publicKey.value().verify(messageSlice, signatureSlice);
    } catch (std::exception &e) {
        qWarning() << e.what();
        this->_ws.close();
        return;
    }

    std::array<uint8_t, 32> receivedIdentityKey{};
    for (size_t i = 0; i < 32; ++i) {
        receivedIdentityKey[i] = static_cast<uint8_t>(binary[i]);
    }
    if (this->_account.curve25519Key().bytes != receivedIdentityKey) {
        qWarning() << "The received identity key is not the same as the actual one";
        this->_ws.close();
        return;
    }

    std::vector<uint8_t> messageBytes{binaryPtr + 96, binaryPtr + binary.size() - 64};
    rust::Vec<uint8_t> messageRustVec;
    std::ranges::copy(messageBytes, std::back_inserter(messageRustVec));
    vodozemac::olm::OlmMessage message;
    message.bytes = messageRustVec;

    vodozemac::Curve25519PublicKey theirIdentityKey;
    for (size_t i = 0; i < 32; ++i) {
        theirIdentityKey.bytes[i] = static_cast<uint8_t>(binary[i + 64]);
    }

    auto [session, _] = this->_account.createInboundSession(theirIdentityKey, message);
    this->_session = std::move(session);
    this->_encrypted = true;
    emit encryptedChanged();
}

void WebSocket::handleEncryptedMessage(std::u8string &&message) {
    const QByteArray byteArray{reinterpret_cast<const char *>(message.data()), static_cast<qsizetype>(message.size())};
    const auto jsonDoc = QJsonDocument::fromJson(byteArray);
    if (jsonDoc.isNull()) {
        qWarning() << "Got invalid json from encrypted message";
        return;
    }

    QJsonObject json = jsonDoc.object();
    if (!json.contains("type") || !json["type"].isString()) {
        qWarning() << "Got a type that either does not exist or is not a string";
    }

    const QString type = json["type"].toString();
    qDebug() << "Got websocket message with type:" << type;
    emit this->message(type, json);
}

static WebSocket *singletonInstance = nullptr;

WebSocket::WebSocket(QObject *parent) : QObject(parent) {
    singletonInstance = this;

    this->_ws.onOpen([this] {
        qInfo() << "Connected to websocket server";
        emit connectedChanged();
        
        this->_connecting = false;
        emit connectingChanged();
    });
    this->_ws.onClosed([this] {
        qInfo() << "Websocket connection closed";
        this->_encrypted = false;
        this->_established = false;
        this->_connecting = false;
        this->_publicKey = std::nullopt;
        this->_session = std::nullopt;
        this->_stunServers.clear();
        this->_account.regenerate();

        emit connectedChanged();
        emit encryptedChanged();
        emit connectedChanged();
        emit connectingChanged();
        emit stunServersChanged();
        emit isDownloaderChanged();
    });
}

WebSocket *WebSocket::instance() { return singletonInstance; }

void WebSocket::open(const QString &url, QString publicKey) {
    const QString pubKeyCopy = publicKey;
    publicKey.replace('+', '-').replace('/', '_');
    QUrlQuery query{};
    query.addQueryItem("key", publicKey);
    query.addQueryItem("type", "downloader");

    QUrl fullUrl{url};
    fullUrl.setQuery(query);

    this->_publicKey = vodozemac::Ed25519PublicKey::fromBase64(pubKeyCopy.toStdString());
    this->_ws.onMessage([this](auto message) { this->onMessage(std::move(message)); });
    this->_ws.open(fullUrl.toString().toStdString());
    emit isDownloaderChanged();
    
    this->_connecting = true;
    emit connectingChanged();
}

void WebSocket::open(const QString &url) {
    QString publicKey = this->publicKey();
    publicKey.replace('+', '-').replace('/', '_');

    QByteArray randomBuffer{};
    randomBuffer.resize(16);
    const auto random = QRandomGenerator::system();
    random->generate(randomBuffer.begin(), randomBuffer.end());

    const QString rndMessage = randomBuffer.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    const QByteArray rndMessageBytes = rndMessage.toUtf8();

    const std::array<uint8_t, 64> signature = this->_account.sign({rndMessageBytes.begin(), rndMessageBytes.end()});
    const QByteArray qSignature{reinterpret_cast<const char *>(signature.data()), 64};
    const QString base64Signature = qSignature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    QUrlQuery query{};
    query.addQueryItem("key", publicKey);
    query.addQueryItem("type", "uploader");
    query.addQueryItem("message", rndMessage);
    query.addQueryItem("signature", base64Signature);

    QUrl fullUrl{url};
    fullUrl.setQuery(query);
    qInfo() << "Connecting with url" << fullUrl;

    this->_ws.onMessage([this](auto message) { this->onMessage(std::move(message)); });
    this->_ws.open(fullUrl.toString().toStdString());

    this->_connecting = true;
    emit connectingChanged();
}

void WebSocket::send(const QString &message) {
    if (this->_encrypted) {
        sendEncrypted(message);
    } else {
        throw std::invalid_argument{"Connection is not encrypted yet"};
    }
}

void WebSocket::close() { this->_ws.close(); }

QStringList WebSocket::stunServers() const { return this->_stunServers; }

QString WebSocket::publicKey() const {
    rust::String publicKey = this->_account.ed25519Key().toBase64();
    const std::string stdString{publicKey.begin(), publicKey.end()};
    return QString::fromStdString(stdString);
}

bool WebSocket::encrypted() const { return this->_encrypted; }
bool WebSocket::established() const { return this->_established; }
bool WebSocket::connected() const { return this->_ws.isOpen(); }
bool WebSocket::isDownloader() const { return this->_publicKey.has_value(); }
bool WebSocket::connecting() const { return this->_connecting; }
