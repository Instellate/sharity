#include "UploaderPeer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include "Websocket.h"

QUrl UploaderPeer::selectedFile() const { return this->_selectedFile; }
void UploaderPeer::setSelectedFile(QUrl url) { 
    this->_selectedFile = url; 
    emit selectedFileChanged();
}

void UploaderPeer::startFileNegotiation() {
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
        qInfo() << "Got candidate" << std::string{candidate};
        const QJsonObject json{{"type", "rtc_candidate"}, {"candidate", QString::fromStdString(candidate)}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    });

    connect(WebSocket::instance(), &WebSocket::message, this, &UploaderPeer::wsMessage);

    // A channel needs to be created for establishment to be made with libdatachannel
    // TODO: Have the first datachannel be a file stream
    DataChannel channel = this->_peer->createDataChannel("establish");
    channel->onOpen([channel] {
        qInfo() << "Data channel open";
        channel->send("Hello!");
    });
    channel->onClosed([channel] { qInfo() << "Data channel" << channel->label() << "has been closed"; });
    this->_channels.emplace_back(std::move(channel));
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UploaderPeer::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "rtc_answer") {
        const rtc::Description answer = json["answer"].toString().toStdString();
        this->_peer->setRemoteDescription(answer);
    } else if (type == "rtc_candidate") {
        const rtc::Candidate candidate = json["candidate"].toString().toStdString();
        this->_peer->addRemoteCandidate(candidate);
    }
}
