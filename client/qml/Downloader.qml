import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ColumnLayout {
    DownloaderPeer {
        id: peer
    }

    Label {
        Layout.alignment: Qt.AlignHCenter

        text: "Downloader"
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        text: {
            switch (peer.downloadState) {
            case "Waiting":
                return qsTr("Waiting");
            case "Downloading":
                return qsTr("Downloading");
            case "Downloaded":
                return qsTr("Downloaded");
            }
        }
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        //: How much of the file has been downloaded out of the total amount
        text: qsTr("%1  out of %2").arg(Qt.locale().formattedDataSize(peer.amountDownloaded), Qt.locale().formattedDataSize(peer.fileSize))
        color: palette.text
    }

    ProgressBar {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        value: peer.amountDownloaded / peer.fileSize
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        text: `${Qt.locale().formattedDataSize(peer.speed)}/s`
    }

    Button {
        Layout.alignment: Qt.AlignHCenter

        text: qsTr("Cancel")
        onClicked: {
            peer.close();
            WebSocket.close();
        }
    }
}
