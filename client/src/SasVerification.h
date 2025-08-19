#pragma once

#include <QJsonObject>
#include <QObject>
#include <qqmlintegration.h>

#include "vodozemac.h"

class SasVerification : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool sasEstablished READ sasEstablished NOTIFY sasEstablishedChanged)
    Q_PROPERTY(bool sasConfirmed READ sasConfirmed NOTIFY sasConfirmedChanged)
    Q_PROPERTY(bool otherSasConfirmed READ otherSasConfirmed NOTIFY otherSasConfirmedChanged)
    Q_PROPERTY(QString emojis READ emojis)
    Q_PROPERTY(QString decimals READ decimals)
    QML_ELEMENT

    vodozemac::sas::Sas _sas;

    bool _sasConfirmed = false;
    bool _otherSasConfirmed = false;

public:
    explicit SasVerification(QObject *parent = nullptr);

    [[nodiscard]] bool sasEstablished() const;
    [[nodiscard]] bool sasConfirmed() const;
    [[nodiscard]] bool otherSasConfirmed() const;
    [[nodiscard]] QString emojis() const;
    [[nodiscard]] QString decimals() const;

    Q_INVOKABLE void confirmSas();
    Q_INVOKABLE void declineSas();

signals:
    void sasEstablishedChanged();
    void sasConfirmedChanged();
    void otherSasConfirmedChanged();

private slots:
    void wsMessage(const QString &type, const QJsonObject &json);
};
