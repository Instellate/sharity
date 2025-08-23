import QtQuick
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Controls.Material

ColumnLayout {
    id: root

    property bool isUploading: false

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: "Uploader"
        color: palette.text
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        visible: !root.isUploading

        text: "Select file"
        onClicked: fileDialog.open()
    }

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: {
            if (fileDialog.selectedFiles.length <= 0) {
                return "No file selected";
            }

            const path = fileDialog.selectedFile.toString().replace(/^(file:\/{2})/, "");
            const cleanPath = decodeURIComponent(path);
            return `Selected file: ${cleanPath}`;
        }
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        visible: !root.isUploading
        enabled: fileDialog.selectedFiles.length > 0

        text: "Upload file"
        onClicked: {
            root.isUploading = true;
            peer.startFileNegotiation();
        }
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: root.isUploading
        text: `${Qt.locale().formattedDataSize(peer.amountUploaded)} out of ${Qt.locale().formattedDataSize(peer.fileSize)}`
        color: palette.text
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
        text: "Cancel"
        enabled: !root.isUploading

        onClicked: WebSocket.close()
    }

    UploaderPeer {
        id: peer
        selectedFile: fileDialog.selectedFile

        onFileUploaded: root.isUploading = false
    }
}
