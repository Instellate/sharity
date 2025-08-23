import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ColumnLayout {
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
}
