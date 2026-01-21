import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material

ColumnLayout {
    id: root

    property bool isUploading: false

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Uploader")
        color: palette.text
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        visible: !root.isUploading

        text: qsTr("Select file")
        onClicked: fileDialog.open()
    }

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (fileDialog.selectedFiles.length <= 0) {
                return qsTr("No file selected");
            }

            return qsTr("Selected file: %1").arg(peer.fileName);
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        visible: !root.isUploading
        enabled: fileDialog.selectedFiles.length > 0

        text: qsTr("Upload file")
        onClicked: {
            root.isUploading = true;
            peer.startFileNegotiation();
        }
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: root.isUploading
        //: How much of the file has been downloaded
        text: qsTr("%1  out of %2").arg(Qt.locale().formattedDataSize(peer.amountUploaded)).arg(Qt.locale().formattedDataSize(peer.fileSize))
        color: palette.text
    }

    ProgressBar {
        Layout.alignment: Qt.AlignHCenter
        visible: root.isUploading
        value: peer.amountUploaded / peer.fileSize
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: root.isUploading
        text: `${Qt.locale().formattedDataSize(peer.speed)}/s`
    }

    FileDialog {
        id: fileDialog
        fileMode: FileDialog.OpenFile
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Cancel")
        onClicked: {
            peer.close();
            WebSocket.close();
        }
    }

    UploaderPeer {
        id: peer
        selectedFile: fileDialog.selectedFile

        onFileUploaded: root.isUploading = false
    }
}
