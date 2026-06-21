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
            case 1: {
                char mem_id[50], plan[50];
                printf("Enter Member ID: "); scanf("%49s", mem_id);
                printf("Enter Coverage Plan: "); scanf("%49s", plan);
                enroll_policy(mem_id, plan);
                break;
            }
            case 2: {
                int prem;
                printf("Enter Premium Amount: "); scanf("%d", &prem);
                pay_premium(my_address, prem);
                break;
            }
            case 3: {
                char prov[50], pol_id[50]; int amt;
                printf("Enter Provider ID: "); scanf("%49s", prov);
                printf("Enter Policy ID: "); scanf("%49s", pol_id);
                printf("Enter Claim Amount: "); scanf("%d", &amt);
                submit_claim(prov, pol_id, amt);
                break;
            }
            case 4: {
                char prov[50]; int amt;
                printf("Enter Provider ID: "); scanf("%49s", prov);
                printf("Enter Settlement Amount: "); scanf("%d", &amt);
                settle_claim(prov, amt);
                break;
            }
            case 5: {
                char rec[50]; int amt;
                printf("Enter Recipient Address: "); scanf("%49s", rec);
                printf("Enter Amount: "); scanf("%d", &amt);
                Transaction tx = {0};
                strcpy(tx.sender_address, my_address);
                strcpy(tx.receiver_address, rec);
                tx.amount = amt;
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
                    printf("ID: %s | Member: %s | Status: %s\n", policies[i].policy_id, policies[i].member_id, policies[i].status);
                break;
            case 8: switch_ledger_model(); break;
            case 9: view_mempool(); break;
            case 10: mine_solo(); break;
            case 11: mine_pool(); break;
            case 12: print_balances(); break;
            case 13: {
                fraud_review();
                char action[10], tx_id[100];
                printf("\nEnter 'APPROVE' or 'REJECT' followed by TX_ID (or 'SKIP'): ");
                scanf("%9s", action);
                if(strcmp(action, "SKIP") != 0) {
                    scanf("%99s", tx_id);
                    if(strcmp(action, "APPROVE") == 0) approve_suspicious(tx_id);
                    else if(strcmp(action, "REJECT") == 0) reject_suspicious(tx_id);
                }
                break;
            }
            case 14: verify_blockchain(); break;
            case 15: save_state(); return 0;
            default: printf("Invalid.\n");
        }
    }
    return 0;
}
