import QtQuick.Controls
import QtQuick
import QtQuick.Layouts

ColumnLayout {
    width: parent.width
    anchors.centerIn: parent

    spacing: 8

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

        onAccepted: WebSocket.open("ws://localhost:4001/ws", text)
        placeholderText: 'Other parties key...'
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: "Connect as uploader"
        onClicked: WebSocket.open("ws://localhost:4001/ws")
    }
}
