#include "SasVerification.h"

#include <QJsonObject>
#include <QJsonDocument>
#include "Websocket.h"

static QStringList emojiTable{"🐶", "🐱", "🦁", "🐎", "🦄", "🐷", "🐘", "🐰", "🐼", "🐓", "🐧", "🐢", "🐟",
                              "🐙", "🦋", "🌷", "🌳", "🌵", "🍄", "🌏", "🌙", "☁",  "🔥", "🍌", "🍎", "🍓",
                              "🌽", "🍕", "🎂", "❤",  "😀", "🤖", "🎩", "👓", "🔧", "🎅", "👍", "☂",  "⌛",
                              "⏰", "🎁", "💡", "📕", "✏",  "📎", "✂",  "🔒", "🔑", "🔨", "☎",  "🏁", "🚂",
                              "🚲", "✈",  "🚀", "🏆", "⚽", "🎸", "🎺", "🔔", "⚓", "🎧", "📁", "📌"};


SasVerification::SasVerification(QObject *parent) : QObject(parent) {
    const auto ws = WebSocket::instance();
    connect(ws, &WebSocket::message, this, &SasVerification::wsMessage, Qt::QueuedConnection);

    if (ws->isDownloader()) {
        const std::string sasToken = vodozemac::rustToStdString(this->_sas.publicKey().toBase64());
        const QJsonObject json{{"type", "sas_token"}, {"sas_token", QString::fromStdString(sasToken)}};
        const QJsonDocument doc{json};
        ws->send(doc.toJson(QJsonDocument::Compact));
    }
}

bool SasVerification::sasEstablished() const { return this->_sas.isEstablished(); }
bool SasVerification::sasConfirmed() const { return this->_sasConfirmed; }
bool SasVerification::otherSasConfirmed() const { return this->_otherSasConfirmed; }

QString SasVerification::emojis() const {
    QStringList list;
    auto emojis = this->_sas.bytes("wow").emojiIndices();
    for (const auto emoji: emojis) {
        list.emplace_back(emojiTable[emoji]);
    }

    return list.join(", ");
}

QString SasVerification::decimals() const {
    QStringList list;
    auto decimals = this->_sas.bytes("sassy").decimals();
    for (const auto decimal: decimals) {
        list.emplace_back(QString::number(decimal));
    }

    return list.join(", ");
}

void SasVerification::confirmSas() {
    this->_sasConfirmed = true;
    emit sasConfirmedChanged();

    QJsonObject json{{"type", "sas_confirmed"}};
    WebSocket::instance()->send(QJsonDocument{std::move(json)}.toJson());
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void SasVerification::declineSas() { WebSocket::instance()->close(); } // NOLINT

void SasVerification::wsMessage(const QString &type, const QJsonObject &json) {
    if (type == "sas_token") {
        if (this->_sas.isEstablished()) {
            qWarning() << "Got duplicate sas_token when it already is established";
            return;
        }

        if (!json.contains("sas_token") || !json["sas_token"].isString()) {
            qWarning() << "Got invalid sas token";
            return;
        }

        const QString sasToken = json["sas_token"].toString();
        try {
            const auto key = vodozemac::Curve25519PublicKey::fromBase64(sasToken.toStdString());
            this->_sas.diffieHellman(key);
            emit sasEstablishedChanged();
        } catch (std::exception &e) {
            qWarning() << "Got error when trying to establish sas:" << e.what();
        }

        if (!WebSocket::instance()->isDownloader()) {
            const std::string mySasToken = vodozemac::rustToStdString(this->_sas.publicKey().toBase64());
            const QJsonObject response{{"type", "sas_token"}, {"sas_token", QString::fromStdString(mySasToken)}};
            const QJsonDocument doc{response};
            WebSocket::instance()->send(doc.toJson(QJsonDocument::Compact));
        }
    } else if (type == "sas_confirmed") {
        this->_otherSasConfirmed = true;
        emit otherSasConfirmedChanged();
    }
}
