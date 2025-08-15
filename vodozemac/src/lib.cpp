#include "vodozemac.h"

namespace vodozemac::olm {
    Ed25519PublicKey Account::ed25519Key() const { return this->_account->ed25519Key(); }

    Curve25519PublicKey Account::curve25519Key() const { return this->_account->curve25519Key(); }
} // namespace vodozemac::olm

namespace vodozemac::sas {
    bool Sas::isEstablished() const { return this->_sas->isEstablished(); }

    Curve25519PublicKey Sas::publicKey() const { return this->_sas->publicKey(); }

    void Sas::diffieHellman(Curve25519PublicKey theirPublicKey) { this->_sas->diffieHellman(theirPublicKey); }

    SasBytes Sas::bytes(const std::string &info) const { return this->_sas->bytes(info); }
} // namespace vodozemac::sas
