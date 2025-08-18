import QtQuick

Window {
    title: "Sharity"



    Loader {
        width: parent.width
        anchors.centerIn: parent

        // source: WebSocket.connected ? "Status.qml" : "GetOtherKey.qml"
        source: {
            if (!WebSocket.connected) {
                return "GetOtherKey.qml"
            } else if (!WebSocket.encrypted) {
                return "Status.qml"
            } else {
                return "Sas.qml";
            } 
        }
    }
}
