import QtQuick
import QtQuick.Controls.Material

ApplicationWindow {
    id: root
    visible: true
    title: "Sharity"
    width: 500
    height: 500

    Material.theme: Material.System
    Material.accent: Material.Indigo

    property bool sasConfirmed: false
    property alias languages: settings.languages

    Connections {
        target: WebSocket
        function onConnectedChanged() {
            root.sasConfirmed = false;
        }
    }

    Loader {
        id: loader
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

    Binding {
        target: loader.item
        property: "toast"
        value: toast
        when: loader.status == Loader.Ready && loader.item instanceof Connect
    }

    RoundButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        flat: true
        icon.name: "settings"
        onClicked: settings.open()
    }

    SettingsPopup {
        id: settings

        anchors.centerIn: parent
    }

    Toast {
        id: toast
    }
}
