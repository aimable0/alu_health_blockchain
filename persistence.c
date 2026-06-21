#include "blockchain.h"
#include <stdio.h>
#include <string.h>

void save_state() {
    FILE* f = fopen("health_chain.dat", "wb");
    if(!f) return;
    fwrite(&chain.length, sizeof(int), 1, f);
    fwrite(chain.blocks, sizeof(Block), chain.length, f);
    fwrite(&state, sizeof(ChainState), 1, f);
    fclose(f);
    printf("[*] State saved.\n");
}

void load_state() {
    FILE* f = fopen("health_chain.dat", "rb");
    if(!f) {
        state.current_difficulty = 2;
        state.block_reward = 50;
        state.reinsurance_balance = 0;
        return;
    }
    fread(&chain.length, sizeof(int), 1, f);
    fread(chain.blocks, sizeof(Block), chain.length, f);
    fread(&state, sizeof(ChainState), 1, f);
    fclose(f);
    printf("[*] State loaded. Blocks: %d\n", chain.length);
}

void verify_blockchain() {
    printf("\n========== BLOCKCHAIN VERIFICATION ==========\n");
    for(int i=0; i<chain.length; i++) {
        char hash[HASH_HEX_LEN], merkle[HASH_HEX_LEN];
        compute_block_hash(&chain.blocks[i], hash);
        compute_merkle_root(chain.blocks[i].transactions, chain.blocks[i].transaction_count, merkle);

        if(strcmp(hash, chain.blocks[i].hash) != 0) printf("[!] Block %d Hash MISMATCH!\n", i);
        if(strcmp(merkle, chain.blocks[i].merkle_root) != 0) printf("[!] Block %d Merkle MISMATCH!\n", i);
        if(i>0 && strcmp(chain.blocks[i].previous_hash, chain.blocks[i-1].hash) != 0) printf("[!] Block %d Prev Hash BROKEN!\n", i);

        printf("[OK] Block %d Verified.\n", i);
    }
    printf("=============================================\n");
}
