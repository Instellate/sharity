pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl as MaterialImpl
import QtMultimedia
import com.scythestudio.scodes 1.0

Popup {
    id: root

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    modal: true
    width: 250
    height: 250

    signal valueCaptured(message: string)

    background: Rectangle {
        color: palette.window
        layer.enabled: root.Material.elevation > 0
        radius: root.Material.roundedScale

        layer.effect: MaterialImpl.RoundedElevationEffect {
            elevation: root.Material.elevation
            roundedScale: root.Material.roundedScale
        }
    }

    Loader {
        active: root.opened

        sourceComponent: SBarcodeScanner {
            forwardVideoSink: videoOutput.videoSink

            onCapturedChanged: function (captured) {
                root.valueCaptured(captured)
                root.close()
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
