pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ColumnLayout {
    id: root
    spacing: 4

    property bool displayEmojis: true
    property bool displaySas: sas.sasEstablished
    property bool sasConfirmed: sas.sasConfirmed && sas.otherSasConfirmed

    SasVerification {
        id: sas
    }

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (!sas.sasEstablished) {
                return qsTr("Waiting to establish SAS");
            } else if (!sas.sasConfirmed) {
                return qsTr("Verify with the other that the values are equal");
            } else if (!sas.otherSasConfirmed) {
                return qsTr("Waiting on the other the confirm");
            }

            return qsTr("Both confirmed equal sas");
        }
    }

    Loader {
        active: root.displaySas
        Layout.alignment: Qt.AlignHCenter

        sourceComponent: Label {
            Layout.alignment: Qt.AlignHCenter

            font.pointSize: 16
            text: root.displayEmojis ? sas.emojis : sas.decimals
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        visible: root.displaySas
        text: root.displayEmojis ? qsTr("Numbers") : qsTr("Emojis")
        onClicked: root.displayEmojis = !root.displayEmojis
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: root.displaySas

        text: qsTr("Are they equal?")
        color: palette.text
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        visible: root.displaySas
        enabled: !sas.sasConfirmed

        Button {
            text: qsTr("Yes")
            onClicked: sas.confirmSas()
        }

        Button {
            text: qsTr("No")
            onClicked: sas.declineSas()
        }
    }
}
