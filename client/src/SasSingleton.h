#pragma once

#include <QList>
#include <QObject>
#include <QtQmlIntegration>
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
#include <vodozemac.h>

static QStringList emojiTable{"🐶", "🐱", "🦁", "🐎", "🦄", "🐷", "🐘", "🐰", "🐼", "🐓", "🐧", "🐢", "🐟",
                              "🐙", "🦋", "🌷", "🌳", "🌵", "🍄", "🌏", "🌙", "☁",  "🔥", "🍌", "🍎", "🍓",
                              "🌽", "🍕", "🎂", "❤",  "😀", "🤖", "🎩", "👓", "🔧", "🎅", "👍", "☂",  "⌛",
                              "⏰", "🎁", "💡", "📕", "✏",  "📎", "✂",  "🔒", "🔑", "🔨", "☎",  "🏁", "🚂",
                              "🚲", "✈",  "🚀", "🏆", "⚽", "🎸", "🎺", "🔔", "⚓", "🎧", "📁", "📌"};

class SasSingleton : public QObject {
    Q_OBJECT
    QML_SINGLETON
    QML_ELEMENT
    Q_PROPERTY(bool isEstablished READ isEstablished NOTIFY isEstablishedChanged)
    Q_PROPERTY(QString publicKey READ publicKey NOTIFY publicKeyChanged)
    Q_PROPERTY(QString decimals READ decimals NOTIFY decimalsChanged)
    Q_PROPERTY(QString emojis READ emojis NOTIFY emojisChanged)

    vodozemac::sas::Sas _sas;

public:
    SasSingleton(QObject *parent = nullptr) : QObject(parent) {}

    bool isEstablished() const { return this->_sas.isEstablished(); }

    QString publicKey() const { return QString{this->_sas.publicKey().toBase64().data()}; }

    Q_INVOKABLE void diffieHellman(const QString &base64Key) {
        QByteArray utf8 = base64Key.toUtf8();
        const auto key = vodozemac::Curve25519PublicKey::fromBase64(utf8.data());
        this->_sas.diffieHellman(key);
        emit isEstablishedChanged();
    }

    QString decimals() {
        QStringList list;
        auto decimals = this->_sas.bytes("wow").decimals();
        for (const auto decimal: decimals) {
            list.emplace_back(QString::number(decimal));
        }

        return list.join(", ");
    }

    QString emojis() {
        QStringList list;
        auto emojis = this->_sas.bytes("wow").emojiIndices();
        for (const auto emoji: emojis) {
            list.emplace_back(emojiTable[emoji]);
        }

        return list.join(", ");
    }

signals:
    void isEstablishedChanged();

    // Unused, only present to prevent binding errors
    void publicKeyChanged();
    void decimalsChanged();
    void emojisChanged();
};
