#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma pack(push, 1)

#define BLOCO_PRODUTOS 1000
#define BLOCO_ACESSOS  10000

#define ARQ_PRODUTOS "produtos.dat"
#define ARQ_ACESSOS  "acessos.dat"
#define IDX_PRODUTOS "indice_produtos.indice"
#define IDX_ACESSOS  "indice_acessos.indice"

typedef struct {
    int id_produto;
    char marca[30];
    char categoria[30];
    float preco;
    char quebra_linha;
} Produto;

typedef struct {
    int id_evento;
    int id_produto;
    char tipo_evento[10];
    char quebra_linha;
} Acesso;

typedef struct {
    int id_produto;
    long deslocamento;
} IndiceParcial;
#pragma pack(pop)

typedef struct {
    int id_produto;
    char marca[30];
    char categoria[30];
    float preco;
    int visualizacoes;
    int carrinhos;
    int compras;
    float taxa_conversao;
    float taxa_abandono;
    float taxa_sem_carrinho;
} EstatisticaProduto;


void pad_string(char *texto_destino, const char *texto_origem, int tamanho) {
    int comprimento = (int)strlen(texto_origem);
    for (int contador = 0; contador < tamanho; contador++) {
        if (contador < comprimento && texto_origem[contador] != '\r' && texto_origem[contador] != '\n') {
            texto_destino[contador] = texto_origem[contador];
        } 
        else {
            texto_destino[contador] = ' ';
        }
    }
}

void imprimir_produto(const Produto *produto, long posicao) {
    if (produto->marca[0] == '*') {
        printf("[Registro #%ld] ID: %d | [REMOVIDO]\n", posicao, produto->id_produto);
        return;
    }
    printf("[Registro #%ld] ID: %-10d | Marca: %.20s | Categoria: %.25s | Preco: $%.2f\n", posicao, produto->id_produto, produto->marca, produto->categoria, produto->preco);
}

void imprimir_acesso(const Acesso *acesso, long posicao) {
    printf("[Registro #%ld] ID Evento: %-10d | ID Produto: %-10d | Tipo: %.10s\n", posicao, acesso->id_evento, acesso->id_produto, acesso->tipo_evento);
}

void construir_indice_produtos() {
    FILE *arquivo_dados = fopen(ARQ_PRODUTOS, "rb");
    if (!arquivo_dados) return;
    FILE *arquivo_indice = fopen(IDX_PRODUTOS, "wb");
    if (!arquivo_indice) { fclose(arquivo_dados); return; }

    fseek(arquivo_dados, 0, SEEK_END);
    long total_registros = ftell(arquivo_dados) / sizeof(Produto);
    Produto produto;

    for (long contador = 0; contador < total_registros; contador += BLOCO_PRODUTOS) {
        long deslocamento = contador * sizeof(Produto);
        fseek(arquivo_dados, deslocamento, SEEK_SET);
        if (fread(&produto, sizeof(Produto), 1, arquivo_dados) == 1) {
            IndiceParcial indice = {produto.id_produto, deslocamento};
            fwrite(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
        }
    }
    fclose(arquivo_dados);
    fclose(arquivo_indice);
    printf("[Indice] %s gravado com sucesso.\n", IDX_PRODUTOS);
}

void construir_indice_acessos() {
    FILE *arquivo_dados = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_dados) return;
    FILE *arquivo_indice = fopen(IDX_ACESSOS, "wb");
    if (!arquivo_indice) { fclose(arquivo_dados); return; }

    fseek(arquivo_dados, 0, SEEK_END);
    long total_registros = ftell(arquivo_dados) / sizeof(Acesso);
    Acesso acesso;

    for (long contador = 0; contador < total_registros; contador += BLOCO_ACESSOS) {
        long deslocamento = contador * sizeof(Acesso);
        fseek(arquivo_dados, deslocamento, SEEK_SET);
        if (fread(&acesso, sizeof(Acesso), 1, arquivo_dados) == 1) {
            IndiceParcial indice = {acesso.id_produto, deslocamento};
            fwrite(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
        }
    }
    fclose(arquivo_dados);
    fclose(arquivo_indice);
    printf("[Indice] %s gravado com sucesso.\n", IDX_ACESSOS);
}

void carregar_ou_criar_indices() {
    FILE *arquivo_produtos = fopen(IDX_PRODUTOS, "rb");
    if (!arquivo_produtos) construir_indice_produtos();
    else fclose(arquivo_produtos);

    FILE *arquivo_acessos = fopen(IDX_ACESSOS, "rb");
    if (!arquivo_acessos) construir_indice_acessos();
    else fclose(arquivo_acessos);
}

void mostrar_produtos(long inicio, long quantidade) {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb");
    if (!arquivo) return;
    fseek(arquivo, 0, SEEK_END);
    long total_registros = ftell(arquivo) / sizeof(Produto);
    printf("Total de produtos no ficheiro: %ld\n", total_registros);

    if (inicio >= 0 && inicio < total_registros) {
        fseek(arquivo, inicio * sizeof(Produto), SEEK_SET);
        Produto produto;
        for (long contador = 0; contador < quantidade && fread(&produto, sizeof(Produto), 1, arquivo) == 1; contador++) {
            imprimir_produto(&produto, inicio + contador);
        }
    }
    fclose(arquivo);
}

void mostrar_acessos(long inicio, long quantidade) {
    FILE *arquivo = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo) return;
    fseek(arquivo, 0, SEEK_END);
    long total_registros = ftell(arquivo) / sizeof(Acesso);
    printf("Total de acessos no ficheiro: %ld\n", total_registros);

    if (inicio >= 0 && inicio < total_registros) {
        fseek(arquivo, inicio * sizeof(Acesso), SEEK_SET);
        Acesso acesso;
        for (long contador = 0; contador < quantidade && fread(&acesso, sizeof(Acesso), 1, arquivo) == 1; contador++) {
            imprimir_acesso(&acesso, inicio + contador);
        }
    }
    fclose(arquivo);
}

