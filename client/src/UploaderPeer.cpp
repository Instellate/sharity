#include "UploaderPeer.h"

#include <QJsonDocument>
#include <QJsonObject>
#include "Websocket.h"

UploaderPeer::UploaderPeer() {
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
    this->_channel = this->_peer->createDataChannel("test");
    this->_channel->onOpen([this] {
        qInfo() << "Data channel open";
        this->_channel->send("Hello!");
    });
    this->_channel->onClosed([] { qInfo() << "Data channel closed"; });
}

void UploaderPeer::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "rtc_answer") {
        const rtc::Description answer = json["answer"].toString().toStdString();
        this->_peer->setRemoteDescription(answer);
    } else if (type == "rtc_candidate") {
        rtc::Candidate candidate = json["candidate"].toString().toStdString();
        this->_peer->addRemoteCandidate(candidate);
    }
}
