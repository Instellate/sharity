#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <vodozemac/internals/ffi.h>

namespace vodozemac {
    using Ed25519PublicKey = ffi::Ed25519PublicKey;
    using Curve25519PublicKey = ffi::Curve25519PublicKey;

    inline std::string rustToStdString(rust::String &&string) { return {string.begin(), string.end()}; }
} // namespace vodozemac

namespace vodozemac::olm {
    using OlmMessage = ffi::OlmMessage;

    class Session;

    class Account {
        rust::Box<ffi::OlmAccount> _account = ffi::newOlmAccount();

    public:
        Account() = default;

        Account(const Account &) = delete;
        Account(Account &&) = default;

        Account &operator=(const Account &) = delete;
        Account &operator=(Account &&) = default;

        [[nodiscard]] Ed25519PublicKey ed25519Key() const;
        [[nodiscard]] Curve25519PublicKey curve25519Key() const;
        [[nodiscard]] Curve25519PublicKey generateOneTimeKey();
        [[nodiscard]] Session createOutboundSession(int32_t version,
                                                    const Curve25519PublicKey &identityKey,
                                                    const Curve25519PublicKey &oneTimeKey);
        [[nodiscard]] std::tuple<Session, std::vector<uint8_t>>
        createInboundSession(const Curve25519PublicKey &theirIdentityKey, const OlmMessage &message);

        [[nodiscard]] std::array<uint8_t, 64> sign(const std::vector<uint8_t> &message) const;

        void regenerate();
    };

    class Session {
        rust::Box<ffi::OlmSession> _session;

    public:
        explicit Session(rust::Box<ffi::OlmSession> &&session) : _session(std::move(session)) {}

        Session(const Session &) = delete;
        Session(Session &&) = default;

        Session &operator=(const Session &) = delete;
        Session &operator=(Session &&) = default;

        OlmMessage encrypt(const std::string &plaintext);
        rust::Vec<uint8_t> decrypt(const OlmMessage &message);
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
        void diffieHellman(const Curve25519PublicKey &theirPublicKey);
        SasBytes bytes(const std::string &info) const;
    };
} // namespace vodozemac::sas
