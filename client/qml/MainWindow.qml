import QtQuick
import QtQuick.Controls.Material

ApplicationWindow {
    id: root
    visible: true
    title: "Sharity"
    width: 500
    height: 500

    Material.theme: Material.System
    Material.accent: MaterialTheme.accent
    Material.background: MaterialTheme.background
    Material.foreground: MaterialTheme.foreground
    Material.primary: MaterialTheme.primary

    property bool sasConfirmed: loader.item?.sasConfirmed
    property alias languages: settings.languages
    property var connectUrl: null

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
    }

    Connections {
        target: loader.item
        enabled: loader.item instanceof Sas
        ignoreUnknownSignals: true

        function onSasConfirmedChanged() {
            root.sasConfirmed = loader.item.sasConfirmed
        }
    }

    RoundButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left

        flat: true
        icon.name: "settings"
        onClicked: settings.open()
    }

    SettingsModal {
        id: settings

        anchors.centerIn: parent
    }

    ToastPopup {
        id: toast
    }

    Connections {
        target: Toast

        function onDispalyMessage(message: string) {
            toast.display(message)
        }
    }

    onConnectUrlChanged: {
        if (root.connectUrl === null) {
            return;
        }
        const url = new URL(decodeURI(root.connectUrl));

        let host;
        if (url.protocol === "usharity:") {
            host = `ws://${url.host}`;
        } else {
            host = `wss://${url.host}`;
        }

        if (url.searchParams.has("path")) {
            host += decodeURIComponent(url.searchParams.get("path"));
        }

        console.log(decodeURIComponent(url.searchParams.get("key")));
        WebSocket.open(host, decodeURIComponent(url.searchParams.get("key")));
        root.connectUrl = null;
    }
}
