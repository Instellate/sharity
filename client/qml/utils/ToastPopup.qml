import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

Popup {
    id: root

    x: (root.parent.width - root.width) / 2
    y: root.parent.height - root.height - 12

    closePolicy: Popup.NoAutoClose
    background: Rectangle {
        radius: Material.FullScale
        color: Material.dialogColor

        layer.effect: RoundedElevationEffect {
            elevation: Material.elevation
            roundedScale: Material.FullScale
        }
    }

    Label {
        id: label
    }

    Timer {
        id: timer

        interval: 1500
        repeat: false
        onTriggered: root.close()
    }

    function display(msg) {
        label.text = msg;
        root.open();
        timer.start();
    }

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 200
            from: 0.0
            to: 1.0
            easing.type: Easing.InOutQuad
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            duration: 200
            from: 1.0
            to: 0.0
            easing.type: Easing.InOutQuad
        }
    }
}