long pesquisa_binaria_direta_produtos(int id_produto_busca, Produto *resultado) {
    FILE *arquivo = fopen(ARQ_PRODUTOS, "rb");
    if (!arquivo) return -1;
    fseek(arquivo, 0, SEEK_END);
    long limite_esquerdo = 0, limite_direito = (ftell(arquivo) / sizeof(Produto)) - 1;

    while (limite_esquerdo <= limite_direito) {
        long posicao_meio = limite_esquerdo + (limite_direito - limite_esquerdo) / 2;
        fseek(arquivo, posicao_meio * sizeof(Produto), SEEK_SET);
        fread(resultado, sizeof(Produto), 1, arquivo);
        if (resultado->id_produto == id_produto_busca) {
            fclose(arquivo);
            return (resultado->marca[0] == '*') ? -2 : posicao_meio;
        } else if (resultado->id_produto < id_produto_busca) limite_esquerdo = posicao_meio + 1;
        else limite_direito = posicao_meio - 1;
    }
    fclose(arquivo);
    return -1;
}

long pesquisa_binaria_direta_acessos(int id_produto_consultado, Acesso *resultado) {
    FILE *arquivo = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo) return -1;
    fseek(arquivo, 0, SEEK_END);
    long limite_esquerdo = 0, limite_direito = (ftell(arquivo) / sizeof(Acesso)) - 1, posicao_encontrada = -1;

    while (limite_esquerdo <= limite_direito) {
        long posicao_meio = limite_esquerdo + (limite_direito - limite_esquerdo) / 2;
        fseek(arquivo, posicao_meio * sizeof(Acesso), SEEK_SET);
        fread(resultado, sizeof(Acesso), 1, arquivo);
        if (resultado->id_produto == id_produto_consultado) {
            posicao_encontrada = posicao_meio;
            limite_direito = posicao_meio - 1;
        } else if (resultado->id_produto < id_produto_consultado) limite_esquerdo = posicao_meio + 1;
        else limite_direito = posicao_meio - 1;
    }
    if (posicao_encontrada != -1) {
        fseek(arquivo, posicao_encontrada * sizeof(Acesso), SEEK_SET);
        fread(resultado, sizeof(Acesso), 1, arquivo);
    }
    fclose(arquivo);
    return posicao_encontrada;
}

