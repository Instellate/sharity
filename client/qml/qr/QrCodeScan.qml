import QtQuick
import QtQuick.Controls.Material
import QtMultimedia
import com.scythestudio.scodes 1.0

Modal {
    id: root

    width: 250
    height: 250

    signal valueCaptured(message: string)

    Loader {
        active: root.opened

        sourceComponent: SBarcodeScanner {
            forwardVideoSink: videoOutput.videoSink

            onCapturedChanged: function (captured) {
                root.valueCaptured(captured);
                root.close();
            }
        }
    }

    VideoOutput {
        id: videoOutput

        width: 200
        height: 200
        fillMode: VideoOutput.PreserveAspectCrop
        anchors.centerIn: parent
    }
}
