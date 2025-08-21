import QtQuick
import QtCore
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    width: parent.width
    anchors.centerIn: parent

    spacing: 8

    Settings {
        property alias websocketUrl: websocketUrl.text
    }

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: "WebSocket Relay:"
    }

    TextField {
        id: websocketUrl
        Layout.alignment: Qt.AlignHCenter

        text: "wss://sharity.is-dominating.me/ws"
    }

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: "Your connection key:"
    }

    TextEdit {
        Layout.alignment: Qt.AlignHCenter

        text: WebSocket.publicKey
        readOnly: true
        selectByMouse: true
    }

    TextField {
        Layout.alignment: Qt.AlignHCenter

        onAccepted: WebSocket.open(websocketUrl.text, text)
        placeholderText: 'Other parties key...'
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: "Connect as uploader"
        onClicked: WebSocket.open(websocketUrl.text)
    }
}
