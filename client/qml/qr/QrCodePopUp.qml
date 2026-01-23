pragma ComponentBehavior: Bound

import QtQuick
import com.scythestudio.scodes 1.0

Modal {
    id: root

    required property string qrValue

    width: 250
    height: 250

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
