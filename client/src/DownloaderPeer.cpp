#include "DownloaderPeer.h"

#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QtLogging>

#include "Websocket.h"

void DownloaderPeer::handleDataChannel(const DataChannel &channel) {
    qDebug() << "Got data channel" << channel->label();
    const QDir downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    auto file = QSharedPointer<QFile>::create(downloadPath.filePath(QString::fromStdString(channel->label())));

    file->open(QIODeviceBase::WriteOnly, QFileDevice::ReadUser | QFileDevice::WriteUser);

    channel->onClosed([this, channel] {
        qInfo() << "Data channel " << channel->label() << "got closed";
        std::erase(this->_channels, channel); 
    });
    channel->onMessage([this, file](const std::variant<rtc::binary, rtc::string> &message) {
        if (std::holds_alternative<rtc::string>(message)) {
            qWarning() << "Got string from datachannel when expected binary. Ignoring this message";
            return;
        }

        auto binary = std::get<rtc::binary>(message);

        this->_amountDownloaded += binary.size();
        this->_downloadedSinceTick += binary.size();

        const auto data = reinterpret_cast<char *>(binary.data());
        file->write(data, static_cast<qint64>(binary.size()));
    });
}

DownloaderPeer::DownloaderPeer(QObject *parent) : QObject(parent) {
    using namespace std::chrono_literals;
    qDebug() << "Initializing DownloaderPeer";
    connect(WebSocket::instance(), &WebSocket::message, this, &DownloaderPeer::wsMessage);

    this->_timer = new QTimer{this};
    connect(this->_timer, &QTimer::timeout, this, &DownloaderPeer::tick);
    this->_timer->setInterval(1s);

    rtc::Configuration config;
    const auto stunServers = WebSocket::instance()->stunServers();
    for (const QString &server: stunServers) {
        config.iceServers.emplace_back(server.toStdString());
    }

    this->_peer = std::make_shared<rtc::PeerConnection>(config);
    this->_peer->onDataChannel([this](const std::shared_ptr<rtc::DataChannel> &dc) { handleDataChannel(dc); });

    this->_peer->onLocalDescription([](const rtc::Description &sdp) {
        const QJsonObject response{{"type", "rtc_answer"}, {"answer", QString::fromStdString(sdp)}};
        WebSocket::instance()->send(QJsonDocument{response}.toJson());
    });

    this->_peer->onLocalCandidate([](const rtc::Candidate &candidate) {
        const QJsonObject json{{"type", "rtc_candidate"}, {"candidate", QString::fromStdString(candidate)}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    });
}

DownloaderPeer::~DownloaderPeer() {
    for (const auto &channel: this->_channels) {
        channel->close();
    }
}

DownloaderPeer::State DownloaderPeer::downloadState() const { return this->_state; }
qint64 DownloaderPeer::fileSize() const { return this->_fileSize; }
qint64 DownloaderPeer::amountDownloaded() const { return this->_amountDownloaded; }
qint64 DownloaderPeer::speed() const { return this->_speed; }

// ReSharper disable once CppMemberFunctionMayBeConst
void DownloaderPeer::close() {
    for (const auto &channel: this->_channels) {
        channel->close();
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void DownloaderPeer::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "rtc_offer") {
        const rtc::Description offer = json["offer"].toString().toStdString();
        this->_peer->setRemoteDescription(offer);
    } else if (type == "rtc_candidate") {
        const rtc::Candidate candidate = json["candidate"].toString().toStdString();
        this->_peer->addRemoteCandidate(candidate);
    } else if (type == "stream_request") {
        qDebug() << "Stream requested";
        if (json["stream_type"] != "upload") {
            const QJsonObject resp{{"type", "stream_error"}, {"error_type", "unsupported_stream"}};
            WebSocket::instance()->send(QJsonDocument{resp}.toJson());
            qDebug() << "Got unsupported stream type" << json["stream_type"];
            return;
        }

        this->_fileSize = json["size"].toInteger();
        this->_state = Downloading;
        emit fileSizeChanged();
        emit downloadStateChanged();

        const QJsonObject resp{{"type", "stream_accept"}};
        WebSocket::instance()->send(QJsonDocument{resp}.toJson());
        this->_timer->start();
    } else if (type == "stream_completed") {
        this->_timer->stop();
        this->_state = Downloaded;
        emit downloadStateChanged();
    }
}

void DownloaderPeer::tick() {
    this->_speed = this->_downloadedSinceTick.fetchAndStoreAcquire(0);
    emit speedChanged();
    emit amountDownloadedChanged();
}
