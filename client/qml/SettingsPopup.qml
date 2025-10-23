pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl as MaterialImpl

Popup {
    id: root

    property var languages: []

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

    onLanguagesChanged: {
        if (root.languages.length <= 0) {
            return;
        }

        languageSelect.model = root.languages.map(v => {
            const l = Qt.locale(v).nativeLanguageName;
            return `${l[0].toUpperCase()}${l.slice(1)}`;
        });
        languageSelect.currentIndex = root.languages.indexOf(settings.locale);
    }

    Settings {
        id: settings

        property string locale: Qt.locale().name

        onLocaleChanged: {
            Qt.uiLanguage = Qt.locale(settings.locale).uiLanguages[0];
        }
    }

    ColumnLayout {
        Label {
            text: "Settings"
            font.pointSize: 12
            font.bold: true

            Layout.bottomMargin: 4
        }

        Label {
            text: qsTr("Language")
        }

        ComboBox {
            id: languageSelect

            implicitContentWidthPolicy: ComboBox.WidestTextWhenCompleted
            onActivated: {
                settings.locale = root.languages[currentIndex];
            }
        }
    }
}
