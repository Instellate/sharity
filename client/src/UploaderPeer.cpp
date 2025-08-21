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
        qDebug() << "Data channel" << channel->label() << "for uploading was opened";
        this->_streamFuture = QtConcurrent::run(&UploaderPeer::handleFileUpload, this, channel);
    });
    channel->onClosed([channel] { qDebug() << "Data channel" << channel->label() << "has been closed"; });
}

void UploaderPeer::handleFileUpload(const DataChannel &channel) {
    const QString localFile = this->_selectedFile.toLocalFile();
    qDebug() << "Starting file upload for file:" << localFile;
    QFile file{localFile};
    file.open(QIODeviceBase::ReadOnly);

    constexpr size_t minimumBufferSize = 1024 * 1024 * 5;
    auto promise = std::make_shared<std::promise<void>>();
    channel->setBufferedAmountLowThreshold(minimumBufferSize);
    channel->onBufferedAmountLow([promise] { promise->set_value(); });

    const auto bufferSize = static_cast<qint64>(channel->maxMessageSize());
    const auto buffer = new char[channel->maxMessageSize()];

    bool failed = false;
    try {
        while (!file.atEnd() && !this->_streamFuture.isCanceled()) {
            const qint64 dataRead = file.read(buffer, bufferSize);
            channel->send(reinterpret_cast<std::byte *>(buffer), dataRead);
            if (channel->bufferedAmount() > minimumBufferSize * 2) {
                promise->get_future().wait();
                *promise = std::promise<void>{};
            }
        }
    } catch (std::exception &e) {
        qWarning() << "Got error" << e.what() << "when trying to upload file";
        const QJsonDocument json{{"type", "stream_error"}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
        failed = true;
    }

    if (!failed) {
        emit fileUploaded();
        const QJsonObject json{{"type", "stream_completed"}, {"label", QString::fromStdString(channel->label())}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    }

    delete[] buffer;
    channel->close();
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
    QFile file{this->_selectedFile.toLocalFile()};

    qDebug() << "Starting file negotiation for file" << this->_selectedFile.fileName();
    const QJsonObject json{{"type", "stream_request"}, {"stream_type", "upload"}, {"size", file.size()}};
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
