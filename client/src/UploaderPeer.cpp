#include "UploaderPeer.h"

#include <QJsonDocument>
#include <QtConcurrent>

#include <exception>
#include <stdexcept>

#include "WebSocket.h"

void UploaderPeer::startRtcNegotiation() {
    if (!this->_peer) {
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
    }

    DataChannel channel = this->_peer->createDataChannel(this->_selectedFile.fileName().toStdString());
    channel->onOpen([this, channel] {
        qDebug() << "Data channel" << channel->label() << "for uploading was opened";
        this->_streamFuture = QtConcurrent::run(&UploaderPeer::handleFileUpload, this, channel);
    });
    channel->onClosed([channel, this] {
        qDebug() << "Data channel" << channel->label() << "has been closed";
        this->_streamFuture.cancel();
    });

    this->_timer->start();
}

void UploaderPeer::handleFileUpload(const DataChannel &channel) {
    const QString localFile = this->_selectedFile.toLocalFile();
    qDebug() << "Starting file upload for file:" << localFile;

    QFile file{localFile};
    if (!file.open(QIODeviceBase::ReadOnly)) {
        qWarning() << "Cannot open file:" << localFile;
        return;
    }

    this->_fileSize = file.size();
    emit fileSizeChanged();

    const auto bufferSize = static_cast<qint64>(channel->maxMessageSize());
    const auto buffer = new char[bufferSize];

    auto promise = std::make_shared<std::promise<void>>();
    channel->setBufferedAmountLowThreshold(bufferSize * 2);
    channel->onBufferedAmountLow([promise] { promise->set_value(); });
    channel->onClosed([promise] {
        qDebug() << "Channel got closed. Throwing exception for uploader promise";
        promise->set_exception(std::make_exception_ptr(std::runtime_error{"Stream cancelled"}));
    });
    this->_streamFuture.onCanceled([promise] {
        qDebug() << "Stream future got cancelled. Throwing exception for uploader promise";
        promise->set_exception(std::make_exception_ptr(std::runtime_error{"Stream cancelled"}));
    });

    bool failed = false;
    try {
        while (!file.atEnd() && !this->_streamFuture.isCanceled() && !channel->isClosed()) {
            const qint64 dataRead = file.read(buffer, bufferSize);
            channel->send(reinterpret_cast<std::byte *>(buffer), dataRead);

            this->_amountUploaded += dataRead;
            emit amountUploadedChanged();
            this->_uploadedSinceTick += dataRead;

            if (channel->bufferedAmount() > static_cast<size_t>(bufferSize) * 10) {
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
    this->_timer->stop();
    qDebug() << "Finished uploading file";
}

UploaderPeer::UploaderPeer(QObject *parent) : QObject(parent) {
    using namespace std::chrono_literals;

    qDebug() << "Initializing UploaderPeer";
    connect(WebSocket::instance(), &WebSocket::message, this, &UploaderPeer::wsMessage);

    this->_timer = new QTimer{this};
    connect(this->_timer, &QTimer::timeout, this, &UploaderPeer::tick);
    this->_timer->setInterval(1s);
}

UploaderPeer::~UploaderPeer() {
    this->_streamFuture.cancel();
    this->_streamFuture.waitForFinished();
}

qint64 UploaderPeer::amountUploaded() const { return this->_amountUploaded; }
qint64 UploaderPeer::fileSize() const { return this->_fileSize; }
qint64 UploaderPeer::speed() const { return this->_speed; }

QUrl UploaderPeer::selectedFile() const { return this->_selectedFile; }
void UploaderPeer::setSelectedFile(const QUrl &url) {
    this->_selectedFile = url;
    emit selectedFileChanged();
}

void UploaderPeer::startFileNegotiation() const {
#ifdef Q_OS_ANDROID
    const QFile file{this->_selectedFile.toString()};
#else
    const QFile file{this->_selectedFile.toLocalFile()};
#endif

    qDebug() << "Starting file negotiation for file" << this->_selectedFile.fileName();
    const QJsonObject json{{"type", "stream_request"}, {"stream_type", "upload"}, {"size", file.size()}};
    WebSocket::instance()->send(QJsonDocument{json}.toJson());
}

void UploaderPeer::close() { this->_streamFuture.cancel(); }

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

void UploaderPeer::tick() {
    this->_speed = this->_uploadedSinceTick.fetchAndStoreAcquire(0);
    emit speedChanged();
}
