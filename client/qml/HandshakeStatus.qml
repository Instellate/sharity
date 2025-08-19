import QtQuick

Text {
    text: {
        if (!WebSocket.established) {
            return "Waiting to be etablished";
        } else if (!WebSocket.encrypted) {
            return "Waiting for handshake to finish";
        }

        return "Established and encrypted";
    }
}
