import QtQuick

pragma Singleton

Item {
    signal displayMessage(message: string)

    readonly property string test: "Test"

    function display(message: string) {
        displayMessage(message)
    }
}

