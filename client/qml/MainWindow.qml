import QtQuick
import QtQuick.Layouts

Window {
    title: 'Sharity'

    ColumnLayout {
        width: parent.width
        anchors.centerIn: parent

        spacing: 8

        TextEdit {
            Component.onCompleted: {
                parent.width = width;
                parent.height = height;
            }

            Layout.alignment: Qt.AlignHCenter

            text: SasSingleton.publicKey
            readOnly: true
            selectByMouse: true
        }

        Loader {
            Layout.alignment: Qt.AlignHCenter

            source: SasSingleton.isEstablished ? "DisplayDecimal.qml" : "GetOtherKey.qml"
        }
    }
}