long consultar_produto_por_indice(int id_produto_busca, Produto *resultado) {
    FILE *arquivo_indice = fopen(IDX_PRODUTOS, "rb");
    if (!arquivo_indice) return -1;
    fseek(arquivo_indice, 0, SEEK_END);
    long limite_esquerdo = 0, limite_direito = (ftell(arquivo_indice) / sizeof(IndiceParcial)) - 1, bloco_indice = -1;
    IndiceParcial indice;

    while (limite_esquerdo <= limite_direito) {
        long posicao_meio = limite_esquerdo + (limite_direito - limite_esquerdo) / 2;
        fseek(arquivo_indice, posicao_meio * sizeof(IndiceParcial), SEEK_SET);
        fread(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
        if (indice.id_produto <= id_produto_busca) { bloco_indice = posicao_meio; limite_esquerdo = posicao_meio + 1; }
        else limite_direito = posicao_meio - 1;
    }
    if (bloco_indice == -1) { fclose(arquivo_indice); return -1; }

    fseek(arquivo_indice, bloco_indice * sizeof(IndiceParcial), SEEK_SET);
    fread(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
    fclose(arquivo_indice);

    FILE *arquivo_dados = fopen(ARQ_PRODUTOS, "rb");
    if (!arquivo_dados) return -1;
    fseek(arquivo_dados, indice.deslocamento, SEEK_SET);

    for (int contador = 0; contador < BLOCO_PRODUTOS && fread(resultado, sizeof(Produto), 1, arquivo_dados) == 1; contador++) {
        if (resultado->id_produto == id_produto_busca) {
            fclose(arquivo_dados);
            return (resultado->marca[0] == '*') ? -2 : (indice.deslocamento / sizeof(Produto)) + contador;
        }
        if (resultado->id_produto > id_produto_busca) break;
    }
    fclose(arquivo_dados);
    return -1;
}

long consultar_acesso_por_indice(int id_produto_consultado, Acesso *resultado) {
    FILE *arquivo_indice = fopen(IDX_ACESSOS, "rb");
    if (!arquivo_indice) return -1;
    fseek(arquivo_indice, 0, SEEK_END);
    long limite_esquerdo = 0, limite_direito = (ftell(arquivo_indice) / sizeof(IndiceParcial)) - 1, bloco_indice = 0;
    IndiceParcial indice;

    while (limite_esquerdo <= limite_direito) {
        long posicao_meio = limite_esquerdo + (limite_direito - limite_esquerdo) / 2;
        fseek(arquivo_indice, posicao_meio * sizeof(IndiceParcial), SEEK_SET);
        fread(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
        if (indice.id_produto < id_produto_consultado) { bloco_indice = posicao_meio; limite_esquerdo = posicao_meio + 1; }
        else { bloco_indice = posicao_meio; limite_direito = posicao_meio - 1; }
    }
    if (bloco_indice > 0 && indice.id_produto >= id_produto_consultado) bloco_indice--;

    fseek(arquivo_indice, bloco_indice * sizeof(IndiceParcial), SEEK_SET);
    fread(&indice, sizeof(IndiceParcial), 1, arquivo_indice);
    fclose(arquivo_indice);

    FILE *arquivo_dados = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_dados) return -1;
    fseek(arquivo_dados, indice.deslocamento, SEEK_SET);
    long pos_atual = indice.deslocamento / sizeof(Acesso);

    while (fread(resultado, sizeof(Acesso), 1, arquivo_dados) == 1) {
        if (resultado->id_produto == id_produto_consultado) { fclose(arquivo_dados); return pos_atual; }
        if (resultado->id_produto > id_produto_consultado) break;
        pos_atual++;
    }
    fclose(arquivo_dados);
    return -1;
}

void inserir_produto_ordenado(int id_produto, const char *marca, const char *categoria, float preco) {
    Produto produto;
    if (pesquisa_binaria_direta_produtos(id_produto, &produto) >= 0) {
        printf("Erro: Produto ID %d ja existe!\n", id_produto);
        return;
    }

    Produto novo_produto;
    novo_produto.id_produto = id_produto;
    pad_string(novo_produto.marca, marca, 30);
    pad_string(novo_produto.categoria, categoria, 30);
    novo_produto.preco = preco;
    novo_produto.quebra_linha = '\n';

    FILE *arquivo = fopen(ARQ_PRODUTOS, "r+b");
    if (!arquivo) return;
    fseek(arquivo, 0, SEEK_END);
    long total_registros = ftell(arquivo) / sizeof(Produto);
    long limite_esquerdo = 0, limite_direito = total_registros - 1, posicao_insercao = total_registros;

    while (limite_esquerdo <= limite_direito) {
        long posicao_meio = limite_esquerdo + (limite_direito - limite_esquerdo) / 2;
        fseek(arquivo, posicao_meio * sizeof(Produto), SEEK_SET);
        fread(&produto, sizeof(Produto), 1, arquivo);
        if (produto.id_produto == id_produto && produto.marca[0] == '*') {
            fseek(arquivo, posicao_meio * sizeof(Produto), SEEK_SET);
            fwrite(&novo_produto, sizeof(Produto), 1, arquivo);
            fclose(arquivo);
            printf("Produto ID %d reativado na posicao #%ld.\n", id_produto, posicao_meio);
            construir_indice_produtos();
            return;
        } 
        else if (produto.id_produto < id_produto) limite_esquerdo = posicao_meio + 1;
        else { posicao_insercao = posicao_meio; limite_direito = posicao_meio - 1; }
    }

    Produto buffer_produtos[2048];
    long registros_restantes = total_registros - posicao_insercao, cursor_registros = total_registros;
    while (registros_restantes > 0) {
        int quantidade = (registros_restantes > 2048) ? 2048 : (int)registros_restantes;
        long inicio = cursor_registros - quantidade;
        fseek(arquivo, inicio * sizeof(Produto), SEEK_SET);
        fread(buffer_produtos, sizeof(Produto), quantidade, arquivo);
        fseek(arquivo, (inicio + 1) * sizeof(Produto), SEEK_SET);
        fwrite(buffer_produtos, sizeof(Produto), quantidade, arquivo);
        cursor_registros -= quantidade;
        registros_restantes -= quantidade;
    }

    fseek(arquivo, posicao_insercao * sizeof(Produto), SEEK_SET);
    fwrite(&novo_produto, sizeof(Produto), 1, arquivo);
    fclose(arquivo);
    printf("Produto ID %d inserido na posicao #%ld.\n", id_produto, posicao_insercao);
    construir_indice_produtos();
}

void remover_produto(int id_produto) {
    Produto produto;
    long posicao = consultar_produto_por_indice(id_produto, &produto);
    if (posicao < 0) {
        printf("Produto ID %d nao encontrado.\n", id_produto);
        return;
    }
    produto.marca[0] = '*';
    FILE *arquivo = fopen(ARQ_PRODUTOS, "r+b");
    if (!arquivo) return;
    fseek(arquivo, posicao * sizeof(Produto), SEEK_SET);
    fwrite(&produto, sizeof(Produto), 1, arquivo);
    fclose(arquivo);
    printf("Produto ID %d removido na posicao #%ld.\n", id_produto, posicao);
    construir_indice_produtos();
}

int comparar_conversao(const void *a, const void *b) {
    EstatisticaProduto *p1 = (EstatisticaProduto *)a;
    EstatisticaProduto *p2 = (EstatisticaProduto *)b;
    if (p2->taxa_conversao > p1->taxa_conversao) return 1;
    if (p2->taxa_conversao < p1->taxa_conversao) return -1;
    return 0;
}

int comparar_abandono(const void *a, const void *b) {
    EstatisticaProduto *p1 = (EstatisticaProduto *)a;
    EstatisticaProduto *p2 = (EstatisticaProduto *)b;
    if (p2->taxa_abandono > p1->taxa_abandono) return 1;
    if (p2->taxa_abandono < p1->taxa_abandono) return -1;
    return 0;
}

int comparar_sem_carrinho(const void *a, const void *b) {
    EstatisticaProduto *p1 = (EstatisticaProduto *)a;
    EstatisticaProduto *p2 = (EstatisticaProduto *)b;
    if (p2->taxa_sem_carrinho > p1->taxa_sem_carrinho) return 1;
    if (p2->taxa_sem_carrinho < p1->taxa_sem_carrinho) return -1;
    return 0;
}

int comparar_por_categoria(const void *a, const void *b) {
    EstatisticaProduto *p1 = (EstatisticaProduto *)a;
    EstatisticaProduto *p2 = (EstatisticaProduto *)b;
    return strncmp(p1->categoria, p2->categoria, 30);
}

void analisar_maior_taxa_conversao() {
    FILE *arquivo_acessos = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_acessos) {
        printf("Erro ao abrir o arquivo acessos.dat.\n");
        return;
    }

    Acesso acesso;
    EstatisticaProduto top10[10];
    int qtd_top = 0;

    int id_atual = -1;
    int views_atual = 0;
    int compras_atual = 0;

    double soma_preco = 0, soma_conversao = 0, soma_preco_quadrado = 0, soma_conversao_quadrada = 0, soma_produto_cruzado = 0;
    int produtos_validos = 0;

    while (fread(&acesso, sizeof(Acesso), 1, arquivo_acessos) == 1) {
        if (acesso.id_produto != id_atual) {
            if (id_atual != -1 && views_atual > 10) {
                Produto produto;
                long pos = consultar_produto_por_indice(id_atual, &produto);
                if (pos >= 0) {
                    EstatisticaProduto estatistica_atual;
                    estatistica_atual.id_produto = id_atual;
                    estatistica_atual.visualizacoes = views_atual;
                    estatistica_atual.compras = compras_atual;
                    estatistica_atual.preco = produto.preco;
                    strncpy(estatistica_atual.categoria, produto.categoria, 30);
                    
                    estatistica_atual.taxa_conversao = ((float)compras_atual / views_atual) * 100.0f;

                    double p = estatistica_atual.preco;
                    double c = estatistica_atual.taxa_conversao;
                    soma_preco += p;
                    soma_conversao += c;
                    soma_preco_quadrado += p * p;
                    soma_conversao_quadrada += c * c;
                    soma_produto_cruzado += p * c;
                    produtos_validos++;

                    if (qtd_top < 10) {
                        top10[qtd_top] = estatistica_atual;
                        qtd_top++;
                        qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_conversao);
                    } else if (estatistica_atual.taxa_conversao > top10[9].taxa_conversao) {
                        top10[9] = estatistica_atual;
                        qsort(top10, 10, sizeof(EstatisticaProduto), comparar_conversao);
                    }
                }
            }
            
            id_atual = acesso.id_produto;
            views_atual = 0;
            compras_atual = 0;
        }

        if (strncmp(acesso.tipo_evento, "view", 4) == 0) {
            views_atual++;
        } else if (strncmp(acesso.tipo_evento, "purchase", 8) == 0) {
            compras_atual++;
        }
    }

    if (id_atual != -1 && views_atual > 10) {
         Produto produto;
         long pos = consultar_produto_por_indice(id_atual, &produto);
         if (pos >= 0) {
             EstatisticaProduto estatistica_atual;
             estatistica_atual.id_produto = id_atual;
             estatistica_atual.visualizacoes = views_atual;
             estatistica_atual.compras = compras_atual;
             estatistica_atual.preco = produto.preco;
             strncpy(estatistica_atual.categoria, produto.categoria, 30);
             
             estatistica_atual.taxa_conversao = ((float)compras_atual / views_atual) * 100.0f;

             double p = estatistica_atual.preco;
             double c = estatistica_atual.taxa_conversao;
             soma_preco += p;
             soma_conversao += c;
             soma_preco_quadrado += p * p;
             soma_conversao_quadrada += c * c;
             soma_produto_cruzado += p * c;
             produtos_validos++;

             if (qtd_top < 10) {
                 top10[qtd_top] = estatistica_atual;
                 qtd_top++;
                 qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_conversao);
             } else if (estatistica_atual.taxa_conversao > top10[9].taxa_conversao) {
                 top10[9] = estatistica_atual;
                 qsort(top10, 10, sizeof(EstatisticaProduto), comparar_conversao);
             }
         }
    }

    fclose(arquivo_acessos);

    printf("\n    TOP 10 PRODUTOS COM MAIOR TAXA DE CONVERSAO    \n");
    printf("%-10s %-25s %-10s %-10s %-10s %-10s\n", "ID", "Categoria", "Preco", "Views", "Compras", "Taxa (%)");
    
    for (int i = 0; i < qtd_top; i++) {
        printf("%-10d %-25.25s $%-9.2f %-10d %-10d %-10.2f%%\n",
               top10[i].id_produto,
               top10[i].categoria,
               top10[i].preco,
               top10[i].visualizacoes,
               top10[i].compras,
               top10[i].taxa_conversao);
    }

    if (produtos_validos > 1) {
        double numerador = soma_produto_cruzado - (soma_preco * soma_conversao / produtos_validos);
        double denominador = sqrt((soma_preco_quadrado - (soma_preco * soma_preco / produtos_validos)) *
                                  (soma_conversao_quadrada - (soma_conversao * soma_conversao / produtos_validos)));

        double correlacao_pearson = (denominador == 0) ? 0 : numerador / denominador;
        
        printf("\n    VALIDACAO DA HIPOTESE 1    \n");
        printf("Correlacao de Pearson (Preco x Conversao): %.2f\n", correlacao_pearson);
        if (correlacao_pearson < -0.2) {
            printf("Resultado: Hipotese confirmada! Produtos mais baratos tem maior conversao.\n");
        } else {
            printf("Resultado: Hipotese refutada! O preco isolado nao ditou a conversao no dataset.\n");
        }
    }
}

