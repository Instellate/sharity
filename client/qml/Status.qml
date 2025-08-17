pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    id: root
    spacing: 4

    property bool displayEmojis: true
    property bool displaySas: WebSocket.sasEstablished && !WebSocket.sasConfirmed

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (!WebSocket.established) {
                return "Waiting to be etablished";
            } else if (!WebSocket.encrypted) {
                return "Waiting for handshake to finish";
            } else if (!WebSocket.sasEstablished) {
                return "Waiting for SAS to be established";
            } else if (!WebSocket.sasConfirmed) {
                return "Verify with the others that the values are eqaul";
            }

            return "Waiting...";
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        visible: root.displaySas
        text: root.displayEmojis ? "Numbers" : "Emojis"
        onClicked: root.displayEmojis = !root.displayEmojis
    }

    Loader {
        active: root.displaySas
        Layout.alignment: Qt.AlignHCenter

        sourceComponent: Text {
            Layout.alignment: Qt.AlignHCenter

            visible: WebSocket.sasEstablished
            text: root.displayEmojis ? WebSocket.sasEmojis : WebSocket.sasDecimals
        }
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        visible: root.displaySas

        text: "Are they equal?"
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        visible: root.displaySas

        Button {
            text: "Yes"
            onClicked: WebSocket.confirmSas()
        }

        Button {
            text: "No"
            onClicked: WebSocket.declineSas()
        }
    }
}
