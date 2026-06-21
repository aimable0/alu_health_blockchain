#include "blockchain.h"
#include <stdio.h>
#include <string.h>

Policy policies[500];
int policy_count = 0;

void enroll_policy(const char* member_id, const char* coverage) {
    Policy p;
    strcpy(p.member_id, member_id);
    sprintf(p.policy_id, "POL_%ld", time(NULL));
    strcpy(p.coverage_plan, coverage);
    p.enrollment_date = time(NULL);
    p.expiry_date = p.enrollment_date + (365 * 24 * 3600);
    strcpy(p.status, STATUS_ACTIVE);
    policies[policy_count++] = p;

    Transaction tx = {0};
    strcpy(tx.sender_address, my_address);
    strcpy(tx.receiver_address, "SYSTEM");
    tx.amount = 0;
    strcpy(tx.transaction_type, TX_POLICY_ENROLL);
    tx.timestamp = time(NULL);
    tx.fee = 1;
    tx.sender_nonce = get_or_create_account(my_address)->nonce;
    sign_transaction(&tx, global_key);
    add_to_mempool(tx);
    printf("[+] Policy enrolled: %s\n", p.policy_id);
}

void pay_premium(const char* sender, int amount) {
    // 1. Standard Premium Tx
    Transaction tx1 = {0};
    strcpy(tx1.sender_address, sender);
    strcpy(tx1.receiver_address, "INSURANCE_POOL");
    tx1.amount = amount;
    strcpy(tx1.transaction_type, TX_PREMIUM);
    tx1.timestamp = time(NULL);
    tx1.fee = 5;
    tx1.sender_nonce = get_or_create_account(sender)->nonce;
    sign_transaction(&tx1, global_key);
    add_to_mempool(tx1);

    // 2. Reinsurance Tx (5%)
    Transaction tx2 = {0};
    strcpy(tx2.sender_address, "INSURANCE_POOL");
    strcpy(tx2.receiver_address, "REINSURANCE_POOL");
    tx2.amount = amount * 0.05;
    strcpy(tx2.transaction_type, TX_REINSURANCE);
    tx2.timestamp = time(NULL);
    tx2.fee = 5;
    tx2.sender_nonce = get_or_create_account("INSURANCE_POOL")->nonce;
    strcpy(tx2.transaction_id, "SYS_GEN_REINSURANCE"); // System generated
    strcpy(tx2.status, TX_PENDING);

    MempoolNode* node = malloc(sizeof(MempoolNode));
    node->tx = tx2; node->next = mempool_head; mempool_head = node;
    printf("[+] Premium and Reinsurance transactions queued.\n");
}

void check_policy_expiry() {
    time_t now = time(NULL);
    for(int i=0; i<policy_count; i++) {
        if(policies[i].expiry_date < now && strcmp(policies[i].status, STATUS_ACTIVE) == 0) {
            strcpy(policies[i].status, STATUS_EXPIRED);
            printf("[!] Policy %s has EXPIRED.\n", policies[i].policy_id);
        }
    }
}

void submit_claim(const char* provider, const char* policy_id, int amount) {
    check_policy_expiry();
    for(int i=0; i<policy_count; i++) {
        if(strcmp(policies[i].policy_id, policy_id) == 0) {
            if(strcmp(policies[i].status, STATUS_EXPIRED) == 0) {
                printf("[!] Claim Rejected: Policy EXPIRED.\n");
                return;
            }
            Transaction tx = {0};
            strcpy(tx.sender_address, provider);
            strcpy(tx.receiver_address, "INSURANCE_POOL");
            tx.amount = amount;
            strcpy(tx.transaction_type, TX_CLAIM_SUBMIT);
            tx.timestamp = time(NULL);
            tx.fee = 2;
            tx.sender_nonce = get_or_create_account(provider)->nonce;
            sign_transaction(&tx, global_key);
            add_to_mempool(tx);
            return;
        }
    }
    printf("[!] Policy not found.\n");
}

void settle_claim(const char* provider, int amount) {
    if(amount > 1000) {
        int excess = amount - 1000;
        Transaction t1 = {0};
        strcpy(t1.sender_address, "INSURANCE_POOL");
        strcpy(t1.receiver_address, provider);
        t1.amount = 1000;
        strcpy(t1.transaction_type, TX_CLAIM_SETTLE);
        t1.timestamp = time(NULL);
        t1.sender_nonce = get_or_create_account("INSURANCE_POOL")->nonce;
        sprintf(t1.transaction_id, "SETTLE_MAIN_%ld", time(NULL));
        strcpy(t1.status, TX_PENDING);

        Transaction t2 = {0};
        strcpy(t2.sender_address, "REINSURANCE_POOL");
        strcpy(t2.receiver_address, provider);
        t2.amount = excess;
        strcpy(t2.transaction_type, TX_CLAIM_SETTLE);
        t2.timestamp = time(NULL);
        t2.sender_nonce = get_or_create_account("REINSURANCE_POOL")->nonce;
        sprintf(t2.transaction_id, "SETTLE_REINS_%ld", time(NULL));
        strcpy(t2.status, TX_PENDING);

        MempoolNode* n1 = malloc(sizeof(MempoolNode)); n1->tx=t1; n1->next=mempool_head; mempool_head=n1;
        MempoolNode* n2 = malloc(sizeof(MempoolNode)); n2->tx=t2; n2->next=mempool_head; mempool_head=n2;
        printf("[+] Split settlement queued: 1000 from Main, %d from Reinsurance.\n", excess);
    } else {
        Transaction t1 = {0};
        strcpy(t1.sender_address, "INSURANCE_POOL");
        strcpy(t1.receiver_address, provider);
        t1.amount = amount;
        strcpy(t1.transaction_type, TX_CLAIM_SETTLE);
        t1.timestamp = time(NULL);
        t1.sender_nonce = get_or_create_account("INSURANCE_POOL")->nonce;
        sprintf(t1.transaction_id, "SETTLE_%ld", time(NULL));
        strcpy(t1.status, TX_PENDING);
        MempoolNode* n1 = malloc(sizeof(MempoolNode)); n1->tx=t1; n1->next=mempool_head; mempool_head=n1;
        printf("[+] Settlement queued.\n");
    }
}
