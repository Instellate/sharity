import QtQuick
import QtQuick.Controls.Material

ApplicationWindow {
    id: root
    title: "Sharity"
    width: 500
    height: 500

    Material.theme: Material.System
    Material.accent: Material.Indigo

    property bool sasConfirmed: false

    Loader {
        anchors.centerIn: parent

        source: {
            if (!WebSocket.connected) {
                return "Connect.qml";
            } else if (!WebSocket.encrypted) {
                return "HandshakeStatus.qml";
            } else if (!root.sasConfirmed) {
                return "Sas.qml";
            } else if (WebSocket.isDownloader) {
                return "Downloader.qml";
            } else {
                return "Uploader.qml";
            }
        }

        onLoaded: {
            if (item instanceof Sas) {
                // Causes bind loop. Maybe do it another way
                root.sasConfirmed = Qt.binding(() => item.sasConfirmed);
            }
        }
    }
}