void analisar_abandono_carrinho() {
    FILE *arquivo_acessos = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_acessos) {
        printf("Erro ao abrir o arquivo acessos.dat.\n");
        return;
    }

    Acesso acesso;
    EstatisticaProduto top10[10];
    int qtd_top = 0;

    int id_atual = -1;
    int carrinhos_atual = 0;
    int compras_atual = 0;

    double soma_preco = 0, soma_abandono = 0, soma_preco_quadrado = 0, soma_abandono_quadrado = 0, soma_produto_cruzado = 0;
    int produtos_validos = 0;

    while (fread(&acesso, sizeof(Acesso), 1, arquivo_acessos) == 1) {
        if (acesso.id_produto != id_atual) {
            if (id_atual != -1 && carrinhos_atual > 5) {
                Produto produto;
                long pos = consultar_produto_por_indice(id_atual, &produto);
                if (pos >= 0) {
                    EstatisticaProduto estatistica_atual;
                    estatistica_atual.id_produto = id_atual;
                    estatistica_atual.carrinhos = carrinhos_atual;
                    estatistica_atual.compras = compras_atual;
                    estatistica_atual.preco = produto.preco;
                    strncpy(estatistica_atual.categoria, produto.categoria, 30);
                    
                    estatistica_atual.taxa_abandono = (1.0f - ((float)compras_atual / carrinhos_atual)) * 100.0f;
                    if (estatistica_atual.taxa_abandono < 0) estatistica_atual.taxa_abandono = 0;

                    double p = estatistica_atual.preco;
                    double a = estatistica_atual.taxa_abandono;
                    soma_preco += p;
                    soma_abandono += a;
                    soma_preco_quadrado += p * p;
                    soma_abandono_quadrado += a * a;
                    soma_produto_cruzado += p * a;
                    produtos_validos++;

                    if (qtd_top < 10) {
                        top10[qtd_top] = estatistica_atual;
                        qtd_top++;
                        qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_abandono);
                    } else if (estatistica_atual.taxa_abandono > top10[9].taxa_abandono) {
                        top10[9] = estatistica_atual;
                        qsort(top10, 10, sizeof(EstatisticaProduto), comparar_abandono);
                    }
                }
            }
            
            id_atual = acesso.id_produto;
            carrinhos_atual = 0;
            compras_atual = 0;
        }

        if (strncmp(acesso.tipo_evento, "cart", 4) == 0) {
            carrinhos_atual++;
        } else if (strncmp(acesso.tipo_evento, "purchase", 8) == 0) {
            compras_atual++;
        }
    }

    if (id_atual != -1 && carrinhos_atual > 5) {
         Produto produto;
         long pos = consultar_produto_por_indice(id_atual, &produto);
         if (pos >= 0) {
             EstatisticaProduto estatistica_atual;
             estatistica_atual.id_produto = id_atual;
             estatistica_atual.carrinhos = carrinhos_atual;
             estatistica_atual.compras = compras_atual;
             estatistica_atual.preco = produto.preco;
             strncpy(estatistica_atual.categoria, produto.categoria, 30);
             
             estatistica_atual.taxa_abandono = (1.0f - ((float)compras_atual / carrinhos_atual)) * 100.0f;
             if (estatistica_atual.taxa_abandono < 0) estatistica_atual.taxa_abandono = 0;

             double p = estatistica_atual.preco;
             double a = estatistica_atual.taxa_abandono;
             soma_preco += p;
             soma_abandono += a;
             soma_preco_quadrado += p * p;
             soma_abandono_quadrado += a * a;
             soma_produto_cruzado += p * a;
             produtos_validos++;

             if (qtd_top < 10) {
                 top10[qtd_top] = estatistica_atual;
                 qtd_top++;
                 qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_abandono);
             } else if (estatistica_atual.taxa_abandono > top10[9].taxa_abandono) {
                 top10[9] = estatistica_atual;
                 qsort(top10, 10, sizeof(EstatisticaProduto), comparar_abandono);
             }
         }
    }

    fclose(arquivo_acessos);

    printf("\n    TOP 10 PRODUTOS COM MAIOR TAXA DE ABANDONO DE CARRINHO    \n");
    printf("%-10s %-25s %-10s %-10s %-10s %-10s\n", "ID", "Categoria", "Preco", "Cart", "Compras", "Abandono (%)");
    
    for (int i = 0; i < qtd_top; i++) {
        printf("%-10d %-25.25s $%-9.2f %-10d %-10d %-10.2f%%\n",
               top10[i].id_produto,
               top10[i].categoria,
               top10[i].preco,
               top10[i].carrinhos,
               top10[i].compras,
               top10[i].taxa_abandono);
    }

    if (produtos_validos > 1) {
        double numerador = soma_produto_cruzado - (soma_preco * soma_abandono / produtos_validos);
        double denominador = sqrt((soma_preco_quadrado - (soma_preco * soma_preco / produtos_validos)) *
                                  (soma_abandono_quadrado - (soma_abandono * soma_abandono / produtos_validos)));

        double correlacao_pearson = (denominador == 0) ? 0 : numerador / denominador;
        
        printf("\n    VALIDACAO DA HIPOTESE 2    \n");
        printf("Correlacao de Pearson (Preco x Abandono): %.2f\n", correlacao_pearson);
        if (correlacao_pearson > 0.2) {
            printf("Resultado: Hipotese confirmada! Produtos de maior custo sofrem mais abandono.\n");
        } else {
            printf("Resultado: Hipotese refutada! Preco alto nao significa necessariamente abandono garantido.\n");
        }
    }
}

