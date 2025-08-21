import QtQml
import QtQuick
import QtQuick.Layouts

ColumnLayout {
    DownloaderPeer {
        id: peer
    }

    Text {
        Layout.alignment: Qt.AlignHCenter

        text: "Downloader"
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        text: peer.downloadState
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        text: `${Qt.locale().formattedDataSize(peer.amountDownloaded)} out of ${Qt.locale().formattedDataSize(peer.fileSize)}`
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        visible: peer.downloadState !== "Waiting"
        text: `${Qt.locale().formattedDataSize(peer.speed)}/s`
    }
}
