import QtQuick
import QtQuick.Controls.Material
import QtCore
import QtQuick.Layouts

ColumnLayout {
    id: root

    spacing: 8

    Settings {
        property alias websocketUrl: websocketUrl.text
    }

    property bool isUploader: uploaderRadioButton.checked
    property var toast

    TextField {
        id: websocketUrl
        Layout.preferredWidth: 192
        Layout.alignment: Qt.AlignHCenter

        text: "wss://sharity.is-dominating.me/ws"
        placeholderText: qsTr("WebSocket Relay")
    }

    ColumnLayout {
        Layout.topMargin: 8
        Layout.bottomMargin: 8
        Layout.alignment: Qt.AlignHCenter

        Label {
            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Type")
        }

        RadioButton {
            id: uploaderRadioButton
            text: qsTr("Uploader")
            checked: true
        }

        RadioButton {
            text: qsTr("Downloader")
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
        visible: root.isUploader
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: 2
        Layout.bottomMargin: 2
        // The margins are here to prevent UI from moving
        // when changing between uploader and downloader

        text: qsTr("Copy Connection Key")
        onClicked: {
            connectionKey.selectAll();
            connectionKey.copy();
            connectionKey.deselect();
            root.toast.display(qsTr("Copied to clipboard"));
        }
    }

    TextField {
        id: downloaderKey
        visible: !root.isUploader
        Layout.preferredWidth: 192
        Layout.alignment: Qt.AlignHCenter

        onAccepted: {
            if (WebSocket.isValidKey(text)) {
                WebSocket.open(websocketUrl.text, text);
            } else {
                root.toast.display(qsTr("The key is invalid"));
            }
        }
        placeholderText: qsTr("Connection Key")
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Connect")
        enabled: !WebSocket.connecting
        onClicked: {
            if (root.isUploader) {
                WebSocket.open(websocketUrl.text);
            } else {
                if (WebSocket.isValidKey(downloaderKey.text)) {
                    WebSocket.open(websocketUrl.text, downloaderKey.text);
                } else {
                    root.toast.display(qsTr("The key is invalid"));
                }
            }
        }
    }
}
