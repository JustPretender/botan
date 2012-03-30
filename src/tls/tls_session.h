/*
* TLS Session
* (C) 2011-2012 Jack Lloyd
*
* Released under the terms of the Botan license
*/

#ifndef TLS_SESSION_STATE_H__
#define TLS_SESSION_STATE_H__

#include <botan/x509cert.h>
#include <botan/tls_version.h>
#include <botan/tls_ciphersuite.h>
#include <botan/tls_magic.h>
#include <botan/secmem.h>
#include <botan/symkey.h>
#include <chrono>

namespace Botan {

namespace TLS {

/**
* Class representing a TLS session state
*/
class BOTAN_DLL Session
   {
   public:

      /**
      * Uninitialized session
      */
      Session() :
         m_start_time(std::chrono::system_clock::time_point::min()),
         m_version(),
         m_ciphersuite(0),
         m_compression_method(0),
         m_connection_side(static_cast<Connection_Side>(0)),
         m_secure_renegotiation_supported(false),
         m_fragment_size(0)
            {}

      /**
      * New session (sets session start time)
      */
      Session(const MemoryRegion<byte>& session_id,
              const MemoryRegion<byte>& master_secret,
              Protocol_Version version,
              u16bit ciphersuite,
              byte compression_method,
              Connection_Side side,
              bool secure_renegotiation_supported,
              size_t fragment_size,
              const std::vector<X509_Certificate>& peer_certs,
              const MemoryRegion<byte>& session_ticket,
              const std::string& sni_hostname = "",
              const std::string& srp_identifier = "");

      /**
      * Load a session from DER representation (created by DER_encode)
      */
      Session(const byte ber[], size_t ber_len);

      /**
      * Load a session from PEM representation (created by PEM_encode)
      */
      Session(const std::string& pem);

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      SecureVector<byte> DER_encode() const;

      /**
      * Encrypt a session (useful for serialization or session tickets)
      */
      MemoryVector<byte> encrypt(const SymmetricKey& key,
                                 RandomNumberGenerator& rng) const;


      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param ctext_size the size of ctext in bytes
      * @param key the same key used by the encrypting side
      */
      static Session decrypt(const byte ctext[],
                             size_t ctext_size,
                             const SymmetricKey& key);

      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param key the same key used by the encrypting side
      */
      static inline Session decrypt(const MemoryRegion<byte>& ctext,
                                    const SymmetricKey& key)
         {
         return Session::decrypt(&ctext[0], ctext.size(), key);
         }

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      std::string PEM_encode() const;

      /**
      * Get the version of the saved session
      */
      Protocol_Version version() const { return m_version; }

      /**
      * Get the ciphersuite code of the saved session
      */
      u16bit ciphersuite_code() const { return m_ciphersuite; }

      /**
      * Get the ciphersuite info of the saved session
      */
      Ciphersuite ciphersuite() const { return Ciphersuite::by_id(m_ciphersuite); }

      /**
      * Get the compression method used in the saved session
      */
      byte compression_method() const { return m_compression_method; }

      /**
      * Get which side of the connection the resumed session we are/were
      * acting as.
      */
      Connection_Side side() const { return m_connection_side; }

      /**
      * Get the SNI hostname (if sent by the client in the initial handshake)
      */
      std::string sni_hostname() const { return m_sni_hostname; }

      /**
      * Get the SRP identity (if sent by the client in the initial handshake)
      */
      std::string srp_identifier() const { return m_srp_identifier; }

      /**
      * Get the saved master secret
      */
      const SecureVector<byte>& master_secret() const
         { return m_master_secret; }

      /**
      * Get the session identifier
      */
      const MemoryVector<byte>& session_id() const
         { return m_identifier; }

      /**
      * Get the negotiated maximum fragment size (or 0 if default)
      */
      size_t fragment_size() const { return m_fragment_size; }

      /**
      * Is secure renegotiation supported?
      */
      bool secure_renegotiation() const
         { return m_secure_renegotiation_supported; }

      /**
      * Return the certificate chain of the peer (possibly empty)
      */
      std::vector<X509_Certificate> peer_certs() const { return m_peer_certs; }

      /**
      * Get the time this session began (seconds since Epoch)
      */
      std::chrono::system_clock::time_point start_time() const
         { return m_start_time; }

      /**
      * Return the session ticket the server gave us
      */
      const MemoryVector<byte>& session_ticket() const { return m_session_ticket; }

   private:
      enum { TLS_SESSION_PARAM_STRUCT_VERSION = 0x2994e300 };

      std::chrono::system_clock::time_point m_start_time;

      MemoryVector<byte> m_identifier;
      MemoryVector<byte> m_session_ticket; // only used by client side
      SecureVector<byte> m_master_secret;

      Protocol_Version m_version;
      u16bit m_ciphersuite;
      byte m_compression_method;
      Connection_Side m_connection_side;

      bool m_secure_renegotiation_supported;
      size_t m_fragment_size;

      std::vector<X509_Certificate> m_peer_certs;
      std::string m_sni_hostname; // optional
      std::string m_srp_identifier; // optional
   };

}

}

#endif
