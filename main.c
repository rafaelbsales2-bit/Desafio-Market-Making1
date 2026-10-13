#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_LEVELS 4096
#define MAX_ORDERS 100000
#define NULL_VALUE 100001

typedef struct {
    uint32_t next;
    uint32_t prev;
    uint32_t tick;
    uint32_t quantity;
} ORDER;

typedef struct {
    uint32_t HEAD_id;
    uint32_t TAIL_id;
} TICK;

ORDER orders[MAX_ORDERS];
TICK price_levels[MAX_LEVELS];

uint64_t bitmap[MAX_LEVELS/64];
uint64_t root_mask = 0;

uint32_t free_id = 0;

void push();
int32_t pop_max();
void cancel_order();
void init_system();

int main() {
    init_system(); 
    int escolha;
    int32_t max_id;
    
    while(1) {
        printf("\n==========\n[1] Adicionar Ordem\n[2] Executar Ordem de Maior Prioridade (Pop Max)\n[3] Cancelar Ordem\n[Outro] Sair\n==========\nESCOLHA: ");
        if(scanf("%d", &escolha) != 1) break;

        if(escolha == 1) {
            push();
        } else if (escolha == 2) {
            max_id = pop_max();
            if (max_id == -1) {
                printf("O livro de ofertas esta vazio.\n");
            } else {
                printf("Ordem %d (Tick: %u) removida do topo com sucesso.\n", max_id, orders[max_id].tick);
            }
        } else if(escolha == 3) {
            cancel_order();
        } else {
            break;
        }
    }
    return 0;
}

void init_system() {
    for (int i = 0; i < MAX_LEVELS; i++) {
        price_levels[i].HEAD_id = NULL_VALUE;
        price_levels[i].TAIL_id = NULL_VALUE;
        bitmap[i/64] = 0;
    }
    root_mask = 0;
    
    for (int i = 0; i < MAX_ORDERS; i++) {
        orders[i].next = i + 1;
        orders[i].quantity = 0;
    }
    orders[MAX_ORDERS - 1].next = NULL_VALUE;
    free_id = 0;
}

void push() {
    int32_t tick, quantity;

    if (free_id == NULL_VALUE) {
        printf("ERRO: O limite maximo de ordens foi atingido.\n");
        return;
    }

    printf("\nQual e o tick e a quantidade? (<tick 0-4095> <quantidade>): ");
    if (scanf("%d %d", &tick, &quantity) != 2) return;

    if (tick < 0 || tick >= MAX_LEVELS || quantity <= 0) {
        printf("ERRO: Tick fora dos limites ou quantidade invalida.\n");
        return;
    }

    // Retira o no da free list
    uint32_t new_id = free_id;
    free_id = orders[free_id].next;

    orders[new_id].next = NULL_VALUE;
    orders[new_id].quantity = quantity;
    orders[new_id].tick = tick;

    if(price_levels[tick].HEAD_id == NULL_VALUE) {
        orders[new_id].prev = NULL_VALUE;
        price_levels[tick].HEAD_id = new_id;
        price_levels[tick].TAIL_id = new_id;

        uint32_t resto = tick % 64;
        uint32_t bitmap_index = tick / 64;

        bitmap[bitmap_index] |= (1ULL << resto);
        root_mask |= (1ULL << bitmap_index);
    } else {
        // Nivel ja tem ordens, anexa no fim (TAIL)
        uint32_t tail = price_levels[tick].TAIL_id;
        orders[new_id].prev = tail;
        orders[tail].next = new_id;
        price_levels[tick].TAIL_id = new_id;
    }

    printf("Este e o ID da sua ordem: %d\n", new_id);
}

void cancel_order() {
    uint32_t ID;

    printf("Qual e o ID da ordem que voce deseja cancelar?: ");
    if(scanf("%u", &ID) != 1) return;

    if (ID >= MAX_ORDERS || orders[ID].quantity == 0) {
        printf("Ordem inexistente ou ja inativa.\n");
        return;
    }

    uint32_t t = orders[ID].tick;
  

    if(orders[ID].prev != NULL_VALUE) {
        orders[orders[ID].prev].next = orders[ID].next;
    } else {
        price_levels[t].HEAD_id = orders[ID].next;
    }
    
    if(orders[ID].next != NULL_VALUE) {
        orders[orders[ID].next].prev = orders[ID].prev;
    } else {
        price_levels[t].TAIL_id = orders[ID].prev;
    }
        
    if(price_levels[t].HEAD_id == NULL_VALUE) {
        uint32_t resto = t % 64;
        uint32_t bitmap_index = t / 64;

        bitmap[bitmap_index] &= ~(1ULL << resto);
        
        if (bitmap[bitmap_index] == 0) {
            root_mask &= ~(1ULL << bitmap_index);
        }
    } 
    
    orders[ID].quantity = 0;
    orders[ID].next = free_id;
    free_id = ID;
    
    printf("Cancelamento efetuado com sucesso!\n");
}

int32_t pop_max() {
    if (root_mask == 0) return -1; 

    uint32_t leaf_idx = 63 - __builtin_clzll(root_mask);
    uint32_t bit_idx  = 63 - __builtin_clzll(bitmap[leaf_idx]);
    
    uint32_t max_tick = (leaf_idx * 64) + bit_idx;

    uint32_t target_id = price_levels[max_tick].HEAD_id;

    uint32_t next_id = orders[target_id].next;
    price_levels[max_tick].HEAD_id = next_id;

    if (next_id != NULL_VALUE) {
        orders[next_id].prev = NULL_VALUE;
    } else {
        price_levels[max_tick].TAIL_id = NULL_VALUE;
        bitmap[leaf_idx] &= ~(1ULL << bit_idx);
        
        if (bitmap[leaf_idx] == 0) {
            root_mask &= ~(1ULL << leaf_idx);
        }
    }

    orders[target_id].quantity = 0;
    orders[target_id].next = free_id;
    free_id = target_id;

    return target_id;
}