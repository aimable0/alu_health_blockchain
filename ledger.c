#include "blockchain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int current_model = 2; // 2 = ACCOUNT, 1 = UTXO

void switch_ledger_model() {
    current_model = (current_model == 1) ? 2 : 1;
    printf("\n[+] Switched ledger model to %s.\n", current_model == 1 ? "UTXO" : "Account-Based");
}

Account* account_head = NULL;
UTXO* utxo_head = NULL;

Account* get_or_create_account(const char* address) {
    Account* curr = account_head;
    while(curr) {
        if(strcmp(curr->address, address) == 0) return curr;
        curr = curr->next;
    }
    Account* new_acc = malloc(sizeof(Account));
    strcpy(new_acc->address, address);
    new_acc->balance = 0;
    new_acc->nonce = 0;
    new_acc->next = account_head;
    account_head = new_acc;
    return new_acc;
}

void add_utxo(const char* tx_id, const char* owner, int amount) {
    UTXO* u = malloc(sizeof(UTXO));
    strcpy(u->tx_id, tx_id);
    strcpy(u->owner_address, owner);
    u->amount = amount;
    u->is_spent = false;
    u->next = utxo_head;
    utxo_head = u;
}

void mark_utxo_spent(const char* tx_id) {
    UTXO* curr = utxo_head;
    while(curr) {
        if(strcmp(curr->tx_id, tx_id) == 0) {
            curr->is_spent = true;
            return;
        }
        curr = curr->next;
    }
}

int get_utxo_balance(const char* address) {
    int bal = 0;
    UTXO* curr = utxo_head;
    while(curr) {
        if(strcmp(curr->owner_address, address) == 0 && !curr->is_spent)
            bal += curr->amount;
        curr = curr->next;
    }
    return bal;
}

int spend_utxos(const char* sender, int required_amount, const char* new_tx_id) {
    int gathered = 0;
    UTXO* curr = utxo_head;
    while(curr && gathered < required_amount) {
        if(strcmp(curr->owner_address, sender) == 0 && !curr->is_spent) {
            gathered += curr->amount;
            curr->is_spent = true;
        }
        curr = curr->next;
    }
    if (gathered < required_amount) return 0; // Insufficient

    int change = gathered - required_amount;
    if (change > 0) {
        char change_id[100];
        sprintf(change_id, "%s_CHANGE", new_tx_id);
        add_utxo(change_id, sender, change);
    }
    return 1;
}

void print_balances() {
    printf("\n--- ACCOUNT BALANCES ---\n");
    Account* curr = account_head;
    while(curr) {
        printf("Addr: %.10s... | Bal: %d | Nonce: %d\n", curr->address, curr->balance, curr->nonce);
        curr = curr->next;
    }
    printf("Reinsurance Pool Balance: %d AHT\n", state.reinsurance_balance);
}

void print_utxos() {
    printf("\n--- UTXO SET ---\n");
    UTXO* curr = utxo_head;
    while(curr) {
        printf("Owner: %.10s... | Amt: %d | Spent: %d | TX: %.10s\n", curr->owner_address, curr->amount, curr->is_spent, curr->tx_id);
        curr = curr->next;
    }
}
