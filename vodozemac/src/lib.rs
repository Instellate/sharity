use vodozemac::olm::{Account, OlmMessage, Session, SessionConfig};
use vodozemac::sas::{EstablishedSas, Sas, SasBytes};
use vodozemac::{Curve25519PublicKey, Ed25519PublicKey, Ed25519Signature};

use crate::ffi::{CxxCurve25519PublicKey, CxxEd25519PublicKey, CxxOlmMessage, CxxSasBytes};

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

    #[cxx_name = "OlmMessage"]
    struct CxxOlmMessage {
        bytes: Vec<u8>,
    }

    #[cxx_name = "OlmInboundCreationResult"]
    struct CxxOlmInboundCreationResult {
        session: Box<OlmSession>,
        plaintext: Vec<u8>,
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

        fn verify(self: &CxxEd25519PublicKey, message: &[u8], signature: &[u8]) -> Result<()>;

        type OlmAccount;

        #[cxx_name = "newOlmAccount"]
        fn new_olm_account() -> Box<OlmAccount>;

        #[cxx_name = "ed25519Key"]
        fn ed25519_key(self: &OlmAccount) -> CxxEd25519PublicKey;

        #[cxx_name = "curve25519Key"]
        fn curve25519_key(self: &OlmAccount) -> CxxCurve25519PublicKey;

        #[cxx_name = "generateOneTimeKey"]
        fn generate_one_time_key(self: &mut OlmAccount) -> CxxCurve25519PublicKey;

        fn sign(self: &OlmAccount, message: &[u8]) -> [u8; 64];

        fn regenerate(self: &mut OlmAccount);

        #[cxx_name = "createOutboundSession"]
        fn create_outbound_session(
            self: &OlmAccount,
            version: i32,
            ik: CxxCurve25519PublicKey,
            otk: CxxCurve25519PublicKey,
        ) -> Box<OlmSession>;

        #[cxx_name = "createInboundSession"]
        fn create_inbound_session(
            self: &mut OlmAccount,
            their_identity_key: CxxCurve25519PublicKey,
            message: &CxxOlmMessage,
        ) -> Result<CxxOlmInboundCreationResult>;

        type OlmSession;

        fn encrypt(self: &mut OlmSession, plaintext: &str) -> CxxOlmMessage;

        fn decrypt(self: &mut OlmSession, message: CxxOlmMessage) -> Result<Vec<u8>>;

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

    fn verify(&self, message: &[u8], signature: &[u8]) -> anyhow::Result<()> {
        let key: Ed25519PublicKey = self.clone().try_into()?;
        let sig = Ed25519Signature::from_slice(signature)?;

        key.verify(message, &sig).map_err(anyhow::Error::from)
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
        self.0.ed25519_key().into()
    }

    fn curve25519_key(&self) -> ffi::CxxCurve25519PublicKey {
        self.0.curve25519_key().into()
    }

    fn generate_one_time_key(&mut self) -> ffi::CxxCurve25519PublicKey {
        let keys = self.0.generate_one_time_keys(1);
        let key = keys.created.first().unwrap().clone();
        key.into()
    }

    fn sign(&self, message: &[u8]) -> [u8; 64] {
        self.0.sign(message).to_bytes()
    }

    fn create_outbound_session(
        &self,
        version: i32,
        ik: CxxCurve25519PublicKey,
        otk: CxxCurve25519PublicKey,
    ) -> Box<OlmSession> {
        let config = match version {
            1 => SessionConfig::version_1(),
            2 => SessionConfig::version_2(),
            _ => unreachable!(),
        };

        let session = self
            .0
            .create_outbound_session(config, ik.into(), otk.into());
        Box::new(OlmSession(session))
    }

    fn create_inbound_session(
        self: &mut OlmAccount,
        tik: CxxCurve25519PublicKey,
        mesage: &CxxOlmMessage,
    ) -> anyhow::Result<ffi::CxxOlmInboundCreationResult> {
        let message = mesage.try_as_olm_message()?;
        let OlmMessage::PreKey(pre_message) = message else {
            return Err(anyhow::anyhow!("Message is not pre key"));
        };

        let result = self.0.create_inbound_session(tik.into(), &pre_message)?;

        Ok(ffi::CxxOlmInboundCreationResult {
            session: Box::new(OlmSession(result.session)),
            plaintext: result.plaintext,
        })
    }

    fn regenerate(&mut self) {
        let mut account = Account::new();
        std::mem::swap(&mut self.0, &mut account);
    }
}

struct OlmSession(Session);

impl OlmSession {
    fn encrypt(&mut self, plaintext: &str) -> ffi::CxxOlmMessage {
        self.0.encrypt(plaintext).into()
    }

    fn decrypt(&mut self, message: CxxOlmMessage) -> anyhow::Result<Vec<u8>> {
        let message = message.try_into()?;
        self.0.decrypt(&message).map_err(anyhow::Error::from)
    }
}

impl ffi::CxxOlmMessage {
    fn try_as_olm_message(&self) -> anyhow::Result<OlmMessage> {
        let bytes: &[u8] = &self.bytes;
        if bytes.len() < 4 {
            return Err(anyhow::anyhow!("Olm message is too small"));
        }
        let msg_type_bytes: &[u8; 4] = bytes[0..4].try_into().map_err(anyhow::Error::from)?;
        let msg_type = u32::from_be_bytes(*msg_type_bytes);

        OlmMessage::from_parts(msg_type as usize, &bytes[4..]).map_err(anyhow::Error::from)
    }
}

impl From<OlmMessage> for ffi::CxxOlmMessage {
    fn from(value: OlmMessage) -> Self {
        let (msg_type, mut msg) = value.to_parts();
        let msg_type_bytes = (msg_type as u32).to_be_bytes();
        msg.splice(0..0, msg_type_bytes);

        ffi::CxxOlmMessage { bytes: msg }
    }
}

impl TryFrom<CxxOlmMessage> for OlmMessage {
    type Error = anyhow::Error;

    fn try_from(value: CxxOlmMessage) -> Result<Self, Self::Error> {
        value.try_as_olm_message()
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