void analisar_vistos_sem_carrinho() {
    FILE *arquivo_acessos = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_acessos) {
        printf("Erro ao abrir o arquivo acessos.dat.\n");
        return;
    }

    Acesso acesso;
    EstatisticaProduto top10[10];
    int qtd_top = 0;

    int id_atual = -1;
    int views_atual = 0;
    int carrinhos_atual = 0;

    double soma_preco = 0, soma_sem_carrinho = 0, soma_preco_quadrado = 0, soma_sem_carrinho_quadrada = 0, soma_produto_cruzado = 0;
    int produtos_validos = 0;

    while (fread(&acesso, sizeof(Acesso), 1, arquivo_acessos) == 1) {
        if (acesso.id_produto != id_atual) {
            if (id_atual != -1 && views_atual > 10) {
                Produto produto;
                long pos = consultar_produto_por_indice(id_atual, &produto);
                if (pos >= 0) {
                    EstatisticaProduto estatistica_atual;
                    estatistica_atual.id_produto = id_atual;
                    estatistica_atual.visualizacoes = views_atual;
                    estatistica_atual.carrinhos = carrinhos_atual;
                    estatistica_atual.preco = produto.preco;
                    strncpy(estatistica_atual.categoria, produto.categoria, 30);
                    
                    estatistica_atual.taxa_sem_carrinho = (1.0f - ((float)carrinhos_atual / views_atual)) * 100.0f;
                    if (estatistica_atual.taxa_sem_carrinho < 0) estatistica_atual.taxa_sem_carrinho = 0;

                    double p = estatistica_atual.preco;
                    double s = estatistica_atual.taxa_sem_carrinho;
                    soma_preco += p;
                    soma_sem_carrinho += s;
                    soma_preco_quadrado += p * p;
                    soma_sem_carrinho_quadrada += s * s;
                    soma_produto_cruzado += p * s;
                    produtos_validos++;

                    if (qtd_top < 10) {
                        top10[qtd_top] = estatistica_atual;
                        qtd_top++;
                        qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_sem_carrinho);
                    } else if (estatistica_atual.taxa_sem_carrinho > top10[9].taxa_sem_carrinho) {
                        top10[9] = estatistica_atual;
                        qsort(top10, 10, sizeof(EstatisticaProduto), comparar_sem_carrinho);
                    }
                }
            }
            
            id_atual = acesso.id_produto;
            views_atual = 0;
            carrinhos_atual = 0;
        }

        if (strncmp(acesso.tipo_evento, "view", 4) == 0) {
            views_atual++;
        } else if (strncmp(acesso.tipo_evento, "cart", 4) == 0) {
            carrinhos_atual++;
        }
    }

    if (id_atual != -1 && views_atual > 10) {
         Produto produto;
         long pos = consultar_produto_por_indice(id_atual, &produto);
         if (pos >= 0) {
             EstatisticaProduto estatistica_atual;
             estatistica_atual.id_produto = id_atual;
             estatistica_atual.visualizacoes = views_atual;
             estatistica_atual.carrinhos = carrinhos_atual;
             estatistica_atual.preco = produto.preco;
             strncpy(estatistica_atual.categoria, produto.categoria, 30);
             
             estatistica_atual.taxa_sem_carrinho = (1.0f - ((float)carrinhos_atual / views_atual)) * 100.0f;
             if (estatistica_atual.taxa_sem_carrinho < 0) estatistica_atual.taxa_sem_carrinho = 0;

             double p = estatistica_atual.preco;
             double s = estatistica_atual.taxa_sem_carrinho;
             soma_preco += p;
             soma_sem_carrinho += s;
             soma_preco_quadrado += p * p;
             soma_sem_carrinho_quadrada += s * s;
             soma_produto_cruzado += p * s;
             produtos_validos++;

             if (qtd_top < 10) {
                 top10[qtd_top] = estatistica_atual;
                 qtd_top++;
                 qsort(top10, qtd_top, sizeof(EstatisticaProduto), comparar_sem_carrinho);
             } else if (estatistica_atual.taxa_sem_carrinho > top10[9].taxa_sem_carrinho) {
                 top10[9] = estatistica_atual;
                 qsort(top10, 10, sizeof(EstatisticaProduto), comparar_sem_carrinho);
             }
         }
    }

    fclose(arquivo_acessos);

    printf("\n    TOP 10 PRODUTOS COM MAIOR TAXA DE VISTOS NAO ADICIONADOS AO CARRINHO    \n");
    printf("%-10s %-25s %-10s %-10s %-10s %-10s\n", "ID", "Categoria", "Preco", "Views", "Cart", "Sem Cart(%)");
    
    for (int i = 0; i < qtd_top; i++) {
        printf("%-10d %-25.25s $%-9.2f %-10d %-10d %-10.2f%%\n",
               top10[i].id_produto,
               top10[i].categoria,
               top10[i].preco,
               top10[i].visualizacoes,
               top10[i].carrinhos,
               top10[i].taxa_sem_carrinho);
    }

    if (produtos_validos > 1) {
        double numerador = soma_produto_cruzado - (soma_preco * soma_sem_carrinho / produtos_validos);
        double denominador = sqrt((soma_preco_quadrado - (soma_preco * soma_preco / produtos_validos)) *
                                  (soma_sem_carrinho_quadrada - (soma_sem_carrinho * soma_sem_carrinho / produtos_validos)));

        double correlacao_pearson = (denominador == 0) ? 0 : numerador / denominador;
        
        printf("\n    VALIDACAO DA HIPOTESE 3    \n");
        printf("Correlacao de Pearson (Preco x Visto nao Adicionado): %.2f\n", correlacao_pearson);
        if (correlacao_pearson > 0.2) {
            printf("Resultado: Hipotese confirmada! Produtos mais caros tendem a ser apenas 'vitrine'.\n");
        } else {
            printf("Resultado: Hipotese refutada! O preco alto nao e o fator principal para a nao adicao ao carrinho.\n");
        }
    }
}

