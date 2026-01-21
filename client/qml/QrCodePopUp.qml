pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl as MaterialImpl
import com.scythestudio.scodes 1.0

Popup {
    id: root

    required property string qrValue

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    modal: true
    width: 250
    height: 250

    background: Rectangle {
        color: palette.window
        layer.enabled: root.Material.elevation > 0
        radius: root.Material.roundedScale

        layer.effect: MaterialImpl.RoundedElevationEffect {
            elevation: root.Material.elevation
            roundedScale: root.Material.roundedScale
        }
    }

    onQrValueChanged: {
        barcodeGenerator.generate(root.qrValue);
    }

    SBarcodeGenerator {
        id: barcodeGenerator
        format: "QRCode"

        Component.onCompleted: {
            barcodeGenerator.generate(root.qrValue);
        }

        onGenerationFinished: function (error) {
            if (error === "") {
                image.source = "file:///" + barcodeGenerator.filePath;
            } else {
                console.log(error);
            }
        }
    }

    Image {
        id: image
        width: 200
        height: 200

        anchors.centerIn: parent
    }
}
