#include <QApplication>
#include <QFile>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtLogging>
#include <QtSystemDetection>

#ifdef Q_OS_WASM
#include <QFontDatabase>
#include <QStringList>
#endif

#include <rtc/rtc.hpp>

int main(int argc, char **argv) {
    QApplication app{argc, argv};
    app.setOrganizationName("instellate");
    app.setOrganizationDomain("https://instellate.xyz");
    app.setApplicationName("Sharity");
    app.setApplicationDisplayName("Sharity");

    QQmlEngine e;
    QQmlComponent component{&e};
    component.loadFromModule("Sharity", "MainWindow");

    QObject *object = component.create();
    if (object == nullptr) {
        qFatal() << "Component creation failed. Get error: \n" << qPrintable(component.errorString());
    }

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

    QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
    window->show();

    return app.exec();
}