typedef struct {
    char nome[30];
    int max_compras;
    int id_mais_vendido;
    float preco_mais_vendido;
    float min_preco;
    char marca_dominante[30];
    int max_compras_marca_dominante;
    int total_compras_categoria;
} CategoriaStats;

void analisar_mais_vendidos_categoria() {
    FILE *arquivo_acessos = fopen(ARQ_ACESSOS, "rb");
    if (!arquivo_acessos) {
        printf("Erro ao abrir o arquivo acessos.dat.\n");
        return;
    }

    Acesso acesso;
    CategoriaStats cats[500];
    int total_cats = 0;

    int id_atual = -1;
    int compras_atual = 0;

    while (fread(&acesso, sizeof(Acesso), 1, arquivo_acessos) == 1) {
        if (acesso.id_produto != id_atual) {
            if (id_atual != -1 && compras_atual > 0) {
                Produto produto;
                long pos = consultar_produto_por_indice(id_atual, &produto);
                if (pos >= 0) {
                    int c_idx = -1;
                    for (int c = 0; c < total_cats; c++) {
                        if (strncmp(cats[c].nome, produto.categoria, 30) == 0) {
                            c_idx = c;
                            break;
                        }
                    }
                    
                    if (c_idx == -1 && total_cats < 500) {
                        c_idx = total_cats++;
                        strncpy(cats[c_idx].nome, produto.categoria, 30);
                        cats[c_idx].max_compras = 0;
                        cats[c_idx].min_preco = 999999.0f;
                        cats[c_idx].total_compras_categoria = 0;
                        cats[c_idx].max_compras_marca_dominante = 0;
                        strncpy(cats[c_idx].marca_dominante, "", 30);
                    }

                    if (c_idx != -1) {
                        cats[c_idx].total_compras_categoria += compras_atual;
                        
                        if (compras_atual > cats[c_idx].max_compras) {
                            cats[c_idx].max_compras = compras_atual;
                            cats[c_idx].id_mais_vendido = id_atual;
                            cats[c_idx].preco_mais_vendido = produto.preco;
                        }
                        if (produto.preco < cats[c_idx].min_preco) {
                            cats[c_idx].min_preco = produto.preco;
                        }
                        
                        if (compras_atual > cats[c_idx].max_compras_marca_dominante) {
                            cats[c_idx].max_compras_marca_dominante = compras_atual;
                            strncpy(cats[c_idx].marca_dominante, produto.marca, 30);
                        }
                    }
                }
            }
            id_atual = acesso.id_produto;
            compras_atual = 0;
        }

        if (strncmp(acesso.tipo_evento, "purchase", 8) == 0) {
            compras_atual++;
        }
    }

    if (id_atual != -1 && compras_atual > 0) {
         Produto produto;
         long pos = consultar_produto_por_indice(id_atual, &produto);
         if (pos >= 0) {
             int c_idx = -1;
             for (int c = 0; c < total_cats; c++) {
                 if (strncmp(cats[c].nome, produto.categoria, 30) == 0) { c_idx = c; break; }
             }
             if (c_idx == -1 && total_cats < 500) {
                 c_idx = total_cats++;
                 strncpy(cats[c_idx].nome, produto.categoria, 30);
                 cats[c_idx].max_compras = 0;
                 cats[c_idx].min_preco = 999999.0f;
                 cats[c_idx].total_compras_categoria = 0;
                 cats[c_idx].max_compras_marca_dominante = 0;
                 strncpy(cats[c_idx].marca_dominante, "", 30);
             }
             if (c_idx != -1) {
                 cats[c_idx].total_compras_categoria += compras_atual;
                 if (compras_atual > cats[c_idx].max_compras) {
                     cats[c_idx].max_compras = compras_atual;
                     cats[c_idx].preco_mais_vendido = produto.preco;
                 }
                 if (produto.preco < cats[c_idx].min_preco) cats[c_idx].min_preco = produto.preco;
                 
                 if (compras_atual > cats[c_idx].max_compras_marca_dominante) {
                     cats[c_idx].max_compras_marca_dominante = compras_atual;
                     strncpy(cats[c_idx].marca_dominante, produto.marca, 30);
                 }
             }
         }
    }
    fclose(arquivo_acessos);

    printf("\n    ANALISE DE LIDERANCA DE VENDAS E PRECO POR CATEGORIA    \n");
    printf("%-25s %-10s %-10s %-12s %-15s %-10s\n", "Categoria", "Lider Vendas", "Mais Barato", "Lider e Min?", "Marca Dominante", "Share (%)");

    int lideres_nao_sao_mais_baratos = 0;
    double soma_share_marcas = 0;
    int categorias_exibidas = 0;

    for (int i = 0; i < total_cats; i++) {
        if (cats[i].nome[0] == ' ' || cats[i].nome[0] == '\0') continue;

        int eh_mais_barato = (fabs(cats[i].preco_mais_vendido - cats[i].min_preco) < 0.01f);
        if (!eh_mais_barato) lideres_nao_sao_mais_baratos++;

        float share_marca = cats[i].total_compras_categoria > 0 ? ((float)cats[i].max_compras_marca_dominante / cats[i].total_compras_categoria) * 100.0f : 0.0f;
        soma_share_marcas += share_marca;

        if (categorias_exibidas < 10) {
            printf("%-25.25s $%-9.2f $%-9.2f %-12s %-15.15s %-10.2f%%\n",
                   cats[i].nome,
                   cats[i].preco_mais_vendido,
                   cats[i].min_preco,
                   eh_mais_barato ? "Sim" : "Nao",
                   cats[i].marca_dominante,
                   share_marca);
            categorias_exibidas++;
        }
    }

    float pct_lideres_nao_baratos = total_cats > 0 ? ((float)lideres_nao_sao_mais_baratos / total_cats) * 100.0f : 0.0f;
    float media_share_marca = total_cats > 0 ? (float)(soma_share_marcas / total_cats) : 0.0f;

    printf("\n    VALIDACAO DA HIPOTESE 4    \n");
    printf("Categorias Analisadas: %d\n", total_cats);
    printf("Categorias onde o lider NAO e o mais barato: %.2f%%\n", pct_lideres_nao_baratos);
    printf("Dominancia Media da Marca Lider por Categoria: %.2f%%\n", media_share_marca);

    if (pct_lideres_nao_baratos > 50.0f && media_share_marca > 30.0f) {
        printf("Resultado: Hipotese confirmada! Produtos mais baratos nao dominam as vendas e ha forte concentracao de marcas lideres.\n");
    } else {
        printf("Resultado: Hipotese refutada! Produtos mais baratos dominam ou as marcas sao muito pulverizadas.\n");
    }
}

