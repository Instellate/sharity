import QtQuick

Window {
    title: "Sharity"

    Loader {
        width: parent.width
        anchors.centerIn: parent

        source: WebSocket.connected ? "Status.qml" : "GetOtherKey.qml"
    }
}
