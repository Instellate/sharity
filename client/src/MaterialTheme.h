//
// Created by instellate on 2026-01-22.
//

#pragma once

#include <QObject>
#include <qqmlintegration.h>

class MaterialTheme : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString accent READ accent NOTIFY accentChanged)
    Q_PROPERTY(QString background READ background NOTIFY backgroundChanged)
    Q_PROPERTY(QString foreground READ foreground NOTIFY foregroundChanged)
    Q_PROPERTY(QString primary READ primary NOTIFY primaryChanged)
    QML_ELEMENT
    QML_SINGLETON

    QString _accent = "Indigo";
    QString _background = "";
    QString _foreground = "";
    QString _primary = "Indigo";

public:
    explicit MaterialTheme(QObject *parent = nullptr);

    [[nodiscard]] QString accent() const;
    [[nodiscard]] QString background() const;
    [[nodiscard]] QString foreground() const;
    [[nodiscard]] QString primary() const;

signals:
    void accentChanged();
    void backgroundChanged();
    void foregroundChanged();
    void primaryChanged();

    private slots:
    void systemThemeChanged();
};
