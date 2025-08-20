#include "UploaderPeer.h"

#include <QJsonDocument>
#include <QtConcurrent>
#include "Websocket.h"

void UploaderPeer::startRtcNegotiation() {
    rtc::Configuration config;
    const auto stunServers = WebSocket::instance()->stunServers();
    for (const QString &server: stunServers) {
        config.iceServers.emplace_back(server.toStdString());
    }

    this->_peer = std::make_shared<rtc::PeerConnection>(config);
    this->_peer->onLocalDescription([](const rtc::Description &sdp) {
        QString offer = QString::fromStdString(sdp);
        const QJsonObject json{{"type", "rtc_offer"}, {"offer", offer}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    });
    this->_peer->onLocalCandidate([](const rtc::Candidate &candidate) {
        const QJsonObject json{{"type", "rtc_candidate"}, {"candidate", QString::fromStdString(candidate)}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    });

    DataChannel channel = this->_peer->createDataChannel(this->_selectedFile.fileName().toStdString());
    channel->onOpen([this, channel] {
        this->_streamFuture = QtConcurrent::run(&UploaderPeer::handleFileUpload, this, channel);
    });
    channel->onClosed([channel] { qDebug() << "Data channel" << channel->label() << "has been closed"; });
}

void UploaderPeer::handleFileUpload(const DataChannel &channel) {
    const QString localFile = this->_selectedFile.toLocalFile();
    qDebug() << "Starting file upload for file:" << localFile;
    QFile file{localFile};
    file.open(QIODeviceBase::ReadOnly);

    const qint64 bufferSize = channel->maxMessageSize();
    auto buffer = new char[channel->maxMessageSize()];

    bool failed = false;
    try {
        while (!file.atEnd() && !this->_streamFuture.isCanceled()) {
            const qint64 dataRead = file.read(buffer, bufferSize);
            channel->send(reinterpret_cast<std::byte *>(buffer), dataRead);
        }
    } catch (std::exception &e) {
        qWarning() << "Got error" << e.what() << "when trying to upload file";
        QJsonDocument json{{"type", "stream_error"}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
        failed = true;
    }

    if (!failed) {
        emit fileUploaded();
        QJsonObject json{{"type", "stream_completed"}, {"label", QString::fromStdString(channel->label())}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    }

    delete[] buffer;
}

UploaderPeer::UploaderPeer(QObject *parent) : QObject(parent) {
    qDebug() << "Initializing UploaderPeer";
    connect(WebSocket::instance(), &WebSocket::message, this, &UploaderPeer::wsMessage);
}

UploaderPeer::~UploaderPeer() {
    this->_streamFuture.cancel();
    this->_streamFuture.waitForFinished();
}

QUrl UploaderPeer::selectedFile() const { return this->_selectedFile; }
void UploaderPeer::setSelectedFile(const QUrl &url) {
    this->_selectedFile = url;
    emit selectedFileChanged();
}

void UploaderPeer::startFileNegotiation() const {
    qDebug() << "Starting file negotiation for file" << this->_selectedFile.fileName();
    const QJsonObject json{
            {"type", "stream_request"},
            {"stream_type", "upload"},
    };
    WebSocket::instance()->send(QJsonDocument{json}.toJson());
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UploaderPeer::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "rtc_answer") {
        const rtc::Description answer = json["answer"].toString().toStdString();
        this->_peer->setRemoteDescription(answer);
    } else if (type == "rtc_candidate") {
        const rtc::Candidate candidate = json["candidate"].toString().toStdString();
        this->_peer->addRemoteCandidate(candidate);
    } else if (type == "stream_accept") {
        this->startRtcNegotiation();
    }
}
