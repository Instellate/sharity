pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

Modal {
    id: root

    property var languages: []

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
            console.log(Qt.uiLanguage);
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
