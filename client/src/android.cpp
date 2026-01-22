//
// Created by instellate on 2026-01-20.
//

#include "android.h"

#ifdef Q_OS_ANDROID

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QStyleHints>
#include <QUrl>

QString getCaCertificate() {
    QDir appConfig{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
    QFile previousCaCert{appConfig.filePath("cacert.pem")};
    if (appConfig.exists("cacert.pem")) {
        appConfig.remove("cacert.pem");
    }

    if (!appConfig.exists()) {
        appConfig.cdUp();
        // ReSharper disable once CppExpressionWithoutSideEffects
        appConfig.mkpath("settings");
        appConfig.cd("settings");
    }

    QFile caCert{":/resources/cacert.pem"};
    caCert.copy(appConfig.filePath("cacert.pem"));

    if (!appConfig.exists("cacert.pem")) {
        qFatal() << "cacerts.pem does not exist";
    }

    qputenv("SSL_CERT_FILE", appConfig.filePath("cacert.pem").toUtf8());

    return appConfig.filePath("cacert.pem");
}

QJniObject &contentResolverInstance() {
    static QJniObject contentResolver;
    if (!contentResolver.isValid()) {
        contentResolver = QJniObject(QNativeInterface::QAndroidApplication::context())
                                  .callMethod<QtJniTypes::ContentResolver>("getContentResolver");
    }

    return contentResolver;
}

struct JniContentValues {
    QJniObject object{"android/content/ContentValues"};

    void put(const QString &key, const QString &value) const {
        object.callMethod<void>("put",
                                "(Ljava/lang/String;Ljava/lang/String;)V",
                                QJniObject::fromString(key).object<jstring>(),
                                QJniObject::fromString(value).object<jstring>());
    }

    void put(const QString &key, const qint32 value) const {
        QJniObject integer = QJniObject::callStaticMethod<QtJniTypes::Integer>(
                "java/lang/Integer", "valueOf", "(I)Ljava/lang/Integer;", jint(value));

        object.callMethod<void>("put",
                                "(Ljava/lang/String;Ljava/lang/Integer;)V",
                                QJniObject::fromString(key).object<jstring>(),
                                integer.object<jobject>());
    }
};

#define MEDIA_COLUMN_IS_PENDING "is_pending"
#define MEDIA_COLUMN_DISPLAY_NAME "_display_name"
#define MEDIA_COLUMN_MIME_TYPE "mime_type"

QJniObject getDownloadUri(const QString &fileName) {
    if (!QNativeInterface::QAndroidApplication::isActivityContext()) {
        qFatal() << "Non activity context";
    }

    QJniObject externalContentUri = QJniObject::getStaticField<QtJniTypes::Uri>("android/provider/MediaStore$Downloads",
                                                                                "EXTERNAL_CONTENT_URI");

    QJniObject directoryDownloads =
            QJniObject::getStaticField<jstring>("android/os/Environment", "DIRECTORY_DOWNLOADS");

    const QMimeDatabase db{};
    const QMimeType fileType = db.mimeTypeForFile(fileName);
    qDebug() << "Mime type for download file is" << fileType.name();

    JniContentValues contentValues{};
    contentValues.put(MEDIA_COLUMN_DISPLAY_NAME, fileName);
    contentValues.put(MEDIA_COLUMN_MIME_TYPE, fileType.name());
    contentValues.put(MEDIA_COLUMN_IS_PENDING, 1);

    const QJniObject contentResolver = contentResolverInstance();
    const QJniObject androidUri =
            contentResolver.callMethod<jobject>("insert",
                                                "(Landroid/net/Uri;Landroid/content/ContentValues;)Landroid/net/Uri;",
                                                externalContentUri,
                                                contentValues.object);
    if (!androidUri.isValid()) {
        qFatal() << "ContentResolver returned null URI";
    }

    return androidUri;
}

QJniObject getAndroidUri(const QString &url) {
    return QJniObject::callStaticMethod<jobject>(
            "android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;", QJniObject::fromString(url));
}

void urlDonePending(const QJniObject &uri) {
    JniContentValues contentValues;
    contentValues.put(MEDIA_COLUMN_IS_PENDING, 0);

    const QJniObject contentResolver = contentResolverInstance();
    const qint32 updated = contentResolver.callMethod<jint>(
            "update",
            "(Landroid/net/Uri;Landroid/content/ContentValues;Ljava/lang/String;[Ljava/lang/String;)I",
            uri,
            contentValues.object,
            nullptr,
            nullptr);
    qDebug() << "Updated with not pending:" << updated;
}

constexpr qint32 SYSTEM_PRIMARY_DARK = 0x0106008b;
constexpr qint32 SYSTEM_PRIMARY_LIGHT = 0x01060060;
constexpr qint32 SYSTEM_BACKGROUND_DARK = 0x01060095;
constexpr qint32 SYSTEM_BACKGROUND_LIGHT = 0x0106006a;

constexpr qint32 TEXT_COLOR_PRIMARY = 0x01010036;
constexpr qint32 COLOR_ACCENT = 0x01010435;

MaterialThemeColors getMaterialTheme34() {
    QJniObject context = QNativeInterface::QAndroidApplication::context();
    QJniObject theme = context.callMethod<jobject>("getTheme", "()Landroid/content/res/Resources$Theme;");
    QJniObject resources = context.callMethod<jobject>("getResources", "()Landroid/content/res/Resources;");

    auto getColor = [&](qint32 id) {
        return resources.callMethod<jint>("getColor", "(ILandroid/content/res/Resources$Theme;)I", jint(id), theme);
    };

    Qt::ColorScheme colorScheme = QApplication::styleHints()->colorScheme();

    QRgb primary;
    QRgb background;
    if (colorScheme == Qt::ColorScheme::Dark) {
        primary = getColor(SYSTEM_PRIMARY_DARK);
        background = getColor(SYSTEM_BACKGROUND_DARK);
    } else {
        primary = getColor(SYSTEM_PRIMARY_LIGHT);
        background = getColor(SYSTEM_BACKGROUND_LIGHT);
    }

    QJniObject typedValue{"android/util/TypedValue"};
    theme.callMethod<jboolean>(
            "resolveAttribute", "(ILandroid/util/TypedValue;Z)Z", TEXT_COLOR_PRIMARY, typedValue, true);
    QRgb foreground = context.callMethod<jint>("getColor", "(I)I", typedValue.getField<jint>("resourceId"));

    theme.callMethod<jboolean>("resolveAttribute", "(ILandroid/util/TypedValue;Z)Z", COLOR_ACCENT, typedValue, true);
    QRgb accent = context.callMethod<jint>("getColor", "(I)I", typedValue.getField<jint>("resourceId"));

    QColor primaryColor{primary};
    QColor accentColor{accent};
    QColor foregroundColor{foreground};
    QColor backgroundColor{background};

    return MaterialThemeColors{.accent = accentColor.name(),
                               .background = backgroundColor.name(),
                               .foreground = foregroundColor.name(),
                               .primary = primaryColor.name()};
}

MaterialThemeColors getMaterialTheme() {
    if (QNativeInterface::QAndroidApplication::sdkVersion() >= 34) {
        return getMaterialTheme34();
    }

    return MaterialThemeColors{};
}

int AndroidContentFile::openFd(const QJniObject &uri) {
    const QJniObject contentResolver = contentResolverInstance();
    QJniObject fileDescriptor = contentResolver.callMethod<jobject>(
            "openFileDescriptor",
            "(Landroid/net/Uri;Ljava/lang/String;)Landroid/os/ParcelFileDescriptor;",
            uri.object<jobject>(),
            QJniObject::fromString("rw").object<jobject>());

    const int fd = fileDescriptor.callMethod<jint>("detachFd");
    qDebug() << "Got fd" << fd << "for url" << uri.callMethod<jstring>("toString").toString();
    return fd;
}

bool AndroidContentFile::openDownloadFile(const QString &fileName) {
    this->_contentUri = getDownloadUri(fileName);
    this->_isPending = true;

    const QJniObject contentResolver = contentResolverInstance();
    const int fd = openFd(this->_contentUri);

    return open(fd, ReadWrite, AutoCloseHandle);
}

bool AndroidContentFile::openContentUri(const QUrl &url) {
    const QJniObject uri = getAndroidUri(url.toString());
    const int fd = openFd(uri);

    return open(fd, ReadWrite, AutoCloseHandle);
}

void AndroidContentFile::close() {
    if (this->_isPending) {
        urlDonePending(this->_contentUri);
    }
    QFile::close();
}

#endif
