import QtQuick
import QtQuick.Layouts

ColumnLayout {
    Layout.fillWidth: true
    spacing: 4

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: SasSingleton.decimals
    }

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: SasSingleton.emojis
    }
}
