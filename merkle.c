#include "blockchain.h"
#include <string.h>
#include <stdio.h>

void compute_merkle_root(Transaction txs[], int count, char root_out[HASH_HEX_LEN]) {
    if (count == 0) {
        strcpy(root_out, "0000000000000000000000000000000000000000000000000000000000000000");
        return;
    }

    char hashes[MAX_TRANSACTIONS][HASH_HEX_LEN];
    for (int i = 0; i < count; i++) {
        char payload[512];
        sprintf(payload, "%s%s%d%s%ld%d", txs[i].sender_address, txs[i].receiver_address, txs[i].amount, txs[i].transaction_type, txs[i].timestamp, txs[i].sender_nonce);
        sha256_to_hex(payload, strlen(payload), hashes[i]);
    }

    int curr_count = count;
    while (curr_count > 1) {
        int next_count = (curr_count + 1) / 2;
        char next_hashes[MAX_TRANSACTIONS][HASH_HEX_LEN];

        for (int i = 0; i < next_count; i++) {
            int left = i * 2;
            int right = (i * 2 + 1 < curr_count) ? i * 2 + 1 : left; // Duplicate if odd

            char combined[135];
            sprintf(combined, "%s%s", hashes[left], hashes[right]);
            sha256_to_hex(combined, strlen(combined), next_hashes[i]);
        }
        memcpy(hashes, next_hashes, next_count * HASH_HEX_LEN);
        curr_count = next_count;
    }
    strcpy(root_out, hashes[0]);
}
