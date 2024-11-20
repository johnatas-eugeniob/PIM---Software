#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include "libs/cJSON.h"
#include "pesagem.h"
#include <time.h>

// Função para obter a data atual como string no formato "YYYY-MM-DD"
/*void obter_data_atual(char *buffer, size_t tamanho) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, tamanho, "%02d-%02d-%d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
}*/

void pesagem(){
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");

    char unidade[10], data_etiqueta[20];
    float qtd, preco_vendido, total = 0;
    int id, option;
    char input[100];  // Buffer para capturar a entrada do usuário

    // Carregar o arquivo de produtos
    FILE *file = fopen("bd/estoque_list.json", "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de produtos!\n");
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
        fprintf(stderr, "Erro ao analisar JSON de produtos: %s", cJSON_GetErrorPtr());
        return;
    }

    cJSON *produtos = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON de produtos inválido\n");
        cJSON_Delete(root);
        return;
    }

    // Carregar o arquivo de etiquetas
    file = fopen("bd/etiqueta.json", "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de etiquetas!\n");
        cJSON_Delete(root);
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
        cJSON_Delete(root);
        return;
    }

    cJSON *etiquetas = cJSON_GetObjectItemCaseSensitive(etiquetas_root, "etiquetas");
    if (!cJSON_IsArray(etiquetas)) {
        etiquetas = cJSON_AddArrayToObject(etiquetas_root, "etiquetas");
    }

    printf(" -------------------------\n");
    printf("| Pesagem de produto      |\n");
    printf("|-------------------------|\n");
    printf("| Inserir informações - 1 |\n");
    printf("| Voltar - 2              |\n");
    printf("|-------------------------|\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &option);
    getchar();  // Consumir o newline deixado por scanf
    printf("\n\n");

    switch (option) {
        case 1:
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
                    // Solicitar quantidade
                    printf("| Informe o peso em kg: ");
                    fgets(input, sizeof(input), stdin);  // Usar fgets para capturar como string
                    sscanf(input, "%f", &qtd);

                    // Obter a data atual
                    obter_data_atual(data_etiqueta, sizeof(data_etiqueta));

                    // Calcular o total com base no preco por quilo do produto
                    cJSON *preco_kg = cJSON_GetObjectItemCaseSensitive(prod, "preco_kg");
                    if (!cJSON_IsNumber(preco_kg)) {
                        printf("Preço de venda não encontrado para o produto.\n");
                        continue;
                    }
                    float total = qtd * preco_kg->valuedouble;
                    printf("| O total calculado é: %.2f\n", total);
                    printf("\n----------------------------------------\n");

                    // Gerar um novo ID para a etiqueta
                    int new_etiqueta_id = 1;
                    cJSON *last_etiqueta = cJSON_GetArrayItem(etiquetas, cJSON_GetArraySize(etiquetas) - 1);
                    if (last_etiqueta) {
                        cJSON *json_etiqueta_id = cJSON_GetObjectItemCaseSensitive(last_etiqueta, "id_etiqueta");
                        if (cJSON_IsNumber(json_etiqueta_id)) {
                            new_etiqueta_id = json_etiqueta_id->valueint + 1;
                        }
                    }

                    // Criar a nova etiqueta e adicionar ao array de etiquetas
                    cJSON *nova_etiqueta = cJSON_CreateObject();
                    cJSON_AddNumberToObject(nova_etiqueta, "id_etiqueta", new_etiqueta_id);
                    cJSON_AddNumberToObject(nova_etiqueta, "id_produto", id);
                    cJSON_AddStringToObject(nova_etiqueta, "produto", cJSON_GetObjectItemCaseSensitive(prod, "nome")->valuestring);
                    cJSON_AddNumberToObject(nova_etiqueta, "peso", qtd);
                    cJSON_AddNumberToObject(nova_etiqueta, "preco_calculado", total);
                    cJSON_AddStringToObject(nova_etiqueta, "data", data_etiqueta);

                    cJSON_AddItemToArray(etiquetas, nova_etiqueta);

                    // Salvar atualizações no arquivo etiquetas.json
                    data = cJSON_Print(etiquetas_root);
                    file = fopen("bd/etiqueta.json", "w");
                    if (file) {
                        fwrite(data, 1, strlen(data), file);
                        fclose(file);
                        printf("| Etiqueta registrada com sucesso!\n");
                        printf("|----------------------------------------\n\n");
                        system("pause");
                        pesagem();
                    } else {
                        fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
                    }
                    free(data);

                } else {
                    printf("Produto com ID %d não encontrado. Tente novamente!\n\n", id);
                }
            }
            break;

        case 2:
            main();
            break;

        default:
            printf("Opção inválida! Tente novamente.\n\n");
            break;
    }
    cJSON_Delete(root);
    cJSON_Delete(etiquetas_root);
}
