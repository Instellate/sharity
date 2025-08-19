#include "DownloaderPeer.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtLogging>
#include "Websocket.h"

DownloaderPeer::DownloaderPeer() {
    rtc::Configuration config;
    const auto stunServers = WebSocket::instance()->stunServers();
    for (const QString &server: stunServers) {
        config.iceServers.emplace_back(server.toStdString());
    }

    this->_peer = std::make_shared<rtc::PeerConnection>(config);
    this->_peer->onDataChannel([this](const std::shared_ptr<rtc::DataChannel> &dc) {
        this->_channel = dc;
        qInfo() << "Got data channel:" << dc->label();
        dc->onMessage([](const std::variant<rtc::binary, rtc::string> &message) {
            qInfo() << "Received message";
            if (std::holds_alternative<rtc::string>(message)) {
                qInfo() << "Received:" << get<rtc::string>(message);
            }
        });
    });

    this->_peer->onLocalDescription([](const rtc::Description &sdp) {
        const QJsonObject response{{"type", "rtc_answer"}, {"answer", QString::fromStdString(sdp)}};
        WebSocket::instance()->send(QJsonDocument{response}.toJson());
    });
    this->_peer->onLocalCandidate([](const rtc::Candidate &candidate) {
        qInfo() << "Got candidate" << std::string{candidate};
        const QJsonObject json{{"type", "rtc_candidate"}, {"candidate", QString::fromStdString(candidate)}};
        WebSocket::instance()->send(QJsonDocument{json}.toJson());
    });

    connect(WebSocket::instance(), &WebSocket::message, this, &DownloaderPeer::wsMessage);
}

void DownloaderPeer::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "rtc_offer") {
        const rtc::Description offer = json["offer"].toString().toStdString();
        this->_peer->setRemoteDescription(offer);
    } else if (type == "rtc_candidate") {
        rtc::Candidate candidate = json["candidate"].toString().toStdString();
        this->_peer->addRemoteCandidate(candidate);
    }
}
