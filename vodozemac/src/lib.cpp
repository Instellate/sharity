#include "vodozemac.h"

namespace vodozemac::olm {
    Ed25519PublicKey Account::ed25519Key() const { return this->_account->ed25519Key(); }

    Curve25519PublicKey Account::curve25519Key() const { return this->_account->curve25519Key(); }

    Curve25519PublicKey Account::generateOneTimeKey() { return this->_account->generateOneTimeKey(); }

    Session Account::createOutboundSession(const int32_t version,
                                           const Curve25519PublicKey &identityKey,
                                           const Curve25519PublicKey &oneTimeKey) {
        rust::Box<ffi::OlmSession> session = this->_account->createOutboundSession(version, identityKey, oneTimeKey);
        return Session{std::move(session)};
    }

    std::tuple<Session, std::vector<uint8_t>> Account::createInboundSession(const Curve25519PublicKey &theirIdentityKey,
                                                                            const OlmMessage &message) {
        ffi::OlmInboundCreationResult result = this->_account->createInboundSession(theirIdentityKey, message);
        Session session{std::move(result.session)};
        std::vector<uint8_t> plaintext;
        plaintext.insert(plaintext.begin(), result.plaintext.begin(), result.plaintext.end());
        return {std::move(session), plaintext};
    }

    std::array<uint8_t, 64> Account::sign(const std::vector<uint8_t> &message) const {
        const rust::Slice slice{message.data(), message.size()};
        return this->_account->sign(slice);
    }

    OlmMessage Session::encrypt(const std::string &plaintext) { return this->_session->encrypt(plaintext); }

    rust::Vec<uint8_t> Session::decrypt(const OlmMessage &message) { return this->_session->decrypt(message); }
} // namespace vodozemac::olm

namespace vodozemac::sas {
    bool Sas::isEstablished() const { return this->_sas->isEstablished(); }

    Curve25519PublicKey Sas::publicKey() const { return this->_sas->publicKey(); }

    void Sas::diffieHellman(const Curve25519PublicKey &theirPublicKey) { this->_sas->diffieHellman(theirPublicKey); }

    SasBytes Sas::bytes(const std::string &info) const { return this->_sas->bytes(info); }
} // namespace vodozemac::sas