int main() {
    carregar_ou_criar_indices();
    int opcao = -1;

    while (opcao != 0) {
        printf("\n   ---    MENU   ---    \n");
        printf("1. Mostrar produtos.dat (parcial)\n");
        printf("2. Mostrar acessos.dat (parcial)\n");
        printf("3. Pesquisa Binaria Direta nos ficheiros .dat\n");
        printf("4. Consulta via Indice Parcial (.indice + seek)\n");
        printf("5. Inserir Produto\n");
        printf("6. Remover Produto\n");
        printf("7. Analisar Maior Taxa de Conversao (Pergunta 1)\n");
        printf("8. Analisar Abandono de Carrinho (Pergunta 2)\n");
        printf("9. Analisar Vistos sem Carrinho (Pergunta 3)\n");
        printf("10. Analisar Mais Vendidos x Preco e Marca (Pergunta 4)\n");
        printf("0. Sair e gravar indices\n");
        printf("Opcao: ");
        if (scanf("%d", &opcao) != 1) break;

        if (opcao == 1) {
            long inicio, quantidade;
            printf("Posicao inicial e quantidade: ");
            scanf("%ld %ld", &inicio, &quantidade);
            mostrar_produtos(inicio, quantidade);
        } else if (opcao == 2) {
            long inicio, quantidade;
            printf("Posicao inicial e quantidade: ");
            scanf("%ld %ld", &inicio, &quantidade);
            mostrar_acessos(inicio, quantidade);
        } else if (opcao == 3) {
            int tipo_arquivo, id_produto;
            printf("Escolha o arquivo (1=Produtos, 2=Acessos) e ID do Produto: ");
            scanf("%d %d", &tipo_arquivo, &id_produto);
            if (tipo_arquivo == 1) {
                Produto produto;
                long posicao = pesquisa_binaria_direta_produtos(id_produto, &produto);
                if (posicao >= 0) imprimir_produto(&produto, posicao);
                else printf("Produto nao encontrado.\n");
            } else {
                Acesso acesso;
                long posicao = pesquisa_binaria_direta_acessos(id_produto, &acesso);
                if (posicao >= 0) imprimir_acesso(&acesso, posicao);
                else printf("Acesso nao encontrado.\n");
            }
        } else if (opcao == 4) {
            int tipo_arquivo, id_produto;
            printf("Escolha o arquivo (1=Produtos, 2=Acessos) e ID do Produto: ");
            scanf("%d %d", &tipo_arquivo, &id_produto);
            if (tipo_arquivo == 1) {
                Produto produto;
                long posicao = consultar_produto_por_indice(id_produto, &produto);
                if (posicao >= 0) imprimir_produto(&produto, posicao);
                else printf("Produto nao encontrado.\n");
            } else {
                Acesso acesso;
                long posicao = consultar_acesso_por_indice(id_produto, &acesso);
                if (posicao >= 0) imprimir_acesso(&acesso, posicao);
                else printf("Acesso nao encontrado.\n");
            }
        } else if (opcao == 5) {
            int id_produto; char marca[50], categoria[50]; float preco;
            printf("ID, Marca, Categoria e Preco: ");
            scanf("%d %49s %49s %f", &id_produto, marca, categoria, &preco);
            inserir_produto_ordenado(id_produto, marca, categoria, preco);
        } else if (opcao == 6) {
            int id_produto;
            printf("ID do produto a remover: ");
            scanf("%d", &id_produto);
            remover_produto(id_produto);
        } else if (opcao == 7) {
            analisar_maior_taxa_conversao();
        } else if (opcao == 8) {
            analisar_abandono_carrinho();
        } else if (opcao == 9) {
            analisar_vistos_sem_carrinho();
        } else if (opcao == 10) {
            analisar_mais_vendidos_categoria();
        }
    }
    construir_indice_produtos();
    construir_indice_acessos();
    return 0;
}
