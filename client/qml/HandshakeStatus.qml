import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ColumnLayout {
    id: root
    property var toast

    Settings {
        id: settings
        property string websocketUrl
    }

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (!WebSocket.established) {
                return qsTr("Waiting to be etablished");
            } else if (!WebSocket.encrypted) {
                return qsTr("Waiting for handshake to finish");
            }

            return qsTr("Established and encrypted");
        }
    }

    TextEdit {
        id: connectionKey
        visible: false
        Layout.alignment: Qt.AlignHCenter

        text: WebSocket.publicKey
        color: palette.text
        readOnly: true
        selectByMouse: true
    }

    Button {
        visible: !WebSocket.isDownloader
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Copy Connection Key")
        onClicked: {
            connectionKey.selectAll();
            connectionKey.copy();
            connectionKey.deselect();
            root.toast.display(qsTr("Copied to clipboard"));
        }
    }

    Button {
        visible: !WebSocket.isDownloader
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Copy Download URL")
        onClicked: {
            const key = connectionKey.text;
            const wsUrl = new URL(settings.websocketUrl);
            
            const searchParams = new URLSearchParams();
            searchParams.set("path", wsUrl.pathname);
            searchParams.set("key", key);

            const protocol = wsUrl.protocol === "wss:" ? "sharity" : "usharity";
            const sharityUrl = `${protocol}://${wsUrl.host}/?${searchParams.toString()}`;

            connectionKey.text = sharityUrl;
            connectionKey.selectAll();
            connectionKey.copy();
            connectionKey.deselect();

            connectionKey.text = key;
            root.toast.display(qsTr("Copied to clipboard"));
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Cancel")
        enabled: !WebSocket.established
        onClicked: WebSocket.close()
    }
}
