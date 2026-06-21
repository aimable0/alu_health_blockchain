#include "blockchain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

MempoolNode* mempool_head = NULL;

void check_fraud(Transaction* tx) {
    strcpy(tx->status, TX_PENDING);

    // Rule 1: Duplicate TX
    MempoolNode* curr = mempool_head;
    while(curr) {
        if(strcmp(curr->tx.transaction_id, tx->transaction_id) == 0) {
            strcpy(tx->status, TX_SUSPICIOUS);
            printf("[!] FRAUD ALERT: Duplicate transaction detected.\n");
            return;
        }
        curr = curr->next;
    }

    // Rule 2: High Frequency (simplified mempool check)
    int freq = 0;
    curr = mempool_head;
    while(curr) {
        if(strcmp(curr->tx.sender_address, tx->sender_address) == 0) freq++;
        curr = curr->next;
    }
    if(freq >= 2) {
        strcpy(tx->status, TX_SUSPICIOUS);
        printf("[!] FRAUD ALERT: High frequency sender.\n");
        return;
    }
}

void add_to_mempool(Transaction tx) {
    if(!verify_transaction_signature(&tx)) {
        printf("[!] Invalid signature, TX rejected.\n");
        return;
    }

    Account* acc = get_or_create_account(tx.sender_address);
    if(strcmp(tx.sender_address, "SYSTEM") != 0 && tx.sender_nonce != acc->nonce) {
        printf("[!] REPLAY PROTECTION: Invalid Nonce. Expected %d, got %d.\n", acc->nonce, tx.sender_nonce);
        return;
    }

    check_fraud(&tx);

    MempoolNode* new_node = malloc(sizeof(MempoolNode));
    new_node->tx = tx;
    new_node->next = NULL;

    // Insert sorted by Fee (DESC), then Timestamp (ASC)
    if (!mempool_head || (new_node->tx.fee > mempool_head->tx.fee) ||
        (new_node->tx.fee == mempool_head->tx.fee && new_node->tx.timestamp < mempool_head->tx.timestamp)) {
        new_node->next = mempool_head;
        mempool_head = new_node;
    } else {
        MempoolNode* curr = mempool_head;
        while (curr->next && (curr->next->tx.fee > new_node->tx.fee ||
              (curr->next->tx.fee == new_node->tx.fee && curr->next->tx.timestamp <= new_node->tx.timestamp))) {
            curr = curr->next;
        }
        new_node->next = curr->next;
        curr->next = new_node;
    }
    printf("[+] TX %s added to Mempool (Status: %s).\n", tx.transaction_type, tx.status);
}

void view_mempool() {
    printf("\n--- MEMPOOL ---\n");
    MempoolNode* curr = mempool_head;
    if(!curr) printf("Empty.\n");
    while(curr) {
        printf("TX: %.10s... | Type: %-20s | Fee: %d | Status: %s\n", curr->tx.transaction_id, curr->tx.transaction_type, curr->tx.fee, curr->tx.status);
        curr = curr->next;
    }
}

void remove_confirmed_from_mempool(Transaction confirmed_txs[], int count) {
    for(int i=0; i<count; i++) {
        MempoolNode* curr = mempool_head;
        MempoolNode* prev = NULL;
        while(curr) {
            if(strcmp(curr->tx.transaction_id, confirmed_txs[i].transaction_id) == 0) {
                if(prev) prev->next = curr->next;
                else mempool_head = curr->next;
                free(curr);
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }
}

void fraud_review() {
    printf("\n--- SUSPICIOUS TRANSACTIONS ---\n");
    MempoolNode* curr = mempool_head;
    while(curr) {
        if(strcmp(curr->tx.status, TX_SUSPICIOUS) == 0) {
            printf("ID: %s | Type: %s | Sender: %.10s\n", curr->tx.transaction_id, curr->tx.transaction_type, curr->tx.sender_address);
        }
        curr = curr->next;
    }
}

void approve_suspicious(const char* tx_id) {
    MempoolNode* curr = mempool_head;
    while(curr) {
        if(strcmp(curr->tx.transaction_id, tx_id) == 0) {
            strcpy(curr->tx.status, TX_PENDING);
            printf("[+] Transaction approved and set to PENDING.\n");
            return;
        }
        curr = curr->next;
    }
}

void reject_suspicious(const char* tx_id) {
    MempoolNode* curr = mempool_head;
    MempoolNode* prev = NULL;
    while(curr) {
        if(strcmp(curr->tx.transaction_id, tx_id) == 0) {
            if(prev) prev->next = curr->next;
            else mempool_head = curr->next;
            free(curr);
            printf("[+] Transaction discarded.\n");
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}
