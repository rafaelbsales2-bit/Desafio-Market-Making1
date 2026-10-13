# Desafio Market Making: Limit Order Book de Baixa Latência em C

Uma implementação eficiente e de alto desempenho de um **Livro de Ofertas (Limit Order Book)** simplificado voltado para aplicações de trading e Market Making. O motor foi desenhado com foco em **baixa latência** e complexidade **$O(1)$** para todas as operações críticas do mercado.

---

## 🚀 Destaques e Arquitetura

O projeto resolve o desafio de gerenciar ordens com regras de **Prioridade Preço-Tempo (FIFO)** sem incorrer em overheads de alocação de memória dinâmica (`malloc`/`free`) ou buscas lineares:

- **Busca de Melhor Preço em $O(1)$ com Bitmaps em 2 Níveis:**
  - O espaço de ticks (preços de `0` a `4095`) é mapeado em 64 blocos de 64 bits (`uint64_t bitmap[64]`).
  - Um mapa raiz (`uint64_t root_mask`) rastreia quais blocos de 64 bits contêm níveis de preços ativos.
  - Utiliza a instrução intrínseca de CPU `__builtin_clzll` (*Count Leading Zeros*) para encontrar o tick máximo ativo em pouquíssimos ciclos de clock, mapeando diretamente para instruções em hardware (`BSR`/`LZCNT` em x86-64).

- **Pool Estático de Ordens (Memory Pool com Free List):**
  - Armazenamento contíguo de até 100.000 ordens (`ORDER orders[MAX_ORDERS]`).
  - Alocação e desalocação em $O(1)$ usando uma lista ligada de nós livres (`free_id`), eliminando fragmentação de heap e *cache misses*.

- **Lista Duplamente Encadeada por Nível de Preço:**
  - Cada nível de tick (`TICK price_levels[MAX_LEVELS]`) mantém ponteiros para a cabeça (`HEAD_id`) e cauda (`TAIL_id`).
  - Inserções entram no final da fila (prioridade de tempo/FIFO).
  - Cancelamentos de ordens por ID são estritamente $O(1)$, desacoplando o nó diretamente pelos índices `prev` e `next` sem busca linear.

---

## 📊 Complexidade Temporal das Operações

| Operação | Função | Complexidade | Descrição |
|---|---|:---:|---|
| **Inserção de Ordem** | `push()` | **$O(1)$** | Pega nó livre da Free List e insere na cauda do tick correspondente. Ativa bits no bitmap. |
| **Execução Topo / Melhor Preço** | `pop_max()` | **$O(1)$** | Encontra o maior tick via bitmasks com `__builtin_clzll` e remove a ordem mais antiga (HEAD). |
| **Cancelamento de Ordem** | `cancel_order()` | **$O(1)$** | Remove o nó da lista duplamente encadeada pelo ID e o devolve para a Free List. |

---

## 🛠️ Constantes e Estruturas

```c
#define MAX_LEVELS 4096      // Níveis de ticks suportados (0 a 4095)
#define MAX_ORDERS 100000    // Capacidade estática total de ordens simultâneas
#define NULL_VALUE 100001    // Sentinela para ponteiro nulo (índice inválido)
```

### Estruturas Principais
- `ORDER`: Contém `tick`, `quantity`, `next` e `prev` (apontadores por índice no array estático).
- `TICK`: Contém `HEAD_id` (ordem mais prioritária do nível) e `TAIL_id` (ordem mais recente do nível).

---

## 💻 Como Compilar e Executar

### Pré-requisitos
- Compilador C compatível com C99 ou superior (GCC, Clang ou MSVC/MinGW).
- Suporte a `__builtin_clzll` (padrão em GCC e Clang).

### Compilação

Para compilar com otimizações voltadas para alto desempenho:

```bash
gcc -O3 -march=native -Wall -Wextra main.c -o market_maker
```

### Execução

```bash
./market_maker
```

No Windows (PowerShell/CMD):
```powershell
.\market_maker.exe
```

---

## 🎮 Exemplo de Uso Interativo

O programa disponibiliza um menu no terminal para simulação em tempo real:

```text
==========
[1] Adicionar Ordem
[2] Executar Ordem de Maior Prioridade (Pop Max)
[3] Cancelar Ordem
[Outro] Sair
==========
ESCOLHA: 1

Qual e o tick e a quantidade? (<tick 0-4095> <quantidade>): 1500 50
Este e o ID da sua ordem: 0
```

---

## 📝 Licença

Projeto desenvolvido para fins didáticos e desafios técnicos na área de trading algorítmico e sistemas de baixa latência.
