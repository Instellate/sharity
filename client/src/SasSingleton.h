#pragma once

#include <QList>
#include <QObject>
#include <QtQmlIntegration>
#include <qcontainerfwd.h>
#include <qtmetamacros.h>
#include <vodozemac.h>

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

    bool isEstablished() const;

    QString publicKey() const;

    Q_INVOKABLE void diffieHellman(const QString &base64Key);

    QString decimals();

    QString emojis();

signals:
    void isEstablishedChanged();

    // Unused, only present to prevent binding errors
    void publicKeyChanged();
    void decimalsChanged();
    void emojisChanged();
};
