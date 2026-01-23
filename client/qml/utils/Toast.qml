import QtQuick

pragma Singleton

Item {
    signal dispalyMessage(message: string)

    readonly property string test: "Test"

    function display(message: string) {
        dispalyMessage(message)
    }
}

