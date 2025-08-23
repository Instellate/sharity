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
        text: peer.downloadState
    }

    Label {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        text: `${Qt.locale().formattedDataSize(peer.amountDownloaded)} out of ${Qt.locale().formattedDataSize(peer.fileSize)}`
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

        text: "Cancel"
        onClicked: {
            peer.close();
            WebSocket.close();
        }
    }
}
