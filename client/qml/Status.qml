import QtQuick

Text {
    text: {
        if (!WebSocket.established) {
            return "Waiting to be etablished";
        } else if (!WebSocket.encrypted) {
            return "Waiting for handshake to finish";
        } else if (!WebSocket.sasEstablished) {
            return "Waiting for SAS to be established";
        } else if (!WebSocket.sasConfirmed) {
            return "Verify with the others that the values are eqaul";
        } else if (!WebSocket.otherSasConfirmed) {
            return "Waiting for other client to confirm";
        }

        return "Both confirmed equal SAS";
    }
}
