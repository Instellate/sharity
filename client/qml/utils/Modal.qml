pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl as MaterialImpl

Popup {
    id: root

    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    background: Rectangle {
        radius: root.Material.roundedScale
        color: palette.window

        layer.enabled: root.Material.elevation > 0
        layer.effect: MaterialImpl.RoundedElevationEffect {
            elevation: root.Material.elevation
            roundedScale: root.Material.roundedScale
        }
    }
}
