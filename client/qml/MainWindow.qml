import QtQuick

Window {
    id: root
    title: "Sharity"

    property bool sasConfirmed: false

    Loader {
        width: parent.width
        anchors.centerIn: parent

        source: {
            if (!WebSocket.connected) {
                return "GetOtherKey.qml";
            } else if (!WebSocket.encrypted) {
                return "HandshakeStatus.qml";
            } else if (!root.sasConfirmed) {
                return "Sas.qml";
            } else if (WebSocket.isDownloader) {
                return "Downloader.qml";
            } else {
                return "Uploader.qml"
            }
        }

        onLoaded: {
            if (item instanceof Sas) {
                root.sasConfirmed = Qt.binding(() => item.sasConfirmed);
            }
        }
    }
}
