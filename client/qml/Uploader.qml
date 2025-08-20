import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

ColumnLayout {
    id: root

    property bool isUploading: false

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: "Uploader"
    }

    Button {
        Layout.alignment: Qt.AlignHCenter
        enabled: !root.isUploading

        text: "Select file"
        onClicked: fileDialog.open()
    }

    Text {
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
        enabled: !root.isUploading

        text: "Upload file"
        onClicked: {
            root.isUploading = true;
            // peer.startFileNegotiation();
        }
    }

    FileDialog {
        id: fileDialog
        fileMode: FileDialog.OpenFile
    }

    UploaderPeer {
        id: peer
        selectedFile: fileDialog.selectedFile
    }
}
