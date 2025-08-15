use vodozemac::{
    Curve25519PublicKey, Ed25519PublicKey,
    olm::Account,
    sas::{EstablishedSas, Sas, SasBytes},
};

use crate::ffi::{CxxCurve25519PublicKey, CxxEd25519PublicKey, CxxSasBytes};

#[cxx::bridge(namespace = "vodozemac::ffi")]
mod ffi {
    #[derive(Clone, Copy)]
    #[cxx_name = "Ed25519PublicKey"]
    struct CxxEd25519PublicKey {
        bytes: [u8; 32],
    }

    #[derive(Clone, Copy)]
    #[cxx_name = "Curve25519PublicKey"]
    struct CxxCurve25519PublicKey {
        bytes: [u8; 32],
    }

    #[cxx_name = "SasBytes"]
    struct CxxSasBytes {
        bytes: [u8; 6],
    }

    extern "Rust" {
        #[cxx_name = "toBase64"]
        fn to_base64(self: &CxxCurve25519PublicKey) -> String;

        #[cxx_name = "fromBase64"]
        #[Self = "CxxCurve25519PublicKey"]
        fn from_base64(content: &str) -> Result<CxxCurve25519PublicKey>;

        #[cxx_name = "toBase64"]
        fn to_base64(self: &CxxEd25519PublicKey) -> Result<String>;

        #[cxx_name = "fromBase64"]
        #[Self = "CxxEd25519PublicKey"]
        fn from_base64_ed(content: &str) -> Result<CxxEd25519PublicKey>;

        type OlmAccount;

        #[cxx_name = "newOlmAccount"]
        fn new_olm_account() -> Box<OlmAccount>;

        #[cxx_name = "ed25519Key"]
        fn ed25519_key(self: &OlmAccount) -> CxxEd25519PublicKey;

        #[cxx_name = "curve25519Key"]
        fn curve25519_key(self: &OlmAccount) -> CxxCurve25519PublicKey;

        #[cxx_name = "Sas"]
        type CxxSas;

        #[cxx_name = "newSas"]
        fn new_sas() -> Box<CxxSas>;

        #[cxx_name = "isEstablished"]
        fn is_established(self: &CxxSas) -> bool;

        #[cxx_name = "publicKey"]
        fn public_key(self: &CxxSas) -> CxxCurve25519PublicKey;

        #[cxx_name = "diffieHellman"]
        fn diffie_hellman(
            self: &mut CxxSas,
            their_public_key: CxxCurve25519PublicKey,
        ) -> Result<()>;

        fn bytes(self: &CxxSas, info: &str) -> Result<CxxSasBytes>;

        #[cxx_name = "emojiIndices"]
        fn emoji_indices(self: &CxxSasBytes) -> [u8; 7];

        fn decimals(self: &CxxSasBytes) -> [u16; 3];
    }
}

impl CxxCurve25519PublicKey {
    fn to_base64(&self) -> String {
        Curve25519PublicKey::from(*self).to_base64()
    }

    fn from_base64(content: &str) -> Result<Self, vodozemac::KeyError> {
        Curve25519PublicKey::from_base64(&content).map(Self::from)
    }
}

impl From<Curve25519PublicKey> for CxxCurve25519PublicKey {
    fn from(value: Curve25519PublicKey) -> Self {
        CxxCurve25519PublicKey {
            bytes: value.to_bytes(),
        }
    }
}

impl From<CxxCurve25519PublicKey> for Curve25519PublicKey {
    fn from(value: CxxCurve25519PublicKey) -> Self {
        Self::from_bytes(value.bytes)
    }
}

impl CxxEd25519PublicKey {
    fn to_base64(&self) -> anyhow::Result<String> {
        Ed25519PublicKey::try_from(*self)
            .map(|v| v.to_base64())
            .map_err(anyhow::Error::from)
    }

    fn from_base64_ed(content: &str) -> Result<Self, vodozemac::KeyError> {
        Ed25519PublicKey::from_base64(content).map(Self::from)
    }
}

impl From<Ed25519PublicKey> for CxxEd25519PublicKey {
    fn from(value: Ed25519PublicKey) -> Self {
        CxxEd25519PublicKey {
            bytes: *value.as_bytes(),
        }
    }
}

impl TryFrom<CxxEd25519PublicKey> for Ed25519PublicKey {
    type Error = vodozemac::KeyError;

    fn try_from(value: CxxEd25519PublicKey) -> Result<Self, vodozemac::KeyError> {
        Self::from_slice(&value.bytes)
    }
}

struct OlmAccount(Account);

fn new_olm_account() -> Box<OlmAccount> {
    Box::new(OlmAccount(Account::new()))
}

impl OlmAccount {
    fn ed25519_key(&self) -> ffi::CxxEd25519PublicKey {
        ffi::CxxEd25519PublicKey {
            bytes: *self.0.ed25519_key().as_bytes(),
        }
    }

    fn curve25519_key(&self) -> ffi::CxxCurve25519PublicKey {
        ffi::CxxCurve25519PublicKey {
            bytes: self.0.curve25519_key().to_bytes(),
        }
    }
}

enum CxxSas {
    NonEstablished(Sas),
    Established(EstablishedSas),
    Establishing, // Exist to allow getting an owned version of `Sas`
}

fn new_sas() -> Box<CxxSas> {
    Box::new(CxxSas::NonEstablished(Sas::new()))
}

impl CxxSas {
    fn is_established(&self) -> bool {
        matches!(self, Self::Established(_))
    }

    fn public_key(&self) -> CxxCurve25519PublicKey {
        match self {
            Self::Established(e) => e.our_public_key(),
            Self::NonEstablished(n) => n.public_key(),
            _ => unreachable!(),
        }
        .into()
    }

    fn diffie_hellman(&mut self, their_public_key: CxxCurve25519PublicKey) -> anyhow::Result<()> {
        let mut owned = CxxSas::Establishing;
        std::mem::swap(self, &mut owned);

        let Self::NonEstablished(sas) = owned else {
            return Err(anyhow::anyhow!("Sas is already established"));
        };

        let key = Curve25519PublicKey::from_slice(&their_public_key.bytes)?;
        let established = sas.diffie_hellman(key)?;
        *self = Self::Established(established);
        Ok(())
    }

    fn bytes(&self, info: &str) -> anyhow::Result<CxxSasBytes> {
        let Self::Established(sas) = self else {
            return Err(anyhow::anyhow!("Sas is not established"));
        };

        Ok(CxxSasBytes {
            bytes: *sas.bytes(info).as_bytes(),
        })
    }
}

impl CxxSasBytes {
    fn emoji_indices(&self) -> [u8; 7] {
        let bytes = unsafe { std::mem::transmute::<&Self, &SasBytes>(self) };
        bytes.emoji_indices()
    }

    fn decimals(&self) -> [u16; 3] {
        let bytes = unsafe { std::mem::transmute::<&Self, &SasBytes>(self) };
        let (one, two, three) = bytes.decimals();
        [one, two, three]
    }
}
