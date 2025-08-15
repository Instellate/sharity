import QtQuick.Controls

TextField {
    onAccepted: SasSingleton.diffieHellman(text)
    placeholderText: 'Other parties key...'
}
