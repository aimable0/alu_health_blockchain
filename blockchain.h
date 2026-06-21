#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <time.h>
#include <stdbool.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/sha.h>

#define MAX_TRANSACTIONS 10
#define MAX_BLOCKS 1000
#define HASH_HEX_LEN 65
#define SIG_MAX_LEN 128

// Statuses
#define STATUS_ACTIVE "ACTIVE"
#define STATUS_EXPIRED "EXPIRED"
#define STATUS_RENEWED "RENEWED"

#define TX_PENDING "PENDING"
#define TX_CONFIRMED "CONFIRMED"
#define TX_SUSPICIOUS "SUSPICIOUS"

// Tx Types
#define TX_POLICY_ENROLL "POLICY_ENROLL"
#define TX_PREMIUM "PREMIUM"
#define TX_REINSURANCE "REINSURANCE_CONTRIBUTION"
#define TX_HEALTH_REQ "SERVICE_REQUEST"
#define TX_PREAUTH_REQ "PREAUTH_REQ"
#define TX_PREAUTH_APP "PREAUTH_APPROVE"
#define TX_CLAIM_SUBMIT "CLAIM_SUBMIT"
#define TX_CLAIM_APPROVE "CLAIM_APPROVE"
#define TX_CLAIM_REJECT "CLAIM_REJECT"
#define TX_CLAIM_SETTLE "CLAIM_SETTLE"
#define TX_TRANSFER "TOKEN_TRANSFER"
#define TX_MINING_REWARD "MINING_REWARD"

typedef struct {
    char transaction_id[HASH_HEX_LEN];
    char sender_address[65];
    char receiver_address[65];
    int amount;
    char transaction_type[30];
    time_t timestamp;
    int sender_nonce;
    unsigned char digital_signature[SIG_MAX_LEN];
    unsigned int sig_len;

    // Mempool only fields (not hashed for merkle)
    int fee;
    char status[15];
} Transaction;

typedef struct {
    int block_id;
    time_t timestamp;
    int transaction_count;
    Transaction transactions[MAX_TRANSACTIONS];
    char previous_hash[HASH_HEX_LEN];
    char merkle_root[HASH_HEX_LEN];
    unsigned int nonce;
    char miner_id[65];
    int difficulty;
    char hash[HASH_HEX_LEN]; // Store its own hash for quick reference
} Block;

typedef struct {
    Block blocks[MAX_BLOCKS];
    int length;
} Blockchain;

typedef struct MempoolNode {
    Transaction tx;
    struct MempoolNode* next;
} MempoolNode;

typedef struct {
    char member_id[65];
    char policy_id[65];
    char coverage_plan[30];
    time_t enrollment_date;
    time_t expiry_date;
    char status[10];
} Policy;

typedef struct UTXO {
    char tx_id[HASH_HEX_LEN];
    char owner_address[65];
    int amount;
    bool is_spent;
    struct UTXO* next;
} UTXO;

typedef struct Account {
    char address[65];
    int balance;
    int nonce;
    struct Account* next;
} Account;

typedef struct {
    int current_difficulty;
    int block_reward;
    int last_retarget_block;
    int reinsurance_balance;
} ChainState;

// Globals
extern Blockchain chain;
extern MempoolNode* mempool_head;
extern Policy policies[500];
extern int policy_count;
extern UTXO* utxo_head;
extern Account* account_head;
extern ChainState state;
extern EC_KEY* global_key; // Simulate user's wallet
extern char my_address[65];

// Crypto
void sha256_to_hex(const char *input, size_t len, char output[HASH_HEX_LEN]);
EC_KEY* load_or_generate_keys(char* pub_addr);
void sign_transaction(Transaction* tx, EC_KEY* key);
int verify_transaction_signature(Transaction* tx);

// Merkle
void compute_merkle_root(Transaction txs[], int count, char root_out[HASH_HEX_LEN]);

// Mempool
void add_to_mempool(Transaction tx);
void view_mempool();
void remove_confirmed_from_mempool(Transaction confirmed_txs[], int count);
void fraud_review();
void approve_suspicious(const char* tx_id);
void reject_suspicious(const char* tx_id);

// Ledger
Account* get_or_create_account(const char* address);
void add_utxo(const char* tx_id, const char* owner, int amount);
void mark_utxo_spent(const char* tx_id);
int get_utxo_balance(const char* address);
int spend_utxos(const char* sender, int required_amount, const char* new_tx_id);
void print_balances();
void print_utxos();
void print_account_history(const char* address);

// Insurance
void enroll_policy(const char* member_id, const char* coverage);
void renew_policy(const char* policy_id);
void pay_premium(const char* sender, int amount);
void submit_claim(const char* provider, const char* policy_id, int amount);
void approve_claim(const char* claim_id);
void settle_claim(const char* provider, int amount);
void check_policy_expiry();

// Mining
void mine_solo();
void mine_pool();
void retarget_difficulty();
void compute_block_hash(Block* b, char* out);

// Persistence
void save_state();
void load_state();
void verify_blockchain();

#endif
