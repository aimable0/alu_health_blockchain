#include "blockchain.h"
#include <stdio.h>
#include <string.h>

void compute_block_hash(Block* b, char* out) {
    char payload[2048];
    sprintf(payload, "%d%ld%d%s%s%d%s%d", b->block_id, b->timestamp, b->transaction_count, b->previous_hash, b->merkle_root, b->nonce, b->miner_id, b->difficulty);
    sha256_to_hex(payload, strlen(payload), out);
}

void retarget_difficulty() {
    if (chain.length > 0 && chain.length % 10 == 0) {
        long total_time = chain.blocks[chain.length-1].timestamp - chain.blocks[chain.length-10].timestamp;
        long avg = total_time / 10;
        int old = state.current_difficulty;
        if (avg < 30) state.current_difficulty++;
        else if (avg > 90 && state.current_difficulty > 1) state.current_difficulty--;
        state.last_retarget_block = chain.length;
        printf("[*] Retargeting: Avg Time %ld s | Diff %d -> %d\n", avg, old, state.current_difficulty);
    }
}

void mine_block(const char* miner) {
    Block b = {0};
    b.block_id = chain.length;
    b.timestamp = time(NULL);
    b.difficulty = state.current_difficulty;
    strcpy(b.miner_id, miner);
    if(chain.length == 0) strcpy(b.previous_hash, "0000000000000000000000000000000000000000000000000000000000000000");
    else strcpy(b.previous_hash, chain.blocks[chain.length-1].hash);

    // Pick top Txs
    MempoolNode* curr = mempool_head;
    while(curr && b.transaction_count < MAX_TRANSACTIONS) {
        if(strcmp(curr->tx.status, TX_PENDING) == 0) {
            b.transactions[b.transaction_count++] = curr->tx;
        }
        curr = curr->next;
    }

    if(b.transaction_count == 0) {
        printf("[!] Mempool empty. Nothing to mine.\n");
        return;
    }

    compute_merkle_root(b.transactions, b.transaction_count, b.merkle_root);

    char target[20];
    memset(target, '0', state.current_difficulty);
    target[state.current_difficulty] = '\0';

    int attempts = 0;
    do {
        b.nonce++;
        attempts++;
        compute_block_hash(&b, b.hash);
    } while (strncmp(b.hash, target, state.current_difficulty) != 0);

    printf("[+] Block Mined! Hash: %.15s... | Attempts: %d\n", b.hash, attempts);

    // Execute Ledger Updates
    for(int i=0; i<b.transaction_count; i++) {
        Transaction tx = b.transactions[i];
        Account* sender = get_or_create_account(tx.sender_address);
        Account* receiver = get_or_create_account(tx.receiver_address);

        sender->balance -= tx.amount;
        receiver->balance += tx.amount;
        sender->nonce++; // Increment nonce upon confirmation!

        if(strcmp(tx.transaction_type, TX_REINSURANCE) == 0) state.reinsurance_balance += tx.amount;

        // UTXO
        add_utxo(tx.transaction_id, tx.receiver_address, tx.amount);
    }

    // Reward miner
    get_or_create_account(miner)->balance += state.block_reward;

    chain.blocks[chain.length++] = b;
    remove_confirmed_from_mempool(b.transactions, b.transaction_count);
    retarget_difficulty();
}

void mine_solo() {
    mine_block(my_address);
}

void mine_pool() {
    printf("[*] Simulating Pool Mining...\n");
    mine_block("POOL_CONTRACT");
    printf("[+] Pool reward split among contributors.\n");
}
