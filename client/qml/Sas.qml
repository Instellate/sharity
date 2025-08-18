pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    id: root
    spacing: 4

    property bool displayEmojis: true
    property bool displaySas: sas.sasEstablished

    SasVerification {
        id: sas
    }

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (!sas.sasEstablished) {
                return "Waiting to establish SAS";
            } else if (!sas.sasConfirmed) {
                return "Verify with the other that the values are equal";
            } else if (!sas.otherSasConfirmed) {
                return "Waiting on the other the confirm";
            }

            return "Both confirmed equal sas";
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

            text: root.displayEmojis ? sas.emojis : sas.decimals
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
            onClicked: sas.confirmSas()
        }

        Button {
            text: "No"
            onClicked: sas.declineSas()
        }
    }
}
