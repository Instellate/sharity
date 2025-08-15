#pragma once

#include <string>
#include <vodozemac/internals/ffi.h>

namespace vodozemac {
    using Ed25519PublicKey = ffi::Ed25519PublicKey;
    using Curve25519PublicKey = ffi::Curve25519PublicKey;
} // namespace vodozemac

namespace vodozemac::olm {
    class Account {
        rust::Box<ffi::OlmAccount> _account = ffi::newOlmAccount();

    public:
        Account() {}

        Account(const Account &) = delete;
        Account(Account &&) = default;

        Account &operator=(const Account &) = delete;
        Account &operator=(Account &&) = default;

        Ed25519PublicKey ed25519Key() const;
        Curve25519PublicKey curve25519Key() const;
    };
} // namespace vodozemac::olm

namespace vodozemac::sas {
    using SasBytes = ffi::SasBytes;

    class Sas {
        rust::Box<ffi::Sas> _sas = ffi::newSas();

    public:
        Sas() {}

        Sas(const Sas &) = delete;
        Sas(Sas &&) = delete;

        Sas &operator=(const Sas &) = delete;
        Sas &operator=(Sas &&) = default;

        bool isEstablished() const;
        Curve25519PublicKey publicKey() const;
        void diffieHellman(Curve25519PublicKey theirPublicKey);
        SasBytes bytes(const std::string &info) const;
    };
} // namespace vodozemac::sas
