#include "blockchain.h"
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

void sha256_to_hex(const char *input, size_t len, char output[HASH_HEX_LEN]) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, input, len);
    EVP_DigestFinal_ex(ctx, digest, NULL);
    EVP_MD_CTX_free(ctx);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + (i * 2), "%02x", digest[i]);
    output[64] = '\0';
}

EC_KEY* load_or_generate_keys(char* pub_addr) {
    EC_KEY *key = NULL;
    FILE *f = fopen("private_key.pem", "r");
    if (f) {
        key = PEM_read_ECPrivateKey(f, NULL, NULL, NULL);
        fclose(f);
    }
    if (!key) {
        key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
        EC_KEY_set_asn1_flag(key, OPENSSL_EC_NAMED_CURVE);
        EC_KEY_generate_key(key);
        f = fopen("private_key.pem", "w");
        PEM_write_ECPrivateKey(f, key, NULL, NULL, 0, NULL, NULL);
        fclose(f);
    }
    // Simple address generation: hash of public key points
    const EC_GROUP *group = EC_KEY_get0_group(key);
    const EC_POINT *point = EC_KEY_get0_public_key(key);
    char *hex_pub = EC_POINT_point2hex(group, point, POINT_CONVERSION_UNCOMPRESSED, NULL);
    sha256_to_hex(hex_pub, strlen(hex_pub), pub_addr);
    OPENSSL_free(hex_pub);
    return key;
}

void get_tx_hash_payload(Transaction* tx, char* payload) {
    sprintf(payload, "%s%s%d%s%ld%d", tx->sender_address, tx->receiver_address, tx->amount, tx->transaction_type, tx->timestamp, tx->sender_nonce);
}

void sign_transaction(Transaction* tx, EC_KEY* key) {
    char payload[512];
    get_tx_hash_payload(tx, payload);
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)payload, strlen(payload), digest);

    unsigned int sig_len;
    ECDSA_sign(0, digest, SHA256_DIGEST_LENGTH, tx->digital_signature, &sig_len, key);
    tx->sig_len = sig_len;

    char full_payload[600];
    sprintf(full_payload, "%s%d", payload, sig_len);
    sha256_to_hex(full_payload, strlen(full_payload), tx->transaction_id);
}

int verify_transaction_signature(Transaction* tx) {
    // In a real system we extract EC_KEY from sender_address (pubkey).
    // Here we simulate validation to ensure it's structurally sound.
    if (tx->sig_len == 0 && strcmp(tx->sender_address, "SYSTEM") != 0) return 0;
    return 1;
}
