# Sharity Protocol
The sharity platform has its own "protocol" for file sharing.  
This protocol is based on websockets and WebRTC. 
This allows file sharing to happen through a browser as well.

## Roles
There are three roles for this protocol works.
- Uploader. The peer that uploads a file
- Downloader. The peer that receives the file from the Uploader.
- WebSocket Relay. A websocket relay used for control messages. The implementation of this is defined at [WebSocket Establishment](#websocket-establishment)

## WebSocket Establishment
For the websocket establishment to work. The Uploader needs to generate a Ed25519 key.
This key is shared out of bands to the Downloader.

The websocket url is defined by the implementor. 
But to establish a connection to the websocket the following queries are needed:
- `type`: Either `downloader` or `uploader`
- `key`: The public key
- `message`: Only needed for the Uploader. Used to confirm the ownage of the key. Should preferably be cryptographically random bytes.
- `signature`: Only needed for the Uploader. The signature for `message`.

For the uploader. The following HTTP status codes may be returned by the WebSocket Relay.
- `400 Bad Request`: The key is invalid or the signature check failed
- `409 Conflict`: The key is already occupied

For the downloader. The following HTTP status codes may be returned by the WebSocket Relay.
- `400 Bad Request`: The key is invalid
- `404`: There is no uploader for the provided key or the key already has a downloader established

The Uploader should connect to the WebSocket Relay first.
When the Uploader has connected the Downloader can proceed to connect.
When both parties have connected the following JSON message will be sent to both parties:
```json
{
    "type": "connected",
    "stun_servers": [...]
}
```
When this message has been sent it indicates that the WebSocket Relay will start relaying messages between the two peers.

## Peer Handshake
The peer handshake is done to switch the websocket communication to E2EE.
This prevents a corrupt WebSocket Relay from eevesdropping or MiTM attacks.  
The whole handshake is done in binary.

The Downloader and Uploader creates an ephemeral Olm Account.  
The Downloader sends their identity key and a one time key.  
The Uploader creates a new outbounds Olm Sesssion based on these parameters.
The Uploader concatenates the Downloader's identity key, one time key, their own identity key,
a pre message containing cryptographically strong random bytes and signs this using the private key 
linked to the public key used in the `key` query parameter. The Uploader then sends this back to the Downloader.
The Downloader verifies that the signature is valid against the public key, and that the echoed identity key is equal to their own.
If the signature is invalid or the identity key echoed back is not equal to the own, the Downloader will terminate the connection.

If everything is valid the Downloader creates a inbounds Olm Session based on the given parameters from the Uploader.  
After this both peers switch to encrypting messages using Olm. All messages after this should be encrypted JSON.

This only confirms the Uploader's identity to the Downloader. To confirm the Downloader's to the Uploader SAS is used.
When the Downloader has created their session they will create a SAS private public key pair. 
When this is created the uploader will send the following JSON message
```json
{
    "type": "sas_token",
    "sas_token": "base64 encooded curve25519 public key without padding",
    "message": "cryptographically generated message"
}
```
When the Uploader receives this, they should themselves create a public private SAS key pair.  
The Uploader then sends a JSON message with the same properties as the Downloader's JSON message 
with their own public curve25519 key and without a message field.

The equality of these tokens should then be confirmed by the Uploader and the Downloader out of bands.
If they match both peers will send the following JSON message.
```json
{
    "type": "sas_confirmed"
}
```

If any of these step fails. The connection should be terminated.  
If all these steps succeed. The handshake have been completed.

## Peer communication

