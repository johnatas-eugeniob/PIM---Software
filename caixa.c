#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include "libs/cJSON.h"
#include "caixa.h"
#include "main.h"
#include <time.h> //também servirá para gerar ids unicos a cada extrato gerado

// Função para obter a data atual como string no formato "YYYY-MM-DD"
void obter_data_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, tamanho, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}
//================================================================================================
void finalizar_etiquetagem(cJSON *etiquetas_root, cJSON *root, float total) {
    int choice_pagamento;
    float valor_pago, troco;
    char forma_pagamento[20];

    printf("Etiquetagem finalizada! Total: %.2f\n", total);

    // Solicita a forma de pagamento
    printf("| Informe o método de pagamento:          |\n");
    printf("| 1 - Crédito/Débito                      |\n");
    printf("| 2 - PIX                                 |\n");
    printf("| 3 - Dinheiro                            |\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &choice_pagamento);

    // Determina o método de pagamento
    switch (choice_pagamento) {
        case 1:
            strcpy(forma_pagamento, "Crédito/Débito");
            break;
        case 2:
            strcpy(forma_pagamento, "PIX");
            break;
        case 3:
            strcpy(forma_pagamento, "Dinheiro");
            printf("| Valor pago: ");
            scanf("%f", &valor_pago);
            if (valor_pago < total) {
                printf("Valor insuficiente! Tente novamente.\n");
                return;
            }
            troco = valor_pago - total;
            break;
        default:
            printf("Opção inválida! Tente novamente.\n");
            return;
    }

    if (choice_pagamento != 3) {
        valor_pago = total;
        troco = 0;
    }

    printf("Troco: %.2f\n", troco);

    // Atualizar e salvar arquivo de estoque
    FILE *file = fopen("bd/estoque_list.json", "w");
    if (file) {
        char *updated_json_string = cJSON_Print(root);
        fputs(updated_json_string, file);
        fclose(file);
        free(updated_json_string);
    } else {
        fprintf(stderr, "Erro ao salvar o arquivo JSON atualizado de estoque.\n");
    }

    // Atualizar e salvar arquivo de etiquetas
    file = fopen("bd/etiqueta.json", "w");
    if (file) {
        char *updated_json_string = cJSON_Print(etiquetas_root);
        fputs(updated_json_string, file);
        fclose(file);
        free(updated_json_string);
    } else {
        fprintf(stderr, "Erro ao salvar o arquivo JSON atualizado de etiquetas.\n");
    }

    // Carregar o arquivo de extrato existente
    file = fopen("bd/extrato.json", "r");
    cJSON *extrato = cJSON_CreateArray();
    if (file) {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *data = malloc(length + 1);
        fread(data, 1, length, file);
        data[length] = '\0';
        fclose(file);

        cJSON *extrato_existente = cJSON_Parse(data);
        free(data);
        if (extrato_existente) {
            extrato = extrato_existente;
        }
    } else {
        fprintf(stderr, "Erro ao abrir o arquivo JSON do extrato. Criando novo.\n");
    }

    // Adicionar transação ao extrato
    cJSON *transacao = cJSON_CreateObject();
    cJSON_AddItemToObject(transacao, "total", cJSON_CreateNumber(total));
    cJSON_AddItemToObject(transacao, "metodo_pagamento", cJSON_CreateString(forma_pagamento));
    cJSON_AddItemToObject(transacao, "valor_pago", cJSON_CreateNumber(valor_pago));
    cJSON_AddItemToObject(transacao, "troco", cJSON_CreateNumber(troco));

    // Adicionar IDs dos produtos vendidos ao extrato
    cJSON *produtos_vendidos = cJSON_CreateArray();
    cJSON *produto = NULL;
    cJSON_ArrayForEach(produto, cJSON_GetObjectItemCaseSensitive(root, "produtos")) {
        cJSON *saidas = cJSON_GetObjectItemCaseSensitive(produto, "saidas");
        if (cJSON_IsArray(saidas)) {
            cJSON *saida = NULL;
            cJSON_ArrayForEach(saida, saidas) {
                if (cJSON_GetObjectItemCaseSensitive(saida, "id_etiqueta")) {
                    cJSON *id_produto = cJSON_GetObjectItemCaseSensitive(produto, "id");
                    cJSON_AddItemToArray(produtos_vendidos, cJSON_CreateNumber(id_produto->valueint));
                }
            }
        }
    }
    cJSON_AddItemToObject(transacao, "produto_vendido", produtos_vendidos);

    cJSON_AddItemToArray(extrato, transacao);

    // Salvar o extrato atualizado
    file = fopen("bd/extrato.json", "w");
    if (file) {
        char *updated_json_string = cJSON_Print(extrato);
        fputs(updated_json_string, file);
        fclose(file);
        free(updated_json_string);
    } else {
        fprintf(stderr, "Erro ao salvar o arquivo JSON do extrato.\n");
    }

    cJSON_Delete(etiquetas_root);
}

void prod_etiquetado(cJSON *root) {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");

    FILE *file;
    char *data;
    long length;
    int choice;
    float total = 0;

    // Carregar o arquivo de etiquetas
    file = fopen("bd/etiqueta.json", "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de etiquetas!\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *etiquetas_root = cJSON_Parse(data);
    free(data);
    if (!etiquetas_root) {
        fprintf(stderr, "Erro ao analisar JSON de etiquetas: %s", cJSON_GetErrorPtr());
        return;
    }

    cJSON *etiquetas = cJSON_GetObjectItemCaseSensitive(etiquetas_root, "etiquetas");
    if (!cJSON_IsArray(etiquetas)) {
        fprintf(stderr, "Formato de JSON de etiquetas inválido\n");
        cJSON_Delete(etiquetas_root);
        return;
    }

    int etiqueta_id;
    printf("Digite o ID da etiqueta: ");
    scanf("%d", &etiqueta_id);
    getchar();

    // Buscar e atualizar produtos com dados das etiquetas
    cJSON *etiqueta = NULL;
    cJSON_ArrayForEach(etiqueta, etiquetas) {
        cJSON *id_etiqueta = cJSON_GetObjectItemCaseSensitive(etiqueta, "id_etiqueta");
        if (cJSON_IsNumber(id_etiqueta) && id_etiqueta->valueint == etiqueta_id) {
            cJSON *id_produto = cJSON_GetObjectItemCaseSensitive(etiqueta, "id_produto");
            if (cJSON_IsNumber(id_produto)) {
                cJSON *produto = NULL;
                cJSON_ArrayForEach(produto, cJSON_GetObjectItemCaseSensitive(root, "produtos")) {
                    cJSON *prod_id = cJSON_GetObjectItemCaseSensitive(produto, "id");
                    if (cJSON_IsNumber(prod_id) && prod_id->valueint == id_produto->valueint) {
                        cJSON *saidas = cJSON_GetObjectItemCaseSensitive(produto, "saidas");
                        if (!cJSON_IsArray(saidas)) {
                            saidas = cJSON_AddArrayToObject(produto, "saidas");
                        }
                        cJSON *saida = cJSON_CreateObject();
                        cJSON_AddItemToObject(saida, "id_etiqueta", cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(etiqueta, "id_etiqueta"), 1));
                        cJSON_AddItemToObject(saida, "produto", cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(etiqueta, "produto"), 1));
                        cJSON_AddItemToObject(saida, "peso", cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(etiqueta, "peso"), 1));
                        cJSON_AddItemToObject(saida, "preco_calculado", cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(etiqueta, "preco_calculado"), 1));
                        cJSON_AddItemToObject(saida, "data", cJSON_Duplicate(cJSON_GetObjectItemCaseSensitive(etiqueta, "data"), 1));
                        cJSON_AddItemToArray(saidas, saida);

                        // Atualizar quantidade disponível
                        cJSON *quantidade_disponivel = cJSON_GetObjectItemCaseSensitive(produto, "quantidade_disponivel");
                        if (cJSON_IsNumber(quantidade_disponivel)) {
                            quantidade_disponivel->valueint -= cJSON_GetObjectItemCaseSensitive(etiqueta, "peso")->valueint;
                        }

                        // Atualizar resumo_saidas
                        cJSON *resumo_saidas = cJSON_GetObjectItemCaseSensitive(produto, "resumo_saidas");
                        if (cJSON_IsObject(resumo_saidas)) {
                            cJSON *quantidade_total_vendida = cJSON_GetObjectItemCaseSensitive(resumo_saidas, "quantidade_total_vendida");
                            cJSON *valor_total_vendas = cJSON_GetObjectItemCaseSensitive(resumo_saidas, "valor_total_vendas");
                            if (cJSON_IsNumber(quantidade_total_vendida) && cJSON_IsNumber(valor_total_vendas)) {
                                quantidade_total_vendida->valueint += cJSON_GetObjectItemCaseSensitive(etiqueta, "peso")->valueint;
                                valor_total_vendas->valuedouble += cJSON_GetObjectItemCaseSensitive(etiqueta, "preco_calculado")->valuedouble;
                            }
                        }

                        // Atualizar resumo_geral
                        cJSON *resumo_geral = cJSON_GetObjectItemCaseSensitive(root, "resumo_geral");
                        if (cJSON_IsObject(resumo_geral)) {
                            cJSON *quantidade_total_vendida = cJSON_GetObjectItemCaseSensitive(resumo_geral, "quantidade_total_vendida");
                            cJSON *valor_total_vendas = cJSON_GetObjectItemCaseSensitive(resumo_geral, "valor_total_vendas");
                            if (cJSON_IsNumber(quantidade_total_vendida) && cJSON_IsNumber(valor_total_vendas)) {
                                quantidade_total_vendida->valueint += cJSON_GetObjectItemCaseSensitive(etiqueta, "peso")->valueint;
                                valor_total_vendas->valuedouble += cJSON_GetObjectItemCaseSensitive(etiqueta, "preco_calculado")->valuedouble;
                            }
                        }

                        total += cJSON_GetObjectItemCaseSensitive(etiqueta, "preco_calculado")->valuedouble;
                        break;
                    }
                }
            }
            break;
        }
    }

    // Finalizar etiquetagem
    finalizar_etiquetagem(etiquetas_root, root, total);

    cJSON_Delete(etiquetas_root);
}
//================================================================================================
// Função para finalizar a compra e salvar o extrato
void finalizar_compra(cJSON *extrato_root, float total, cJSON *root) {
    setlocale(LC_NUMERIC, "C");
    char forma_pagamento[20];
    float troco = 0, pagamento = 0;
    int choice_pagamento, gerar_via;
    char id_extrato[50];  // Para armazenar o ID do extrato único

    // Exibe o resumo do total
    printf("|-----------------------------------------|\n");
    printf("| Total: %.2f                             |\n", total);
    printf("|-----------------------------------------|\n");

    // Solicita a forma de pagamento
    printf("| Informe o método de pagamento:          |\n");
    printf("| 1 - Crédito/Débito                      |\n");
    printf("| 2 - PIX                                 |\n");
    printf("| 3 - Dinheiro                            |\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &choice_pagamento);

    // Determina o método de pagamento
    switch (choice_pagamento) {
        case 1:
            strcpy(forma_pagamento, "Crédito/Débito");
            break;
        case 2:
            strcpy(forma_pagamento, "PIX");
            break;
        case 3:
            strcpy(forma_pagamento, "Dinheiro");
            printf("| Valor pago: ");
            scanf("%f", &pagamento);
            if (pagamento < total) {
                printf("Valor insuficiente! Tente novamente.\n");
                return;
            }
            troco = pagamento - total;
            break;
        default:
            printf("Opção inválida! Tente novamente.\n");
            return;
    }

    // Exibe o troco, se aplicável
    if (troco > 0) {
        printf("| Troco: %.2f\n", troco);
    }else{
        pagamento = total;
    }

    // Cria o objeto principal para o extrato desta compra
    cJSON *extrato_atual = cJSON_CreateObject();
    cJSON_AddItemToObject(extrato_atual, "itens", extrato_root);

    // Adiciona detalhes de pagamento ao extrato atual
    cJSON *pagamento_info = cJSON_CreateObject();
    cJSON_AddItemToObject(pagamento_info, "forma_pagamento", cJSON_CreateString(forma_pagamento));
    cJSON_AddItemToObject(pagamento_info, "valor_pago", cJSON_CreateNumber(pagamento));
    cJSON_AddItemToObject(pagamento_info, "troco", cJSON_CreateNumber(troco));
    cJSON_AddItemToObject(pagamento_info, "total", cJSON_CreateNumber(total));
    cJSON_AddItemToObject(extrato_atual, "pagamento", pagamento_info);

    // Gera um ID único para o extrato baseado no timestamp
    time_t now = time(NULL);
    strftime(id_extrato, sizeof(id_extrato), "%Y%m%d%H%M%S", localtime(&now));

    // Pergunta ao usuário se deseja gerar uma via própria
    printf("|-----------------------------------------|\n");
    printf("| Deseja gerar uma via própria?           |\n");
    printf("| 1 - Sim                                 |\n");
    printf("| 2 - Não                                 |\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &gerar_via);

    if (gerar_via == 1) {
        char nome_arquivo[100];
        sprintf(nome_arquivo, "extratos/extrato_%s.json", id_extrato);  // Nome único do arquivo

        char *extrato_cliente_data = cJSON_Print(extrato_atual);
        FILE *extrato_cliente_file = fopen(nome_arquivo, "w");
        if (extrato_cliente_file) {
            fwrite(extrato_cliente_data, 1, strlen(extrato_cliente_data), extrato_cliente_file);
            fclose(extrato_cliente_file);
            printf("| Via própria salva como: %s\n", nome_arquivo);
        } else {
            fprintf(stderr, "Erro ao salvar a via própria!\n");
        }
        free(extrato_cliente_data);
    }

    // Atualiza o extrato da loja incrementalmente
    FILE *extrato_loja_file = fopen("bd/extrato.json", "r+");
    cJSON *extrato_loja = NULL;
    if (extrato_loja_file) {
        fseek(extrato_loja_file, 0, SEEK_END);
        long file_size = ftell(extrato_loja_file);
        rewind(extrato_loja_file);
        char *loja_data = malloc(file_size + 1);
        fread(loja_data, 1, file_size, extrato_loja_file);
        loja_data[file_size] = '\0';
        fclose(extrato_loja_file);

        extrato_loja = cJSON_Parse(loja_data);
        free(loja_data);
    }

    if (!extrato_loja) {
        extrato_loja = cJSON_CreateArray();  // Se o arquivo não existir ou estiver vazio, cria um novo
    }

    cJSON_AddItemToArray(extrato_loja, extrato_atual);

    char *extrato_loja_data = cJSON_Print(extrato_loja);
    extrato_loja_file = fopen("bd/extrato.json", "w");
    if (extrato_loja_file) {
        fwrite(extrato_loja_data, 1, strlen(extrato_loja_data), extrato_loja_file);
        fclose(extrato_loja_file);
        printf("| Extrato da loja atualizado com sucesso!\n");
    } else {
        fprintf(stderr, "Erro ao atualizar o extrato da loja!\n");
    }
    free(extrato_loja_data);
    cJSON_Delete(extrato_loja);

    // Salva o estado atualizado do estoque no arquivo "estoque_list.json"
    char *estoque_data = cJSON_Print(root);
    FILE *estoque_file = fopen("bd/estoque_list.json", "w");
    if (estoque_file) {
        fwrite(estoque_data, 1, strlen(estoque_data), estoque_file);
        fclose(estoque_file);
        printf("| Estoque atualizado com sucesso!\n");
    } else {
        fprintf(stderr, "Erro ao salvar o estoque no arquivo!\n");
    }
    free(estoque_data);

    printf("| Compra finalizada com sucesso!\n");
    printf("|-----------------------------------------|\n");
    system("pause");
}
//================================================================================================
void fluxo_caixa() {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");

    char unidade[10], data_saida[20];
    float qtd, preco_vendido, total = 0;
    int id, choice;
    char input[100];  // Buffer para capturar a entrada do usuário

    FILE *file = fopen("bd/estoque_list.json", "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        fprintf(stderr, "Erro ao analisar JSON: %s", cJSON_GetErrorPtr());
        return;
    }

    cJSON *produtos = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *extrato_root = cJSON_CreateArray();  // Criar array para registrar extrato

    while (1) {
        printf(" ____________________________\n");
        printf("|      Registrar Venda       |\n");
        printf("|----------------------------|\n");
        printf("| Inserir informações - 1    |\n");
        printf("| Produto entiquetado - 2    |\n");
        printf("| Voltar - 3                 |\n");
        printf("|----------------------------|\n");
        printf("| Escolha uma das opções: ");
        scanf("%d", &choice);
        printf("\n\n");

        if (choice == 1) {
            system("cls || clear");
            while (1) {
                // Solicitar o ID do produto
                printf("| Insira o ID do produto: ");
                scanf("%d", &id);
                getchar();

                bool prod_found = false;
                cJSON *prod = NULL;
                cJSON_ArrayForEach(prod, produtos) {
                    cJSON *json_id = cJSON_GetObjectItemCaseSensitive(prod, "id");
                    if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
                        prod_found = true;
                        break;
                    }
                }

                if (prod_found) {
                    cJSON *json_preco_vendido = cJSON_GetObjectItemCaseSensitive(prod, "preco_vendido");
                    if (cJSON_IsNumber(json_preco_vendido)) {
                        preco_vendido = json_preco_vendido->valuedouble;
                    } else {
                        printf("Preço não encontrado para o produto.\n");
                        continue;
                    }

                    // Solicitar quantidade
                    printf("| Informe a quantidade comprada: ");
                    fgets(input, sizeof(input), stdin);  // Usar fgets para capturar como string
                    sscanf(input, "%f", &qtd);            // Converter para float

                    // Obter a data atual
                    obter_data_atual(data_saida, sizeof(data_saida));

                    // Calcular o total e exibir
                    float total_item = qtd * preco_vendido;
                    total += total_item;
                    printf("| O subtotal é: %.2f\n", total_item);
                    printf("-----------------------------------------\n");

                    // Atualizar a quantidade disponível subtraindo a quantidade vendida
                    cJSON *quantidade_disponivel = cJSON_GetObjectItemCaseSensitive(prod, "quantidade_disponivel");
                    if (cJSON_IsNumber(quantidade_disponivel)) {
                        int nova_quantidade = quantidade_disponivel->valueint - (int)qtd;
                        cJSON_SetNumberValue(quantidade_disponivel, nova_quantidade);
                    }

                    // Adicionar a entrada ao histórico de saídas
                    cJSON *saidas = cJSON_GetObjectItemCaseSensitive(prod, "saidas");
                    if (!cJSON_IsArray(saidas)) {
                        saidas = cJSON_AddArrayToObject(prod, "saidas");
                    }

                    cJSON *new_saida = cJSON_CreateObject();
                    cJSON_AddItemToObject(new_saida, "id", cJSON_CreateNumber(id));
                    cJSON_AddItemToObject(new_saida, "data", cJSON_CreateString(data_saida));
                    cJSON_AddItemToObject(new_saida, "quantidade", cJSON_CreateNumber(qtd));
                    cJSON_AddItemToObject(new_saida, "motivo", cJSON_CreateString("venda"));
                    cJSON_AddItemToObject(new_saida, "total", cJSON_CreateNumber(total_item));
                    cJSON_AddItemToArray(saidas, new_saida);

                    // Atualizar resumo_saidas do produto
                    cJSON *resumo_saidas = cJSON_GetObjectItemCaseSensitive(prod, "resumo_saidas");
                    if (resumo_saidas) {
                        cJSON *quantidade_total_vendida = cJSON_GetObjectItemCaseSensitive(resumo_saidas, "quantidade_total_vendida");
                        if (cJSON_IsNumber(quantidade_total_vendida)) {
                            quantidade_total_vendida->valueint += (int)qtd;
                        }
                        cJSON *quantidade_total_kg_vendida = cJSON_GetObjectItemCaseSensitive(resumo_saidas, "quantidade_total_kg_vendida");
                        if (cJSON_IsNumber(quantidade_total_kg_vendida) && strcmp(unidade, "kg") == 0) {
                            quantidade_total_kg_vendida->valuedouble += qtd;
                        }
                        cJSON *valor_total_vendas = cJSON_GetObjectItemCaseSensitive(resumo_saidas, "valor_total_vendas");
                        if (cJSON_IsNumber(valor_total_vendas)) {
                            valor_total_vendas->valuedouble += total_item;
                        }
                    }

                    // Adicionar produto ao extrato
                    cJSON *extrato_item = cJSON_CreateObject();
                    cJSON_AddItemToObject(extrato_item, "id", cJSON_CreateNumber(id));
                    cJSON_AddItemToObject(extrato_item, "quantidade", cJSON_CreateNumber(qtd));
                    cJSON_AddItemToObject(extrato_item, "total", cJSON_CreateNumber(total_item));
                    cJSON_AddItemToArray(extrato_root, extrato_item);

                    // Opção para finalizar a compra a partir do segundo while
                    printf("\n| Finalizar compra   - 2 |\n");
                    printf("| Continuar - 1          |\n");
                    printf("|------------------------|\n");
                    printf("| Escolha uma das opções: ");
                    scanf("%d", &choice);
                    if (choice == 2) {
                        // Atualizar resumo_geral
                        cJSON *resumo_geral = cJSON_GetObjectItemCaseSensitive(root, "resumo_geral");
                        if (resumo_geral) {
                            cJSON *quantidade_total_vendida = cJSON_GetObjectItemCaseSensitive(resumo_geral, "quantidade_total_vendida");
                            if (cJSON_IsNumber(quantidade_total_vendida)) {
                                quantidade_total_vendida->valueint += (int)qtd;
                            }
                            cJSON *valor_total_vendas = cJSON_GetObjectItemCaseSensitive(resumo_geral, "valor_total_vendas");
                            if (cJSON_IsNumber(valor_total_vendas)) {
                                valor_total_vendas->valuedouble += total_item;
                            }
                        }
                        finalizar_compra(extrato_root, total, root);
                        break;
                    } else if (choice == 1) {
                        continue;
                    } else {
                        printf("Opção inválida!\n");
                    }
                } else {
                    printf("Produto com ID %d não encontrado. Tente novamente!\n\n", id);
                }
            }
        } else if (choice == 2) {
            // Função para lidar com produtos etiquetados
            prod_etiquetado(root);
        } else if (choice == 3) {
            main();
        } else {
            printf("Opção inválida! Tente novamente.\n\n");
        }
    }
    cJSON_Delete(root);
}
