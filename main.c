#include "blockchain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Blockchain chain;
ChainState state;
EC_KEY* global_key = NULL;
char my_address[65];

void print_menu() {
    printf("\n====== ALU HEALTH INSURANCE BLOCKCHAIN ======\n");
    printf("1. Enroll Policy         9. View Mempool\n");
    printf("2. Pay Premium           10. Mine Solo\n");
    printf("3. Submit Claim          11. Mine Pool\n");
    printf("4. Settle Claim          12. View Balances\n");
    printf("5. Token Transfer        13. Fraud Review\n");
    printf("6. UTXO View             14. Verify Blockchain\n");
    printf("7. Policy View           15. Save & Exit\n");
    printf("8. Switch Ledger Model\n");
    printf("Choose [1-15]: ");
}

int main() {
    global_key = load_or_generate_keys(my_address);
    printf("[*] Wallet Addr: %.15s...\n", my_address);
    load_state();

    // Give genesis funds
    get_or_create_account(my_address)->balance = 10000;

    int choice;
    while(1) {
        print_menu();
        if(scanf("%d", &choice) != 1) { getchar(); continue; }

        switch(choice) {
            case 1: enroll_policy("MEM_101", "GOLD_PLAN"); break;
            case 2: pay_premium(my_address, 100); break;
            case 3: submit_claim("HOSP_KIGALI", "POL_XXXX", 500); break;
            case 4: settle_claim("HOSP_KIGALI", 1200); break; // Triggers split!
            case 5: {
                Transaction tx = {0};
                strcpy(tx.sender_address, my_address);
                strcpy(tx.receiver_address, "ALU_STUDENT");
                tx.amount = 50;
                strcpy(tx.transaction_type, TX_TRANSFER);
                tx.timestamp = time(NULL);
                tx.sender_nonce = get_or_create_account(my_address)->nonce;
                sign_transaction(&tx, global_key);
                add_to_mempool(tx);
                break;
            }
            case 6: print_utxos(); break;
            case 7:
                printf("\n--- POLICIES ---\n");
                for(int i=0; i<policy_count; i++)
                    printf("ID: %s | Status: %s\n", policies[i].policy_id, policies[i].status);
                break;
            case 8: printf("Toggle Model Placeholder.\n"); break;
            case 9: view_mempool(); break;
            case 10: mine_solo(); break;
            case 11: mine_pool(); break;
            case 12: print_balances(); break;
            case 13: fraud_review(); break;
            case 14: verify_blockchain(); break;
            case 15: save_state(); return 0;
            default: printf("Invalid.\n");
        }
    }
    return 0;
}
