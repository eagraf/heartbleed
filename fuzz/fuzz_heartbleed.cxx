#include <openssl/ssl.h>
#include <openssl/err.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

#include "fuzzbuzz.hxx"

SSL_CTX *Init() {
    SSL_library_init();
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    SSL_CTX *sctx;
    assert (sctx = SSL_CTX_new(TLSv1_method()));
    /* These two file were created with this command:
    openssl req -x509 -newkey rsa:512 -keyout server.key     -out server.pem -days 9999 -nodes -subj /CN=a/
    */
    assert(SSL_CTX_use_certificate_file(sctx, "runtime/server.pem", SSL_FILETYPE_PEM));
    assert(SSL_CTX_use_PrivateKey_file(sctx, "runtime/server.key", SSL_FILETYPE_PEM));
    return sctx;
}

FZBZ_TARGET_CLASS(FuzzHandshake);
struct FuzzHandshake {
    FuzzHandshake(fzbz::Fuzzer &f) {
        auto gen = fzbz::generator::byteArray<std::vector<uint8_t>>();

        auto target = fzbz::fuzzTarget([&](auto Data) {
              static SSL_CTX *sctx = Init();
              SSL *server = SSL_new(sctx);
              BIO *sinbio = BIO_new(BIO_s_mem());
              BIO *soutbio = BIO_new(BIO_s_mem());
              SSL_set_bio(server, sinbio, soutbio);
              SSL_set_accept_state(server);
              BIO_write(sinbio, Data.data(), Data.size());
              SSL_do_handshake(server);
              SSL_free(server);
              return 0;
        }, gen);

        f.addFuzzTarget(target);
    }
};
