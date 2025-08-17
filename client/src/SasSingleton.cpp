#include "SasSingleton.h"

static QStringList emojiTable{"🐶", "🐱", "🦁", "🐎", "🦄", "🐷", "🐘", "🐰", "🐼", "🐓", "🐧", "🐢", "🐟",
                              "🐙", "🦋", "🌷", "🌳", "🌵", "🍄", "🌏", "🌙", "☁",  "🔥", "🍌", "🍎", "🍓",
                              "🌽", "🍕", "🎂", "❤",  "😀", "🤖", "🎩", "👓", "🔧", "🎅", "👍", "☂",  "⌛",
                              "⏰", "🎁", "💡", "📕", "✏",  "📎", "✂",  "🔒", "🔑", "🔨", "☎",  "🏁", "🚂",
                              "🚲", "✈",  "🚀", "🏆", "⚽", "🎸", "🎺", "🔔", "⚓", "🎧", "📁", "📌"};

bool SasSingleton::isEstablished() const { return this->_sas.isEstablished(); }

QString SasSingleton::publicKey() const { return QString{this->_sas.publicKey().toBase64().data()}; }

void SasSingleton::diffieHellman(const QString &base64Key) {
    QByteArray utf8 = base64Key.toUtf8();
    const auto key = vodozemac::Curve25519PublicKey::fromBase64(utf8.data());
    this->_sas.diffieHellman(key);
    emit isEstablishedChanged();
}

QString SasSingleton::decimals() {
    QStringList list;
    auto decimals = this->_sas.bytes("wow").decimals();
    for (const auto decimal: decimals) {
        list.emplace_back(QString::number(decimal));
    }

    return list.join(", ");
}

QString SasSingleton::emojis() {
    QStringList list;
    auto emojis = this->_sas.bytes("wow").emojiIndices();
    for (const auto emoji: emojis) {
        list.emplace_back(emojiTable[emoji]);
    }

    return list.join(", ");
}

