#include <QApplication>
#include <QDir>
#include <QFile>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QtDebug>
#include <QtLogging>
#include <QtSystemDetection>
#include <rtc/global.hpp>

#include "android.h"

#ifdef Q_OS_WIN
#include <QSettings>
#endif

#ifdef Q_OS_WASM
#include <QFontDatabase>
#include <QStringList>
#endif

#include <iostream>

void logCallback(rtc::LogLevel level, const std::string &message);

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message);

int main(int argc, char **argv) {
    QApplication app{argc, argv};
    app.setOrganizationName("instellate");
    app.setOrganizationDomain("https://instellate.xyz");
    app.setApplicationName("Sharity");
    app.setApplicationDisplayName("Sharity");

    // The custom message handler just does 0 logging on android.
#ifndef Q_OS_ANDROID
    qInstallMessageHandler(messageHandler);
#endif

#ifdef Q_OS_WASM
    // Loads in twemoji emojis for WASM targets
    // Default emoji font in WASM does not have all the SAS emojis
    QFile twemoji{":/fonts/twemoji.ttf"};
    if (twemoji.exists()) {
        int fontId = QFontDatabase::addApplicationFont(":/fonts/twemoji.ttf");
        if (fontId == -1) {
            qWarning() << "Couldn't add twemoji font";
        } else {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            for (const QString &fontFamily: fontFamilies) {
                QFontDatabase::addApplicationEmojiFontFamily(fontFamily);
                qDebug() << "Added font family" << fontFamily;
            }
        }
    } else {
        qWarning() << "Couldn't find the resource for the twemoji file";
    }
#endif

#ifdef Q_OS_WIN
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif
    rtc::InitLogger(rtc::LogLevel::Warning, logCallback);

    QIcon::setThemeName("material");

    QStringList languageQmFiles = QDir{":/qt/qml/Sharity/i18n"}.entryList();
    QStringList languages;
    for (const QString &languageQmFile: std::as_const(languageQmFiles)) {
        QString language = languageQmFile.sliced(4, 5);
        languages.emplaceBack(std::move(language));
    }

#ifdef Q_OS_ANDROID
    contentResolverInstance();
#endif

    QQmlApplicationEngine engine{"Sharity", "MainWindow"};
    // engine.setUiLanguage(QLocale::system().name());

    QObject *main = engine.rootObjects().first();
    main->setProperty("languages", languages);
    if (argc > 1) {
        const QUrl url{argv[1]};
        qDebug() << "Got url" << url;
        main->setProperty("connectUrl", url);
    }

    return app.exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &message) {
    QString msgType;
    switch (type) {
        case QtDebugMsg:
            msgType = "Debug";
            break;
        case QtInfoMsg:
            msgType = "Info";
            break;
        case QtWarningMsg:
            msgType = "Warning";
            break;
        case QtCriticalMsg:
            msgType = "Critical";
            break;
        case QtFatalMsg:
            msgType = "Fatal";
            break;
        default:
            break;
    }
    std::cerr << '[' << qUtf8Printable(msgType) << "] " << qUtf8Printable(message) << '\n';
}

void logCallback(const rtc::LogLevel level, const std::string &message) {
    switch (level) {
        case rtc::LogLevel::None:
            break;
        case rtc::LogLevel::Fatal:
            qFatal() << message;
            break;
        case rtc::LogLevel::Error:
            qWarning() << message;
            break;
        case rtc::LogLevel::Warning:
            qWarning() << message;
            break;
        case rtc::LogLevel::Info:
            qInfo() << message;
            break;
        case rtc::LogLevel::Debug:
            qDebug() << message;
            break;
        case rtc::LogLevel::Verbose:
            qDebug() << message;
            break;
    }
}
