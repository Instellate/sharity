pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    id: root
    spacing: 4

    property bool displayEmojis: true

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (!WebSocket.established) {
                return "Waiting to be etablished";
            } else if (!WebSocket.encrypted) {
                return "Waiting for handshake to finish";
            } else if (!WebSocket.sasEstablished) {
                return "Waiting for SAS to be established";
            }

            return "Verify with the others that the values are eqaul";
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        visible: WebSocket.sasEstablished
        text: root.displayEmojis ? "Numbers" : "Emojis"
        onClicked: root.displayEmojis = !root.displayEmojis
    }

    Loader {
        active: WebSocket.sasEstablished
        Layout.alignment: Qt.AlignHCenter

        sourceComponent: Text {
            Layout.alignment: Qt.AlignHCenter

            visible: WebSocket.sasEstablished
            text: root.displayEmojis ? WebSocket.sasEmojis : WebSocket.sasDecimals
        }
    }
}
