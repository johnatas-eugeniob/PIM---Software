#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <stdbool.h>
#include <string.h>
#include "caixa.h"
#include "pesagem.h"
#include "libs/cJSON.h"
#define MAX_SAIDAS 1000 //Quantidade maxima de saidas que a função vai verificar
#define MAX_ENTRADAS 1000
#include <time.h>


void controle_f(){
    system("cls || clear");
    int option;

    printf("| Controle de Fornecedores\n");
    printf("|------------------------------------|\n");
    printf("| Cadastrar fornecedor            - 1|\n");
    printf("| Ver fornecedores                - 2|\n");
    printf("| Atualizar dados de fornecedor   - 3|\n");
    printf("| Excluir fornecedores            - 4|\n");
    printf("| Voltar - 5                         |\n");
    printf("|------------------------------------|\n");
    printf("| Escolha uma opção: ");
    scanf("%d", &option);
    getchar();

    switch (option){
    case 1:
        cadastro_f();
        break;
    case 2:
        visualizar_f();
        break;
    case 3:
        atualizar_f();
        break;
    case 4:
        excluir_f();
        break;
    case 5:
        dashboard_adm_navigate();
        break;
    default:
        printf("Opção inválida!\n");
        system("pause");
        break;
    }
}
//-----------------------------------------------------------Cad f---------------------------------------------------------------
void cadastro_f() {
    system("cls || clear");
    char username[50], email[100], addres[150], category[50];
    bool cad_successful = false;
    int option, prazo;
    long long cpf, telefone;
    char input[100];  // Buffer para capturar a entrada do usuário

    FILE *file = fopen("bd/forn_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *cad_forn = cJSON_GetObjectItemCaseSensitive(root, "fornecedores");
    if (!cJSON_IsArray(cad_forn)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    int max_id = 0;
    cJSON *forn = NULL;
    cJSON_ArrayForEach(forn, cad_forn) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(forn, "id");
        if (cJSON_IsNumber(json_id) && json_id->valueint > max_id) {
            max_id = json_id->valueint;
        }
    }
    int next_id = max_id + 1;

    printf(" -------------------------------\n");
    printf("| Cadastrar novo Fornecedor     |\n");
    printf("|-------------------------------|\n");
    printf("| Começar cadastro - 1          |\n");
    printf("| Voltar - 2                    |\n");
    printf("|_______________________________|\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &option);
    getchar();  // Consumir o newline deixado por scanf
    printf("\n\n");

    switch(option) {
        case 1:
            while (!cad_successful) {
                printf("| Adicione as credenciais do Fornecedor  |\n");
                printf("|----------------------------------------|\n");
                printf("| Insira o nome: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0;  // Remover newline

                printf("| Insira o CPF/CNPJ (Somente números): ");
                scanf("%lld", &cpf);
                getchar();  // Consumir o newline deixado por scanf

                printf("| Insira o Endereço completo: ");
                fgets(addres, sizeof(addres), stdin);
                addres[strcspn(addres, "\n")] = 0;  // Remover newline

                printf("| Insira o Telefone para contato: ");
                scanf("%lld", &telefone);
                getchar();

                printf("| Insira o Email: ");
                fgets(email, sizeof(email), stdin);
                email[strcspn(email, "\n")] = 0;  // Remover newline

                printf("| Categoria mais vendida: ");
                fgets(category, sizeof(category), stdin);
                category[strcspn(category, "\n")] = 0;  // Remover newline

                printf("| Prazo para pagamento (em dias): ");
                scanf("%d", &prazo);
                getchar();  // Consumir o newline deixado por scanf

                printf("-----------------------------------------\n");

                bool forn_exists = false;
                cJSON_ArrayForEach(forn, cad_forn) {
                    cJSON *json_cpf = cJSON_GetObjectItemCaseSensitive(forn, "cpf");
                    cJSON *json_email = cJSON_GetObjectItemCaseSensitive(forn, "email");
                    if ((cJSON_IsString(json_email) && strcmp(email, json_email->valuestring) == 0) ||
                        (cJSON_IsNumber(json_cpf) && json_cpf->valuedouble == (double)cpf)) {
                        forn_exists = true;
                        break;
                    }
                }

                if (!forn_exists) {
                    cJSON *new_forn = cJSON_CreateObject();
                    cJSON_AddItemToObject(new_forn, "id", cJSON_CreateNumber(next_id)); // Adicionando ID
                    cJSON_AddItemToObject(new_forn, "username", cJSON_CreateString(username));
                    cJSON_AddItemToObject(new_forn, "cpf", cJSON_CreateNumber(cpf));
                    cJSON_AddItemToObject(new_forn, "endereco", cJSON_CreateString(addres));
                    cJSON_AddItemToObject(new_forn, "telefone", cJSON_CreateNumber(telefone));
                    cJSON_AddItemToObject(new_forn, "email", cJSON_CreateString(email));
                    cJSON_AddItemToObject(new_forn, "categoria", cJSON_CreateString(category));
                    cJSON_AddItemToObject(new_forn, "prazo", cJSON_CreateNumber(prazo));
                    cJSON_AddItemToArray(cad_forn, new_forn);

                    data = cJSON_Print(root);
                    file = fopen("bd/forn_list.json", "w");  // Corrigido o nome do arquivo
                    if (file) {
                        fwrite(data, 1, strlen(data), file);
                        fclose(file);
                        cad_successful = true;
                        printf(" Cadastro bem-sucedido!\n");
                        system("pause\n");
                        dashboard_adm_navigate();
                    } else {
                        fprintf(stderr, " Não foi possível abrir o arquivo para escrita!\n");
                    }
                    free(data);
                } else {
                    printf(" CPF ou e-mail já existente. Tente novamente!\n\n");
                }
            }
            cJSON_Delete(root);
            break;
        case 2:
            dashboard_adm_navigate();
            break;
        default:
            printf("Opção inválida!\n");
            break;
    }
}
//-----------------------------------------------------------Final cad f---------------------------------------------------------
//-----------------------------------------------------------Visualizar F--------------------------------------------------------
void visualizar_f() {
    system("cls || clear");
    int option;
    FILE *file = fopen("bd/forn_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *read_forn = cJSON_GetObjectItemCaseSensitive(root, "fornecedores");
    if (!cJSON_IsArray(read_forn)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Lista de todos os colaboradores:\n");
    printf(" ----------------------------------\n\n");

    cJSON *forn = NULL;
    cJSON_ArrayForEach(forn, read_forn) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(forn, "id");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(forn, "username");
        cJSON *cpf = cJSON_GetObjectItemCaseSensitive(forn, "cpf/cnpj");
        cJSON *addres = cJSON_GetObjectItemCaseSensitive(forn, "endereco");
        cJSON *telefone = cJSON_GetObjectItemCaseSensitive(forn, "telefone");
        cJSON *email = cJSON_GetObjectItemCaseSensitive(forn, "email");
        cJSON *category = cJSON_GetObjectItemCaseSensitive(forn, "categoria");
        cJSON *prazo = cJSON_GetObjectItemCaseSensitive(forn, "prazo");

        printf("| ID: %d\n", id->valueint);
        printf("| Nome: %s\n", username->valuestring);
        printf("| CPF/CNPJ: %lld\n", (long long) cpf->valuedouble);
        printf("| Endereço: %s\n", addres->valuestring);
        printf("| Contato: %lld\n", (long long) telefone->valuedouble);
        printf("| Email: %s\n", email->valuestring);
        printf("| Categoria mais vendida: %s\n", category->valuestring);
        printf("| Prazo de pagamento: %dd\n", prazo->valueint);
        printf("|-----------------------------------\n");
    }
    cJSON_Delete(root);

    printf("\nPara voltar, digite 1: ");
    scanf("%d", &option);
    if (option == 1) {
        controle_f(); // Certifique-se de que essa função existe para navegar no menu
    }
}
//-----------------------------------------------------------Fim Visualizar F----------------------------------------------------
//-----------------------------------------------------------Atualizar F---------------------------------------------------------
void atualizar_f() {
    system("cls || clear");
    int id, new_prazo;
    char new_username[50], new_email[100], new_addres[150], new_category[50];
    long long new_cpf, new_telefone;
    bool forn_found = false;

    FILE *file = fopen("bd/forn_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *update_forn = cJSON_GetObjectItemCaseSensitive(root, "fornecedores");
    if (!cJSON_IsArray(update_forn)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Digite o ID do fornecedor: ");
    scanf("%d", &id);
    printf("|---------------------------\n\n");
    getchar();  // Consumir o newline deixado por scanf

    cJSON *forn = NULL;
    cJSON_ArrayForEach(forn, update_forn) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(forn, "id");
        if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
            forn_found = true;

            printf("| Fornecedor encontrado! Insira os novos dados.\n");

            printf("| Novo nome: ");
            fgets(new_username, sizeof(new_username), stdin);
            new_username[strcspn(new_username, "\n")] = 0;  // Remover newline

            printf("| Novo CPF/CNPJ (Somente números): ");
            scanf("%lld", &new_cpf);
            getchar();  // Consumir o newline deixado por scanf

            printf("| Insira o Endereço completo: ");
            fgets(new_addres, sizeof(new_addres), stdin);
            new_addres[strcspn(new_addres, "\n")] = 0;  // Remover newline

            printf("| Insira o Telefone para contato: ");
            scanf("%lld", &new_telefone);
            getchar();

            printf("| Insira o Email: ");
            fgets(new_email, sizeof(new_email), stdin);
            new_email[strcspn(new_email, "\n")] = 0;  // Remover newline

            printf("| Categoria mais vendida: ");
            fgets(new_category, sizeof(new_category), stdin);
            new_category[strcspn(new_category, "\n")] = 0;  // Remover newline

            printf("| Prazo para pagamento (em dias): ");
            scanf("%d", &new_prazo);
            getchar();  // Consumir o newline deixado por scanf

            // Substituir os itens no objeto JSON
            cJSON_ReplaceItemInObject(forn, "username", cJSON_CreateString(new_username));
            cJSON_ReplaceItemInObject(forn, "cpf", cJSON_CreateNumber(new_cpf));
            cJSON_ReplaceItemInObject(forn, "endereco", cJSON_CreateString(new_addres));
            cJSON_ReplaceItemInObject(forn, "telefone", cJSON_CreateNumber(new_telefone));
            cJSON_ReplaceItemInObject(forn, "email", cJSON_CreateString(new_email));
            cJSON_ReplaceItemInObject(forn, "categoria", cJSON_CreateString(new_category));
            cJSON_ReplaceItemInObject(forn, "prazo", cJSON_CreateNumber(new_prazo));

            printf("\n-------------------------------\n");
            printf("| Dados atualizados com sucesso! |\n");
            printf("| -------------------------------|\n");

            // Salvar alterações no arquivo
            data = cJSON_Print(root);
            file = fopen("bd/forn_list.json", "w");  // Corrigido o nome do arquivo
            if (file) {
                fwrite(data, 1, strlen(data), file);
                fclose(file);
            } else {
                fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
            }
            free(data);

            break;
        }
    }

    if (!forn_found) {
        printf("Fornecedor com ID %d não encontrado.\n", id);
    }

    cJSON_Delete(root);
    system("pause");
    controle_f();
}
//-----------------------------------------------------------Fim atualizar F-----------------------------------------------------
//-----------------------------------------------------------Excluir F-----------------------------------------------------------
void excluir_f() {
    system("cls || clear");
    int id, option;
    bool forn_found = false;

    FILE *file = fopen("bd/forn_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *delete_forn = cJSON_GetObjectItemCaseSensitive(root, "fornecedores");
    if (!cJSON_IsArray(delete_forn)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Digite o ID do usuário: ");
    scanf("%d", &id);
    printf("|---------------------------\n");
    getchar();  // Consumir o newline deixado por scanf

    cJSON *forn = NULL;
    cJSON_ArrayForEach(forn, delete_forn) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(forn, "id");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(forn, "username");
        if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
            forn_found = true;

            printf("| Fornecedor encontrado!|\n");
            printf("|-----------------------|\n");
            printf("| ID do fornecedor a ser excluído: %d - %s\n| Você tem certeza disso?\n", id, username->valuestring);
            printf("| SIM - 1\n| NÃO - 2\n");
            printf("|_____________________________|\n");
            printf("| Escolha uma opção: ");
            scanf("%d", &option);
            getchar(); // Consumir o newline deixado por scanf

            switch (option) {
            case 1:
                cJSON_DeleteItemFromArray(delete_forn, cJSON_GetArraySize(delete_forn) - 1);
                printf(" ------------------------------ |\n");
                printf("| Usuário excluído com sucesso! |\n");
                printf("|-------------------------------|\n");

                data = cJSON_Print(root);
                file = fopen("bd/forn_list.json", "w");  // Corrigido o nome do arquivo
                if (file) {
                    fwrite(data, 1, strlen(data), file);
                    fclose(file);
                } else {
                    fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
                }
                free(data);
                system("pause");
                controle_f();
                break;
            case 2:
                printf("| Você cancelou a exclusão!\n");
                system("pause");
                controle_f();
                break;
            default:
                printf("| Opção inválida!\n");
                system("pause");
                controle_f();
            }
            break;
        }
    }
    if (!forn_found) {
        printf("| Fornecedor com ID %d não encontrado.\n", id);
        printf("| Tente novamente!\n");
        printf("|-------------------------------------\n");
        system("pause");
        controle_f();
    }
    cJSON_Delete(root);
}
//-----------------------------------------------------------Fim Excluir F-------------------------------------------------------
//-----------------------------------------------------------Registrar compras---------------------------------------------------
void registrar_compra() {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");  // Configurar separador decimal como ponto

    char fornecedor[50], unidade[10], data_entrada[20];
    bool cad_prod_successful = false;
    float qtd;
    int option, id, id_fornecedor;
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

    cJSON *cad_prod = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(cad_prod)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }
    printf(" -------------------------\n");
    printf("| Compra com fornecedor   |\n");
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
            while (!cad_prod_successful) {
                // Solicitar o ID do produto
                printf("| Insira o ID do produto: ");
                scanf("%d", &id);
                getchar();

                bool prod_found = false;
                cJSON *prod = NULL;
                cJSON_ArrayForEach(prod, cad_prod) {
                    cJSON *json_id = cJSON_GetObjectItemCaseSensitive(prod, "id");
                    if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
                        prod_found = true;
                        break;
                    }
                }

                if (prod_found) {
                    // Solicitar o ID do fornecedor
                    printf("| Insira o ID do fornecedor: ");
                    scanf("%d", &id_fornecedor);
                    getchar();

                    // Solicitar quantidade
                    printf("| Informe a quantidade comprada: ");
                    fgets(input, sizeof(input), stdin);  // Usar fgets para capturar como string
                    sscanf(input, "%f", &qtd);            // Converter para float

                    // Solicitar a data personalizada
                    printf("| Insira a data da compra (formato: YYYY-MM-DD): ");
                    fgets(data_entrada, sizeof(data_entrada), stdin);
                    data_entrada[strcspn(data_entrada, "\n")] = 0;

                    // Calcular o total com base no preco_comprado do produto
                    cJSON *preco_comprado = cJSON_GetObjectItemCaseSensitive(prod, "preco_de_compra");
                    if (!cJSON_IsNumber(preco_comprado)) {
                        printf("Preço de compra não encontrado para o produto.\n");
                        continue;
                    }
                    float total = qtd * preco_comprado->valuedouble;
                    printf("| O total da compra é: %.2f\n", total);
                    printf("\n----------------------------------------\n");

                    // Atualizar a quantidade disponível somando com a quantidade comprada
                    cJSON *quantidade_disponivel = cJSON_GetObjectItemCaseSensitive(prod, "quantidade_disponivel");
                    if (cJSON_IsNumber(quantidade_disponivel)) {
                        int nova_quantidade = quantidade_disponivel->valueint + (int)qtd;
                        cJSON_SetNumberValue(quantidade_disponivel, nova_quantidade);
                    } else {
                        cJSON_AddNumberToObject(prod, "quantidade_disponivel", (int)qtd);
                    }

                    // Adicionar a entrada ao histórico de compras com ID único
                    cJSON *entradas = cJSON_GetObjectItemCaseSensitive(prod, "entradas");
                    if (!cJSON_IsArray(entradas)) {
                        entradas = cJSON_AddArrayToObject(prod, "entradas");
                    }

                    // Gerar um novo ID para a entrada
                    int new_entry_id = 1;
                    cJSON *last_entry = cJSON_GetArrayItem(entradas, cJSON_GetArraySize(entradas) - 1);
                    if (last_entry) {
                        cJSON *json_entry_id = cJSON_GetObjectItemCaseSensitive(last_entry, "id");
                        if (cJSON_IsNumber(json_entry_id)) {
                            new_entry_id = json_entry_id->valueint + 1;
                        }
                    }

                    cJSON *new_entrada = cJSON_CreateObject();
                    cJSON_AddItemToObject(new_entrada, "id", cJSON_CreateNumber(new_entry_id)); // Adicionar novo ID
                    cJSON_AddItemToObject(new_entrada, "data", cJSON_CreateString(data_entrada)); // Usar data personalizada
                    cJSON_AddItemToObject(new_entrada, "quantidade", cJSON_CreateNumber(qtd));
                    cJSON_AddItemToObject(new_entrada, "id_fornecedor", cJSON_CreateNumber(id_fornecedor));
                    cJSON_AddItemToObject(new_entrada, "total", cJSON_CreateNumber(total));
                    cJSON_AddItemToArray(entradas, new_entrada);

                    // Salvar atualizações no arquivo
                    data = cJSON_Print(root);
                    file = fopen("bd/estoque_list.json", "w");
                    if (file) {
                        fwrite(data, 1, strlen(data), file);
                        fclose(file);
                        printf("| Compra registrada com sucesso!\n");
                        printf("|----------------------------------------\n\n");
                        system("pause");
                        registrar_compra();
                    } else {
                        fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
                    }
                    free(data);
                } else {
                    printf("Produto com ID %d não encontrado. Tente novamente!\n\n", id);
                }
            }
            cJSON_Delete(root);
            break;

        case 2:
            dashboard_adm_navigate();
            break;

        default:
            printf("Opção inválida!\n");
            controle_de_estoque();
            break;
    }
}
//-----------------------------------------------------------Final reg produto---------------------------------------------------
//-----------------------------------------------------------Visualizar prod-----------------------------------------------------
void visualizar_prod() {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");  // Configurar separador decimal como ponto
    int option;
    bool cad_prod_successful = false;  // Inicializando a variável utilizada

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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *read_prod = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(read_prod)) {  // Verificar se é um array
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    system("cls || clear");
    printf("| Lista de todos os produtos:\n");
    printf(" ----------------------------------\n\n");

    cJSON *prod = NULL;
    cJSON_ArrayForEach(prod, read_prod) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(prod, "id");
        cJSON *nome = cJSON_GetObjectItemCaseSensitive(prod, "nome");
        cJSON *preco_comprado = cJSON_GetObjectItemCaseSensitive(prod, "preco_de_compra");
        cJSON *preco_vendido = cJSON_GetObjectItemCaseSensitive(prod, "preco_vendido");
        cJSON *quantidade_disponivel = cJSON_GetObjectItemCaseSensitive(prod, "quantidade_disponivel");
        cJSON *unidade = cJSON_GetObjectItemCaseSensitive(prod, "unidade");
        cJSON *nivel_minimo = cJSON_GetObjectItemCaseSensitive(prod, "nivel_minimo");
        cJSON *nivel_maximo = cJSON_GetObjectItemCaseSensitive(prod, "nivel_maximo");
        cJSON *validade = cJSON_GetObjectItemCaseSensitive(prod, "validade");

        printf("| ID: %d\n", id->valueint);
        printf("| Nome: %s\n", nome->valuestring);
        printf("| Preco de compra: %.2f\n", preco_comprado->valuedouble);
        printf("| Preco vendido: %.2f\n", preco_vendido->valuedouble);
        printf("| Quantidade disponivel: %d\n", quantidade_disponivel->valueint);
        printf("| Unidade: %s\n", unidade->valuestring);
        printf("| Nivel minimo: %d\n", nivel_minimo->valueint);
        printf("| Nivel maximo: %d\n", nivel_maximo->valueint);
        printf("| Validade: %s\n", validade->valuestring);
        printf("|-----------------------------------\n");
    }
    cJSON_Delete(root);

    printf("\nPara voltar, digite 1: ");
    scanf("%d", &option);
    if (option == 1) {
        dashboard_adm_navigate();
    }
}
//-----------------------------------------------------------Final visualizar prod-----------------------------------------------
//-----------------------------------------------------------Excluir prod--------------------------------------------------------
void excluir_produto() {
    system("cls || clear");

    int id_para_excluir;
    printf("Digite o ID do produto que deseja excluir: ");
    scanf("%d", &id_para_excluir);

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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *read_prod = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(read_prod)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *prod = NULL;
    cJSON *prev = NULL;
    bool found = false;

    cJSON_ArrayForEach(prod, read_prod) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(prod, "id");
        if (id->valueint == id_para_excluir) {
            found = true;
            break;
        }
        prev = prod;
    }

    if (found) {
        if (prev) {
            cJSON_DeleteItemFromArray(read_prod, cJSON_GetArraySize(read_prod) - 1);
        } else {
            cJSON_DeleteItemFromArray(read_prod, 0);
        }
        file = fopen("bd/estoque_list.json", "w");
        if (!file) {
            fprintf(stderr, "Não foi possível abrir o arquivo!\n");
            cJSON_Delete(root);
            return;
        }
        char *updated_data = cJSON_Print(root);
        fwrite(updated_data, sizeof(char), strlen(updated_data), file);
        fclose(file);
        free(updated_data);
        printf("Produto com ID %d excluído com sucesso.\n", id_para_excluir);
    } else {
        printf("Produto com ID %d não encontrado.\n", id_para_excluir);
    }

    cJSON_Delete(root);
}
//-----------------------------------------------------------fim excluir prod----------------------------------------------------
//-----------------------------------------------------------Cadastrar Produto---------------------------------------------------
void cadastrar_prod() {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");  // Configurar separador decimal como ponto

    char nome[50], unidade[10], validade[20];
    int choice, id, nivel_minimo, nivel_maximo;
    float preco_vendido, preco_de_compra;
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

    cJSON *cad_prod = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(cad_prod)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf(" ________________________________________\n");
    printf("| Cadastro de Novo Produto               |\n");
    printf("|----------------------------------------|\n");
    printf("| Inserir informações - 1                |\n");
    printf("| Voltar - 2                             |\n");
    printf("|----------------------------------------|\n");
    printf("| Sua escolha: ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        // Coletar informações do novo produto
        printf("| Nome do produto: ");
        fgets(nome, sizeof(nome), stdin);
        nome[strcspn(nome, "\n")] = 0; // Remover o newline

        printf("| Preço de compra: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%f", &preco_de_compra);

        printf("| Preço de venda: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%f", &preco_vendido);

        printf("| Unidade (ex: kg, unidade): ");
        fgets(unidade, sizeof(unidade), stdin);
        unidade[strcspn(unidade, "\n")] = 0; // Remover o newline

        printf("| Nível mínimo de estoque: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &nivel_minimo);

        printf("| Nível máximo de estoque: ");
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &nivel_maximo);

        printf("| Validade (YYYY-MM-DD): ");
        fgets(validade, sizeof(validade), stdin);
        validade[strcspn(validade, "\n")] = 0; // Remover o newline

        // Gerar um novo ID para o produto
        int new_id = 1;
        cJSON *last_product = cJSON_GetArrayItem(cad_prod, cJSON_GetArraySize(cad_prod) - 1);
        if (last_product) {
            cJSON *json_id = cJSON_GetObjectItemCaseSensitive(last_product, "id");
            if (cJSON_IsNumber(json_id)) {
                new_id = json_id->valueint + 1;
            }
        }

        // Criar um novo objeto de produto
        cJSON *new_product = cJSON_CreateObject();
        cJSON_AddNumberToObject(new_product, "id", new_id);
        cJSON_AddStringToObject(new_product, "nome", nome);
        cJSON_AddNumberToObject(new_product, "preco_de_compra", preco_de_compra);
        cJSON_AddNumberToObject(new_product, "preco_vendido", preco_vendido);
        cJSON_AddNumberToObject(new_product, "quantidade_disponivel", 0);
        cJSON_AddStringToObject(new_product, "unidade", unidade);
        cJSON_AddNumberToObject(new_product, "nivel_minimo", nivel_minimo);
        cJSON_AddNumberToObject(new_product, "nivel_maximo", nivel_maximo);
        cJSON_AddStringToObject(new_product, "validade", validade);
        cJSON_AddItemToObject(new_product, "entradas", cJSON_CreateArray());
        cJSON_AddItemToObject(new_product, "saidas", cJSON_CreateArray());

        // Adicionar o novo produto ao array de produtos
        cJSON_AddItemToArray(cad_prod, new_product);

        // Salvar de volta no arquivo
        data = cJSON_Print(root);
        file = fopen("bd/estoque_list.json", "w");
        if (file) {
            fwrite(data, 1, strlen(data), file);
            fclose(file);
            printf(" _____________________________________\n");
            printf("| Produto cadastrado com sucesso!     |\n");
            printf("|-------------------------------------|\n");
            system("pause");
            cadastrar_prod();
        } else {
            fprintf(stderr, "| Não foi possível abrir o arquivo para escrita!\n");
        }
        free(data);
        cJSON_Delete(root);
    } else if (choice == 2) {
        dashboard_adm_navigate(); // Certifique-se de que essa função existe para navegar no menu
    } else {
        printf("Opção inválida!\n");
        cJSON_Delete(root);
        return;
    }
}
//-----------------------------------------------------------Final cadastrar-----------------------------------------------------
//-----------------------------------------------------------att prod------------------------------------------------------------
void atualizar_produto() {
    system("cls || clear");
    setlocale(LC_NUMERIC, "C");

    char nome[50], unidade[10], validade[20];
    int id, nivel_minimo, nivel_maximo, quantidade_disponivel;
    float preco_vendido, preco_de_compra;
    char input[100];  // Buffer para capturar a entrada do usuário

    printf("Digite o ID do produto que deseja atualizar: ");
    scanf("%d", &id);
    getchar();

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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *produtos = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *produto = NULL;
    bool found = false;
    cJSON_ArrayForEach(produto, produtos) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(produto, "id");
        if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
            found = true;
            break;
        }
    }

    if (!found) {
        printf("Produto com ID %d não encontrado.\n", id);
        cJSON_Delete(root);
        return;
    }

    printf("Insira as novas informações do produto (pressione Enter para manter o valor atual):\n");

    printf("Nome (%s): ", cJSON_GetObjectItemCaseSensitive(produto, "nome")->valuestring);
    fgets(nome, sizeof(nome), stdin);
    if (strcmp(nome, "\n") != 0) {
        nome[strcspn(nome, "\n")] = '\0';
        cJSON_ReplaceItemInObject(produto, "nome", cJSON_CreateString(nome));
    }

    printf("Preço de compra (%.2f): ", cJSON_GetObjectItemCaseSensitive(produto, "preco_de_compra")->valuedouble);
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "\n") != 0) {
        sscanf(input, "%f", &preco_de_compra);
        cJSON_ReplaceItemInObject(produto, "preco_de_compra", cJSON_CreateNumber(preco_de_compra));
    }

    printf("Preço de venda (%.2f): ", cJSON_GetObjectItemCaseSensitive(produto, "preco_vendido")->valuedouble);
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "\n") != 0) {
        sscanf(input, "%f", &preco_vendido);
        cJSON_ReplaceItemInObject(produto, "preco_vendido", cJSON_CreateNumber(preco_vendido));
    }

    printf("Quantidade disponível (%d): ", cJSON_GetObjectItemCaseSensitive(produto, "quantidade_disponivel")->valueint);
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "\n") != 0) {
        sscanf(input, "%d", &quantidade_disponivel);
        cJSON_ReplaceItemInObject(produto, "quantidade_disponivel", cJSON_CreateNumber(quantidade_disponivel));
    }

    printf("Unidade (%s): ", cJSON_GetObjectItemCaseSensitive(produto, "unidade")->valuestring);
    fgets(unidade, sizeof(unidade), stdin);
    if (strcmp(unidade, "\n") != 0) {
        unidade[strcspn(unidade, "\n")] = '\0';
        cJSON_ReplaceItemInObject(produto, "unidade", cJSON_CreateString(unidade));
    }

    printf("Nível mínimo (%d): ", cJSON_GetObjectItemCaseSensitive(produto, "nivel_minimo")->valueint);
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "\n") != 0) {
        sscanf(input, "%d", &nivel_minimo);
        cJSON_ReplaceItemInObject(produto, "nivel_minimo", cJSON_CreateNumber(nivel_minimo));
    }

    printf("Nível máximo (%d): ", cJSON_GetObjectItemCaseSensitive(produto, "nivel_maximo")->valueint);
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "\n") != 0) {
        sscanf(input, "%d", &nivel_maximo);
        cJSON_ReplaceItemInObject(produto, "nivel_maximo", cJSON_CreateNumber(nivel_maximo));
    }

    printf("Validade (%s): ", cJSON_GetObjectItemCaseSensitive(produto, "validade")->valuestring);
    fgets(validade, sizeof(validade), stdin);
    if (strcmp(validade, "\n") != 0) {
        validade[strcspn(validade, "\n")] = '\0';
        cJSON_ReplaceItemInObject(produto, "validade", cJSON_CreateString(validade));
    }

    file = fopen("bd/estoque_list.json", "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo!\n");
        cJSON_Delete(root);
        return;
    }
    char *updated_data = cJSON_Print(root);
    fwrite(updated_data, sizeof(char), strlen(updated_data), file);
    fclose(file);
    free(updated_data);
    cJSON_Delete(root);

    printf("Produto com ID %d atualizado com sucesso.\n", id);
}
//-----------------------------------------------------------final att-----------------------------------------------------------
//-----------------------------------------------------------Cadastro C----------------------------------------------------------
void cadastro_c() {
    system ("cls || clear");
    char username[50], email[100], password[50];
    bool cad_successful = false;
    int option;
    long long cpf;
    char input[100];  // Buffer para capturar a entrada do usuário

    FILE *file = fopen("bd/users_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *cad_users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(cad_users)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    int max_id = 0;
    cJSON *user = NULL;
    cJSON_ArrayForEach(user, cad_users) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(user, "id");
        if (cJSON_IsNumber(json_id) && json_id->valueint > max_id) {
            max_id = json_id->valueint;
        }
    }
    int next_id = max_id + 1;

    printf(" -------------------------------\n");
    printf("| Menu Controle de Colaboradores|\n");
    printf("|-------------------------------|\n");
    printf("| Começar cadastro           - 1|\n");
    printf("| Visualizar colaboradores   - 2|\n");
    printf("| Atualizar colaborador      - 3|\n");
    printf("| Excluir colaborador        - 4|\n");
    printf("| Voltar - 5                    |\n");
    printf("|_______________________________|\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &option);
    getchar();  // Consumir o newline deixado por scanf
    printf("\n\n");

    switch(option) {
        case 1:
            while (!cad_successful) {
                printf("| Adicione as credenciais do colaborador |\n");
                printf("|----------------------------------------|\n");
                printf("| Insira o nome: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0;  // Remover newline

                printf("| Insira o CPF (Somente números): ");
                scanf("%lld", &cpf);
                getchar();  // Consumir o newline deixado por scanf

                printf("| Insira o Email: ");
                fgets(email, sizeof(email), stdin);
                email[strcspn(email, "\n")] = 0;  // Remover newline

                printf("-----------------------------------------\n");

                bool user_exists = false;
                cJSON_ArrayForEach(user, cad_users) {
                    cJSON *json_cpf = cJSON_GetObjectItemCaseSensitive(user, "cpf");
                    cJSON *json_email = cJSON_GetObjectItemCaseSensitive(user, "email");
                    if ((cJSON_IsString(json_email) && strcmp(email, json_email->valuestring) == 0) ||
                        (cJSON_IsNumber(json_cpf) && json_cpf->valuedouble == (double)cpf)) {
                        user_exists = true;
                        break;
                    }
                }

                if (!user_exists) {
                    cJSON *new_user = cJSON_CreateObject();
                    cJSON_AddItemToObject(new_user, "id", cJSON_CreateNumber(next_id)); // Adicionando ID
                    cJSON_AddItemToObject(new_user, "username", cJSON_CreateString(username));
                    cJSON_AddItemToObject(new_user, "cpf", cJSON_CreateNumber(cpf));
                    cJSON_AddItemToObject(new_user, "email", cJSON_CreateString(email));
                    cJSON_AddItemToObject(new_user, "password", cJSON_CreateString("123456"));
                    cJSON_AddItemToArray(cad_users, new_user);

                    data = cJSON_Print(root);
                    file = fopen("bd/users_list.json", "w");
                    if (file) {
                        fwrite(data, 1, strlen(data), file);
                        fclose(file);
                        cad_successful = true;
                        printf(" Cadastro bem-sucedido!\n");
                        system("pause\n");
                        dashboard_adm_navigate();
                    } else {
                        fprintf(stderr, " Não foi possível abrir o arquivo para escrita!\n");
                    }
                    free(data);
                } else {
                    printf(" CPF ou e-mail já existente. Tente novamente!\n\n");
                }
            }
            cJSON_Delete(root);
            break;
        case 2:
            visualizar_c();
            break;
        case 3:
            atualizar_c();
            break;
        case 4:
            excluir_c();
            break;
        case 5:
            dashboard_adm_navigate();
            break;
        default:
            printf("Opção inválida!\n");
            break;
    }
}
//-----------------------------------------------------------------Fim Cadastro C------------------------------------------------
//-----------------------------------------------------------------Visualizar Colaboradores--------------------------------------
void visualizar_c() {
    system ("cls || clear");
    int option;
    FILE *file = fopen("bd/users_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *read_users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(read_users)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Lista de todos os colaboradores:\n");
    printf(" ----------------------------------\n\n");

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, read_users) {
        cJSON *id = cJSON_GetObjectItemCaseSensitive(user, "id");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(user, "username");
        cJSON *cpf = cJSON_GetObjectItemCaseSensitive(user, "cpf");
        cJSON *email = cJSON_GetObjectItemCaseSensitive(user, "email");

        printf("| ID: %d\n", id->valueint);
        printf("| Nome: %s\n", username->valuestring);
        printf("| CPF: %lld\n", (long long) cpf->valuedouble);
        printf("| Email: %s\n", email->valuestring);
        printf("|-----------------------------------\n");
    }
    cJSON_Delete(root);
    printf("\nPara voltar, digite 1: ");
    scanf("%d", &option);
    if(option == 1){
        cadastro_c();
    }
}
//-----------------------------------------------------------------Final do read-------------------------------------------------
//-----------------------------------------------------------------Atualizar C---------------------------------------------------
void atualizar_c() {
    system("cls || clear");
    int id;
    char new_username[50], new_email[100];
    long long new_cpf;
    bool user_found = false;

    FILE *file = fopen("bd/users_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *update_users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(update_users)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Digite o ID do usuario: ");
    scanf("%d", &id);
    printf("|---------------------------\n\n");
    getchar();  // Consumir o newline deixado por scanf

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, update_users) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(user, "id");
        if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
            user_found = true;

            printf("| Usuário encontrado! Insira os novos dados.\n");

            printf("| Novo nome: ");
            fgets(new_username, sizeof(new_username), stdin);
            new_username[strcspn(new_username, "\n")] = 0;  // Remover newline

            printf("| Novo CPF (Somente números): ");
            scanf("%lld", &new_cpf);
            getchar();  // Consumir o newline deixado por scanf

            printf("| Novo email: ");
            fgets(new_email, sizeof(new_email), stdin);
            new_email[strcspn(new_email, "\n")] = 0;  // Remover newline

            // Substituir os itens no objeto JSON
            cJSON_ReplaceItemInObject(user, "username", cJSON_CreateString(new_username));
            cJSON_ReplaceItemInObject(user, "cpf", cJSON_CreateNumber(new_cpf));
            cJSON_ReplaceItemInObject(user, "email", cJSON_CreateString(new_email));

            printf("\n-------------------------------\n");
            printf("| Dados atualizados com sucesso! |\n");
            printf("| -------------------------------|\n");

            // Salvar alterações no arquivo
            data = cJSON_Print(root);
            file = fopen("bd/users_list.json", "w");
            if (file) {
                fwrite(data, 1, strlen(data), file);
                fclose(file);
            } else {
                fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
            }
            free(data);

            break;
        }
    }

    if (!user_found) {
        printf("Usuário com ID %d não encontrado.\n", id);
    }

    cJSON_Delete(root);
    system("pause");
    cadastro_c();
}
//-----------------------------------------------------------------Final Atualzar------------------------------------------------
//-----------------------------------------------------------------Atualzar senha------------------------------------------------
void atualizar_senha(const char *user_email) {
    system("cls || clear");
    char new_password[50];
    bool user_found = false;

    FILE *file = fopen("bd/users_list.json", "r");
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
    free(data);  // Mover free(data) para o lugar correto
    if (!root) {
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *update_users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(update_users)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, update_users) {
        cJSON *json_email = cJSON_GetObjectItemCaseSensitive(user, "email");
        if (cJSON_IsString(json_email) && (json_email->valuestring != NULL) && strcmp(json_email->valuestring, user_email) == 0) {
            user_found = true;

            printf("| Usuário encontrado! Insira os novos dados.\n");
            printf("| Email: %s\n", user_email);
            printf("| Insira sua nova senha: ");
            scanf("%49s", new_password);  // Remover o operador & para new_password
            getchar();
            printf("\n");

            // Substituir a senha no JSON
            cJSON *json_password = cJSON_GetObjectItemCaseSensitive(user, "password");
            if (json_password) {
                cJSON_SetValuestring(json_password, new_password);
            } else {
                cJSON_AddItemToObject(user, "password", cJSON_CreateString(new_password));
            }

            printf(" -------------------------------\n");
            printf("| Senha atualizada com sucesso!  |\n");
            printf("|--------------------------------|\n");

            // Salvar alterações no arquivo
            char *new_data = cJSON_Print(root);
            file = fopen("bd/users_list.json", "w");
            if (file) {
                fwrite(new_data, 1, strlen(new_data), file);
                fclose(file);
            } else {
                fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
            }
            free(new_data);

            break;
        }
    }

    if (!user_found) {
        printf("Usuário não encontrado!\n");
    }

    cJSON_Delete(root);
    system("pause");
    dashboard_navigate(user_email);
}
//-----------------------------------------------------------------Final Atualzar senha------------------------------------------
//-----------------------------------------------------------------Excluir-------------------------------------------------------
void excluir_c(){
    system("cls || clear");
    int id, option;
    bool user_found = false;

    FILE *file = fopen("bd/users_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *delete_user = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(delete_user)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf("| Digite o ID do usuario: ");
    scanf("%d", &id);
    printf("|-------------------------------------|\n");
    getchar();  // Consumir o newline deixado por scanf

    cJSON *user = NULL;
    cJSON_ArrayForEach(user, delete_user) {
        cJSON *json_id = cJSON_GetObjectItemCaseSensitive(user, "id");
        cJSON *username = cJSON_GetObjectItemCaseSensitive(user, "username");
        if (cJSON_IsNumber(json_id) && json_id->valueint == id) {
            user_found = true;

            printf("|    Usuário encontrado!              |\n");
            printf("|-------------------------------------|\n");
            printf("| ID e Nome do usuario a ser excluído:\n");
            printf("| ID: %d - Nome: %s                   \n", id, username->valuestring);
            printf("|-------------------------------------|\n");
            printf("| Você tem certeza disso?             |\n");
            printf("| SIM - 1                             |\n");
            printf("| NÃO - 2                             |\n");
            printf("|-------------------------------------|\n");
            printf("| Escolha uma opção: ");
            scanf("%d", &option);
            getchar(); // Consumir o newline deixado por scanf

            switch (option){
            case 1:
                cJSON_DeleteItemFromArray(delete_user, cJSON_GetArraySize(delete_user) - 1);
                printf(" ------------------------------ |\n");
                printf("| Usuário excluído com sucesso! |\n");
                printf("|-------------------------------|\n");

                data = cJSON_Print(root);
                file = fopen("bd/users_list.json", "w");
                if (file) {
                    fwrite(data, 1, strlen(data), file);
                    fclose(file);
                } else {
                    fprintf(stderr, "Não foi possível abrir o arquivo para escrita!\n");
                }
                free(data);
                system("pause");
                cadastro_c();
                break;
            case 2:
                printf("| Você cancelou a exclusão!\n");
                system("pause");
                cadastro_c();
                break;
            default:
                printf("| Opção inválida!\n");
                system("pause");
                cadastro_c();
            }
            break;
        }
    }
    if (!user_found) {
        printf("| Usuário com ID %d não encontrado.\n", id);
        printf("| Tente novamente!\n");
        printf("|-----------------------------------\n");
        system("pause");
        cadastro_c();
    }
    cJSON_Delete(root);
}
//-----------------------------------------------------------------Final Excluir-------------------------------------------------
//-----------------------------------------------------------------Dashboard-----------------------------------------------------
void dashboard_navigate(const char *user_email){
    system ("cls || clear");
    int option;
    bool senha_default = false;
    FILE *file = fopen("bd/users_list.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *read_users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(read_users)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }
    cJSON *user = NULL;
    bool email_found = false;
    cJSON_ArrayForEach(user, read_users) {
        cJSON *email = cJSON_GetObjectItemCaseSensitive(user, "email");
        cJSON *password = cJSON_GetObjectItemCaseSensitive(user, "password");

        if (cJSON_IsString(email) && (email->valuestring != NULL) && strcmp(email->valuestring, user_email) == 0) {
            email_found = true;
            if (cJSON_IsString(password) && (password->valuestring != NULL) && strcmp(password->valuestring, "123456") == 0) {
                    senha_default = true;
            }
            break;
        }
    }
    if (senha_default){
        printf(" _______________________________________\n");
        printf("| Recomendado atualizar senha!          |\n");
        printf("| Deseja atualizar a senha de acesso?   |\n");
        printf("| Sim - 1                               |\n");
        printf("| Não - 2                               |\n");
        printf("|---------------------------------------|\n");
        printf("| Escolha sua opção: ");
        scanf("%d", &option);
        if(option == 1){
            atualizar_senha(user_email);
        }
    }else if(email_found){ //Dashboard de colaborador começa aqui:
        int option;
        system ("cls || clear");
        printf(" ______________________________\n");
        printf("|          Bem vindo!          |\n");
        printf("|------------------------------|\n");
        printf("|   O que você deseja fazer?   |\n");
        printf("|______________________________|\n");
        printf("| Registro de Vendas        - 1|\n");
        printf("| Pesagem de Produtos       - 2|\n");
        printf("| Gerenciamento de Estoque  - 3|\n");
        printf("| Sair - 4                     |\n");
        printf("|------------------------------|\n");
        printf("|Escolha uma opção: ");
        scanf("%d", &option);

        switch (option){
            case 1:
                system ("cls || clear");
                fluxo_caixa();
                break;
            case 2:
                system ("cls || clear");
                pesagem();
                break;
            case 3:
                system ("cls || clear");
                controle_de_estoque_c();
                break;
            case 4:
                system ("cls || clear");
                main();
                break;
            default:
                printf("| Opção inválida!\n");
                break;
        }
        return 0;
    }else{
        printf("Usuario não encontrado!\n");
    }
    cJSON_Delete(root);
}
//-----------------------------------------------------------------Controle financeiro-------------------------------------------
void controle_financeiro(){
    int option;
    printf("Selecione uma das opções: ");
    printf("Controle de Vendas   - 1\n");
    printf("Controle de Despesas - 2\n");
    printf("Previsões            - 3\n");
    printf("Relatórios           - 4\n");
    printf("Fluxo de Caixa       - 5\n");
    printf("Sair - 6\n");

    switch (option){
    case 1:
        printf("Chama função");
        break;
    case 2:
        printf("Chama função");
        break;
    case 3:
        printf("Chama função");
        break;
    case 4:
        printf("Chama função");
        break;
    case 5:
        printf("Chama função");
        break;
    case 6:
        dashboard_adm_navigate();
        break;
    default:
        printf("Opção inválida!\n");
        break;
    }
    return;
}
//-----------------------------------------------------------------control prod--------------------------------------------------
void controle_de_estoque(){
    system("cls || clear");
    int option;
    printf(" --------------------------------------|\n");
    printf("|         O que você deseja?           |\n");
    printf("|______________________________________|\n");
    printf("| Ver produtos                      - 1|\n");
    printf("| Cadastrar produtos                - 2|\n");
    printf("| Registrar compra com fornecedor   - 3|\n");
    printf("| Excluir produto                   - 4|\n");
    printf("| Voltar                            - 5|\n");
    printf("|--------------------------------------|\n");
    printf("| Escolha uma opção: ");
    scanf("%d", &option);

    switch (option){
    case 1:
        visualizar_prod();
        break;
    case 2:
        cadastrar_prod();
        break;
    case 3:
        registrar_compra();
        break;
    case 4:
        excluir_produto();
        break;
    case 5:
        dashboard_adm_navigate();
        break;
    default:
        printf("Opção inválida!\n");
        break;
    }
}
//-----------------------------------------------------------------Final control prod--------------------------------------------
void controle_de_estoque_c(){
    system("cls || clear");
    int option;
    printf(" --------------------------------------|\n");
    printf("|         O que você deseja?           |\n");
    printf("|______________________________________|\n");
    printf("| Ver produtos                      - 1|\n");
    printf("| Cadastrar produtos                - 2|\n");
    printf("| Excluir produto                   - 4|\n");
    printf("| Voltar                            - 5|\n");
    printf("|--------------------------------------|\n");
    printf("| Escolha uma opção: ");
    scanf("%d", &option);

    switch (option){
    case 1:
        visualizar_prod();
        break;
    case 2:
        cadastrar_prod();
        break;
    case 3:
        excluir_produto();
        break;
    case 4:
        dashboard_adm_navigate();
        break;
    default:
        printf("Opção inválida!\n");
        break;
    }
}
//-----------------------------------------------------------------Dash ADM------------------------------------------------------
void dashboard_adm_navigate(){
    int option;
    system ("cls || clear");
    printf(" ______________________________\n");
    printf("|          Bem vindo!          |\n");
    printf("|------------------------------|\n");
    printf("| O que você deseja fazer?     |\n");
    printf("|______________________________|\n");
    printf("| Controle de Colaboradores - 1|\n");
    printf("| Controle Financeiro       - 2|\n");
    printf("| Controle de Estoque       - 3|\n");
    printf("| Controle de Fornecedores  - 4|\n");
    printf("| Sair - 5                     |\n");
    printf("|------------------------------|\n");
    printf("|Escolha uma opção: ");
    scanf("%d", &option);

    switch (option){
        case 1:
            system ("cls || clear");
            cadastro_c();
            break;
        case 2:
            system ("cls || clear");
            financeiro();
            break;
        case 3:
            system ("cls || clear");
            controle_de_estoque();
            break;
        case 4:
            controle_f();
            break;
        case 5:
            system ("cls || clear");
            main();
            break;
        default:
            printf("| Opção inválida!\n");
            break;
    }
    return 0;
}
//-----------------------------------------------------------------Login ADM-----------------------------------------------------
void login_adm(){
    int option;
    char email[100], password[50];
    bool login_adm_successful = false;
    FILE *file = fopen("bd/adm_list.json", "r");
    if (!file) {
        fprintf(stderr, " Não foi possível abrir o arquivo!\n");
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
        fprintf(stderr, " Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *admins = cJSON_GetObjectItemCaseSensitive(root, "admins");
    if (!cJSON_IsArray(admins)) {
        fprintf(stderr, " Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    printf(" --------------------------\n");
    printf("| Menu Login Gerencial     |\n");
    printf("|__________________________|\n");
    printf("| Inserir credênciais - 1  |\n");
    printf("| Voltar - 2               |\n");
    printf("|__________________________|\n");
    printf("| Escolha sua opçao: ");
    scanf("%d", &option);
    printf("\n\n");

    switch (option){
        case 1:
            system ("cls || clear");
            while (!login_adm_successful){
            printf(" ______________________________\n");
            printf("|Faça o login para acessar \n|o conteúdo de ADM\n");
            printf("|------------------------------|\n");
            printf("| Insira seu nome e senha\n");
            printf("| Email: ");
            scanf("%s", &email);
            printf("| Senha: ");
            int i = 0;
            char ch;
            while ((ch = _getch()) != '\r') { // Enter key is '\r'
                if (ch == '\b' && i > 0) { // Backspace key is '\b'
                    printf("\b \b"); i--;
                } else if (ch != '\b' && i < sizeof(password) - 1) {
                    printf("*");
                    password[i] = ch;
                    i++;
                }
            }
            password[i] = '\0'; // Null-terminate the password
            printf("\n");
            printf("------------------------------\n");


            cJSON *adm = NULL;
                cJSON_ArrayForEach(adm, admins) {
                    cJSON *json_email = cJSON_GetObjectItemCaseSensitive(adm, "email");
                    cJSON *json_password = cJSON_GetObjectItemCaseSensitive(adm, "password");
                    if (cJSON_IsString(json_email) && cJSON_IsString(json_password) &&
                        strcmp(email, json_email->valuestring) == 0 &&
                        strcmp(password, json_password->valuestring) == 0) {
                        printf("| Login bem sucedido!          |\n");
                        printf("|------------------------------|\n");
                        login_adm_successful = true;
                        dashboard_adm_navigate(email);
                    }
                }
                if (!login_adm_successful) {
                    system ("cls || clear");
                    printf("| Erro - Faça o login novamente!\n\n");
                }
            }
            cJSON_Delete(root);
            break;
        case 2:
            system ("cls || clear");
            main();
            break;
        default:
            printf("| Opção inválida!\n");
            login_adm();
            break;
    }
}
//-----------------------------------------------------------------Login_colaborador---------------------------------------------
 void login_c() {
    int option;
    char email[100], password[50];
    bool login_successful = false;
    FILE *file = fopen("bd/users_list.json", "r");
    if (!file) {
        fprintf(stderr, " Não foi possível abrir o arquivo!\n");
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
        fprintf(stderr, " Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *users = cJSON_GetObjectItemCaseSensitive(root, "users");
    if (!cJSON_IsArray(users)) {
        fprintf(stderr, " Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }


    printf(" ______________________________\n");
    printf("|  Menu login  Colaborador  |\n");
    printf("|---------------------------|\n");
    printf("| Inserir credênciais - 1   |\n");
    printf("| Voltar - 2                |\n");
    printf("|---------------------------|\n");
    printf("| Escolha sua opçao: ");
    scanf("%d", &option);
    printf("\n\n");

    switch (option){
        case 1:
            while (!login_successful) {
                printf("| Para o primeiro acesso,\n| digite a senha padrão enviada pelo RH\n");
                printf("\n----------------------------\n");
                printf("| Digite seu email: ");
                scanf("%s", email);
                printf("| Digite sua senha: ");
                int i = 0;
                char ch;
                while ((ch = _getch()) != '\r') { // Enter key is '\r'
                    if (ch == '\b' && i > 0) { // Backspace key is '\b'
                        printf("\b \b"); i--;
                    } else if (ch != '\b' && i < sizeof(password) - 1) {
                        printf("*");
                        password[i] = ch;
                        i++;
                    }
                }
                password[i] = '\0'; // Null-terminate the password
                printf("\n");
                printf("------------------------------\n");

                cJSON *user = NULL;
                cJSON_ArrayForEach(user, users) {
                    cJSON *json_email = cJSON_GetObjectItemCaseSensitive(user, "email");
                    cJSON *json_password = cJSON_GetObjectItemCaseSensitive(user, "password");
                    if (cJSON_IsString(json_email) && cJSON_IsString(json_password) &&
                        strcmp(email, json_email->valuestring) == 0 &&
                        strcmp(password, json_password->valuestring) == 0) {
                        printf(" ________________________\n");
                        printf("| Login bem sucedido!    |\n");
                        printf("|------------------------|\n");
                        login_successful = true;
                        dashboard_navigate(email);
                    }
                }
                if (!login_successful) {
                    printf("Faça o login novamente!\n");
                }
            }
            cJSON_Delete(root);
            break;
        case 2:
            system ("cls || clear");
            main();
            break;
        default:
            system ("cls || clear");
            printf("Opção inválida!\n");
            login_adm();
    }
}

double calcular_total_saidas(const char *estoque_file_path) {
    FILE *file = fopen(estoque_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de estoque!\n");
        return 0.0;
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
        fprintf(stderr, "Erro ao analisar JSON de estoque: %s\n", cJSON_GetErrorPtr());
        return 0.0;
    }

    cJSON *produtos = cJSON_GetObjectItem(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON de estoque inválido\n");
        cJSON_Delete(root);
        return 0.0;
    }

    double total_saidas = 0.0;
    int id_saidas[MAX_SAIDAS] = {0};
    int num_saidas = 0;

    cJSON *produto;
    cJSON_ArrayForEach(produto, produtos) {
        cJSON *saidas = cJSON_GetObjectItem(produto, "saidas");
        if (cJSON_IsArray(saidas)) {
            cJSON *saida;
            cJSON_ArrayForEach(saida, saidas) {
                cJSON *id = cJSON_GetObjectItem(saida, "id");
                if (cJSON_IsNumber(id)) {
                    bool id_existente = false;
                    for (int i = 0; i < num_saidas; i++) {
                        if (id_saidas[i] == id->valueint) {
                            id_existente = true;
                            break;
                        }
                    }
                    if (!id_existente) {
                        id_saidas[num_saidas++] = id->valueint;
                        cJSON *total = cJSON_GetObjectItem(saida, "total");
                        if (cJSON_IsNumber(total)) {
                            total_saidas += total->valuedouble;
                        }
                    }
                }
            }
        }
    }

    cJSON_Delete(root);
    return total_saidas;
}

void atualizar_caixa(const char *caixa_file_path, double total_saidas) {
    FILE *file = fopen(caixa_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa!\n");
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
        fprintf(stderr, "Erro ao analisar JSON de caixa: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *resumo_financeiro = cJSON_GetObjectItem(root, "resumo_financeiro");
    if (cJSON_IsObject(resumo_financeiro)) {
        cJSON *total_receitas = cJSON_GetObjectItem(resumo_financeiro, "total_receitas");
        if (cJSON_IsNumber(total_receitas)) {
            total_receitas->valuedouble += total_saidas;
        } else {
            cJSON_AddNumberToObject(resumo_financeiro, "total_receitas", total_saidas);
        }
    } else {
        fprintf(stderr, "Erro ao acessar resumo financeiro no JSON de caixa\n");
        cJSON_Delete(root);
        return;
    }

    file = fopen(caixa_file_path, "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa para escrita!\n");
        cJSON_Delete(root);
        return;
    }
    char *updated_data = cJSON_Print(root);
    fprintf(file, "%s", updated_data);
    fclose(file);

    cJSON_Delete(root);
    free(updated_data);
    printf("Total de receitas atualizado com sucesso!\n");
}

// Função para calcular o total das despesas
double calcular_total_despesas(const char *estoque_file_path) {
    FILE *file = fopen(estoque_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de estoque!\n");
        return 0.0;
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
        fprintf(stderr, "Erro ao analisar JSON de estoque: %s\n", cJSON_GetErrorPtr());
        return 0.0;
    }

    cJSON *produtos = cJSON_GetObjectItem(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON de estoque inválido\n");
        cJSON_Delete(root);
        return 0.0;
    }

    double total_despesas = 0.0;
    int id_entradas[MAX_ENTRADAS] = {0};
    int num_entradas = 0;

    cJSON *produto;
    cJSON_ArrayForEach(produto, produtos) {
        cJSON *entradas = cJSON_GetObjectItem(produto, "entradas");
        if (cJSON_IsArray(entradas)) {
            cJSON *entrada;
            cJSON_ArrayForEach(entrada, entradas) {
                cJSON *id = cJSON_GetObjectItem(entrada, "id");
                if (cJSON_IsNumber(id)) {
                    bool id_existente = false;
                    for (int i = 0; i < num_entradas; i++) {
                        if (id_entradas[i] == id->valueint) {
                            id_existente = true;
                            break;
                        }
                    }
                    if (!id_existente) {
                        id_entradas[num_entradas++] = id->valueint;
                        cJSON *total = cJSON_GetObjectItem(entrada, "total");
                        if (cJSON_IsNumber(total)) {
                            total_despesas += total->valuedouble;
                        }
                    }
                }
            }
        }
    }

    cJSON_Delete(root);
    return total_despesas;
}

// Função para atualizar o total de despesas
void atualizar_despesas(const char *caixa_file_path, double total_despesas) {
    FILE *file = fopen(caixa_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa!\n");
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
        fprintf(stderr, "Erro ao analisar JSON de caixa: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *resumo_financeiro = cJSON_GetObjectItem(root, "resumo_financeiro");
    if (cJSON_IsObject(resumo_financeiro)) {
        cJSON *total_despesas_obj = cJSON_GetObjectItem(resumo_financeiro, "total_despesas");
        if (cJSON_IsNumber(total_despesas_obj)) {
            total_despesas_obj->valuedouble += total_despesas;
        } else {
            cJSON_AddNumberToObject(resumo_financeiro, "total_despesas", total_despesas);
        }
    } else {
        fprintf(stderr, "Erro ao acessar resumo financeiro no JSON de caixa\n");
        cJSON_Delete(root);
        return;
    }

    file = fopen(caixa_file_path, "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa para escrita!\n");
        cJSON_Delete(root);
        return;
    }
    char *updated_data = cJSON_Print(root);
    fprintf(file, "%s", updated_data);
    fclose(file);

    cJSON_Delete(root);
    free(updated_data);
    printf("Total de despesas atualizado com sucesso!\n");
}

// Função para atualizar as formas de pagamento
void atualizar_formas_pagamento(const char *caixa_file_path, const char *extrato_file_path) {
    // Ler o arquivo de extrato
    FILE *file = fopen(extrato_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de extrato!\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *extrato_root = cJSON_Parse(data);
    free(data);
    if (!extrato_root) {
        fprintf(stderr, "Erro ao analisar JSON de extrato: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // Ler o arquivo de caixa
    file = fopen(caixa_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa!\n");
        cJSON_Delete(extrato_root);
        return;
    }
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *caixa_root = cJSON_Parse(data);
    free(data);
    if (!caixa_root) {
        fprintf(stderr, "Erro ao analisar JSON de caixa: %s\n", cJSON_GetErrorPtr());
        cJSON_Delete(extrato_root);
        return;
    }

    cJSON *resumo_financeiro = cJSON_GetObjectItem(caixa_root, "resumo_financeiro");
    if (!cJSON_IsObject(resumo_financeiro)) {
        fprintf(stderr, "Erro ao acessar resumo financeiro no JSON de caixa\n");
        cJSON_Delete(extrato_root);
        cJSON_Delete(caixa_root);
        return;
    }

    cJSON *formas_pagamento = cJSON_GetObjectItem(resumo_financeiro, "formas_pagamento");
    if (!cJSON_IsObject(formas_pagamento)) {
        fprintf(stderr, "Erro ao acessar formas de pagamento no JSON de caixa\n");
        cJSON_Delete(extrato_root);
        cJSON_Delete(caixa_root);
        return;
    }

    // Atualizar formas de pagamento baseado no extrato
    cJSON *transacao;
    cJSON_ArrayForEach(transacao, extrato_root) {
        cJSON *pagamento = cJSON_GetObjectItem(transacao, "pagamento");
        if (cJSON_IsObject(pagamento)) {
            cJSON *forma_pagamento = cJSON_GetObjectItem(pagamento, "forma_pagamento");
            cJSON *valor_pago = cJSON_GetObjectItem(pagamento, "valor_pago");

            if (cJSON_IsString(forma_pagamento) && cJSON_IsNumber(valor_pago)) {
                if (strcmp(forma_pagamento->valuestring, "DINHEIRO") == 0) {
                    cJSON *dinheiro = cJSON_GetObjectItem(formas_pagamento, "dinheiro");
                    if (cJSON_IsNumber(dinheiro)) {
                        dinheiro->valuedouble += valor_pago->valuedouble;
                    }
                } else if (strcmp(forma_pagamento->valuestring, "CARTÃO") == 0) {
                    cJSON *cartao = cJSON_GetObjectItem(formas_pagamento, "cartao");
                    if (cJSON_IsNumber(cartao)) {
                        cartao->valuedouble += valor_pago->valuedouble;
                    }
                } else if (strcmp(forma_pagamento->valuestring, "PIX") == 0) {
                    cJSON *pix = cJSON_GetObjectItem(formas_pagamento, "pix");
                    if (cJSON_IsNumber(pix)) {
                        pix->valuedouble += valor_pago->valuedouble;
                    }
                }
            }
        }
    }

    // Salvar o arquivo de caixa atualizado
    file = fopen(caixa_file_path, "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa para escrita!\n");
        cJSON_Delete(extrato_root);
        cJSON_Delete(caixa_root);
        return;
    }
    char *updated_data = cJSON_Print(caixa_root);
    fprintf(file, "%s", updated_data);
    fclose(file);

    cJSON_Delete(extrato_root);
    cJSON_Delete(caixa_root);
    free(updated_data);
    printf("Formas de pagamento atualizadas com sucesso!\n");
}

void resetar_totais_financeiros(const char *caixa_file_path) {
    FILE *file = fopen(caixa_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa!\n");
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
        fprintf(stderr, "Erro ao analisar JSON de caixa: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *resumo_financeiro = cJSON_GetObjectItem(root, "resumo_financeiro");
    if (cJSON_IsObject(resumo_financeiro)) {
        cJSON *total_receitas = cJSON_GetObjectItem(resumo_financeiro, "total_receitas");
        if (cJSON_IsNumber(total_receitas)) {
            total_receitas->valuedouble = 0.0;
        } else {
            cJSON_AddNumberToObject(resumo_financeiro, "total_receitas", 0.0);
        }

        cJSON *total_despesas = cJSON_GetObjectItem(resumo_financeiro, "total_despesas");
        if (cJSON_IsNumber(total_despesas)) {
            total_despesas->valuedouble = 0.0;
        } else {
            cJSON_AddNumberToObject(resumo_financeiro, "total_despesas", 0.0);
        }

        cJSON *formas_pagamento = cJSON_GetObjectItem(resumo_financeiro, "formas_pagamento");
        if (cJSON_IsObject(formas_pagamento)) {
            cJSON *dinheiro = cJSON_GetObjectItem(formas_pagamento, "dinheiro");
            if (cJSON_IsNumber(dinheiro)) {
                dinheiro->valuedouble = 0.0;
            } else {
                cJSON_AddNumberToObject(formas_pagamento, "dinheiro", 0.0);
            }

            cJSON *cartao = cJSON_GetObjectItem(formas_pagamento, "cartao");
            if (cJSON_IsNumber(cartao)) {
                cartao->valuedouble = 0.0;
            } else {
                cJSON_AddNumberToObject(formas_pagamento, "cartao", 0.0);
            }

            cJSON *pix = cJSON_GetObjectItem(formas_pagamento, "pix");
            if (cJSON_IsNumber(pix)) {
                pix->valuedouble = 0.0;
            } else {
                cJSON_AddNumberToObject(formas_pagamento, "pix", 0.0);
            }
        }
    } else {
        fprintf(stderr, "Erro ao acessar resumo financeiro no JSON de caixa\n");
        cJSON_Delete(root);
        return;
    }

    file = fopen(caixa_file_path, "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa para escrita!\n");
        cJSON_Delete(root);
        return;
    }
    char *updated_data = cJSON_Print(root);
    fprintf(file, "%s", updated_data);
    fclose(file);

    cJSON_Delete(root);
    free(updated_data);
    printf("Totais financeiros e formas de pagamento resetados com sucesso!\n");
}

// Função para calcular o total de quantidade disponível
int calcular_total_estoque(const char *estoque_file_path) {
    FILE *file = fopen("bd/estoque_list.json", "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de estoque!\n");
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc(length + 1);
    if (!data) {
        fprintf(stderr, "Erro ao alocar memória!\n");
        fclose(file);
        return 0;
    }
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        fprintf(stderr, "Erro ao analisar JSON de estoque: %s\n", cJSON_GetErrorPtr());
        return 0;
    }

    cJSON *produtos = cJSON_GetObjectItem(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON de estoque inválido\n");
        cJSON_Delete(root);
        return 0;
    }

    int total_estoque = 0;
    cJSON *produto;
    cJSON_ArrayForEach(produto, produtos) {
        cJSON *quantidade_disponivel = cJSON_GetObjectItem(produto, "quantidade_disponivel");
        if (cJSON_IsNumber(quantidade_disponivel)) {
            total_estoque += quantidade_disponivel->valueint;
        }
    }

    cJSON_Delete(root);
    return total_estoque;
}

void tesouraria() {
    int option;
    const char *estoque_file_path = "estoque.json";
    const char *caixa_file_path = "bd/caixa.json";

    int total_estoque = calcular_total_estoque(estoque_file_path);


    FILE *file = fopen(caixa_file_path, "r");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa!\n");
        return;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *data = malloc(length + 1);
    if (!data) {
        fprintf(stderr, "Erro ao alocar memória!\n");
        fclose(file);
        return;
    }
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(data);
    free(data);
    if (!root) {
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *empresa = cJSON_GetObjectItem(root, "empresa");
    cJSON *fluxo_caixa = cJSON_GetObjectItem(root, "fluxo_caixa");
    cJSON *resumo_financeiro = cJSON_GetObjectItem(root, "resumo_financeiro");
    cJSON *resumo_estoque = cJSON_GetObjectItem(root, "resumo_estoque");

    if (!empresa || !fluxo_caixa || !resumo_financeiro || !resumo_estoque) {
        fprintf(stderr, "Erro: JSON malformado!\n");
        cJSON_Delete(root);
        return;
    }

    // Atualiza a data do relatório com a data atual
    char data_atual[11];
    obter_data_atual(data_atual, sizeof(data_atual));
    cJSON *data_relatorio = cJSON_GetObjectItem(empresa, "data_relatorio");
    if (cJSON_IsString(data_relatorio)) {
        cJSON_SetValuestring(data_relatorio, data_atual);
    } else {
        cJSON_AddStringToObject(empresa, "data_relatorio", data_atual);
    }

    // Calcula o lucro líquido
    int total_receitas = cJSON_GetObjectItem(resumo_financeiro, "total_receitas")->valueint;
    int total_despesas = cJSON_GetObjectItem(resumo_financeiro, "total_despesas")->valueint;
    int lucro_liquido = total_receitas - total_despesas;

    // Atualiza o campo lucro_liquido no JSON
    cJSON *lucro_liquido_obj = cJSON_GetObjectItem(resumo_financeiro, "lucro_liquido");
    if (cJSON_IsNumber(lucro_liquido_obj)) {
        lucro_liquido_obj->valueint = lucro_liquido;
    } else {
        cJSON_AddNumberToObject(resumo_financeiro, "lucro_liquido", lucro_liquido);
    }

    // Calcula o saldo final
    int saldo_inicial = cJSON_GetObjectItem(fluxo_caixa, "saldo_inicial")->valueint;
    int saldo_final = saldo_inicial + total_receitas - total_despesas;
    cJSON *saldo_final_obj = cJSON_GetObjectItem(fluxo_caixa, "saldo_final");
    if (cJSON_IsNumber(saldo_final_obj)) {
        saldo_final_obj->valueint = saldo_final;
    } else {
        cJSON_AddNumberToObject(fluxo_caixa, "saldo_final", saldo_final);
    }

    // Atualiza o campo total no resumo_estoque
    cJSON *total_obj = cJSON_GetObjectItem(resumo_estoque, "total");
    if (cJSON_IsNumber(total_obj)) {
        total_obj->valueint = total_estoque;
    } else {
        cJSON_AddNumberToObject(resumo_estoque, "total", total_estoque);
    }
    printf("Resumo de estoque atualizado: %d\n", total_estoque);

    printf("\n|---------------------------------------------\n");
    printf("| Empresa:\n");
    printf("| Nome: %s\n", cJSON_GetObjectItem(empresa, "nome")->valuestring);
    printf("| CNPJ: %s\n", cJSON_GetObjectItem(empresa, "cnpj")->valuestring);
    printf("| Data do Relatório: %s\n", data_atual);

    printf("|---------------------------------------------\n");
    printf("\n| Fluxo de Caixa:\n");
    printf("| Saldo Inicial: %d\n", saldo_inicial);
    printf("| Saldo Final: %d\n", saldo_final);

    printf("|---------------------------------------------\n");
    printf("| Resumo Financeiro:\n");
    printf("| Total de Receitas: %d\n", total_receitas);
    printf("| Total de Despesas: %d\n", total_despesas);
    printf("| Lucro Líquido: %d\n", lucro_liquido);

    printf("|---------------------------------------------\n");
    cJSON *formas_pagamento = cJSON_GetObjectItem(resumo_financeiro, "formas_pagamento");
    printf("| Formas de Pagamento:\n");
    printf("| Dinheiro: %d\n", cJSON_GetObjectItem(formas_pagamento, "dinheiro")->valueint);
    printf("| Cartão: %d\n", cJSON_GetObjectItem(formas_pagamento, "cartao")->valueint);
    printf("| Pix: %d\n", cJSON_GetObjectItem(formas_pagamento, "pix")->valueint);

    printf("|---------------------------------------------\n");
    printf("| Resumo Estoque:\n");
    printf("| Total: %d\n", total_estoque);

    printf("|---------------------------------------------\n");
    printf("| Aperte 1 para sair: ");
    if (scanf("%d", &option) != 1) {
        printf("| Erro na leitura da opção!\n");
        cJSON_Delete(root);
        return;
    }

    if (option == 1) {
        resetar_totais_financeiros("bd/caixa.json"); // Chama a função para zerar os totais de receitas, despesas e formas de pagamento
        dashboard_adm_navigate();
    } else {
        printf("| Opção inválida!\n");
    }

    // Salva as mudanças de volta no arquivo JSON
    char *updated_data = cJSON_Print(root);
    if (!updated_data) {
        fprintf(stderr, "Erro ao converter JSON para string\n");
        cJSON_Delete(root);
        return;
    }

    file = fopen(caixa_file_path, "w");
    if (!file) {
        fprintf(stderr, "Não foi possível abrir o arquivo de caixa para escrita!\n");
        cJSON_Delete(root);
        free(updated_data);
        return;
    }
    fprintf(file, "%s", updated_data);
    fclose(file);

    cJSON_Delete(root);
    free(updated_data);
    printf("Totais financeiros e resumo de estoque atualizados com sucesso!\n");
}

void extratos(){
    FILE *file = fopen("bd/extrato.json", "r");
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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *item = cJSON_GetArrayItem(root, 0);

    printf("\n| Total: %d\n", cJSON_GetObjectItem(item, "total")->valueint);
    printf("| Método de Pagamento: %s\n", cJSON_GetObjectItem(item, "metodo_pagamento")->valuestring);
    printf("| Valor Pago: %d\n", cJSON_GetObjectItem(item, "valor_pago")->valueint);
    printf("| Troco: %d\n", cJSON_GetObjectItem(item, "troco")->valueint);

    cJSON *produto_vendido = cJSON_GetObjectItem(item, "produto_vendido");
    printf("| Produto Vendido:\n");
    for (int i = 0; i < cJSON_GetArraySize(produto_vendido); i++) {
        printf("| Produto ID: %d\n", cJSON_GetArrayItem(produto_vendido, i)->valueint);
    }
    system("pause");
    financeiro();
    cJSON_Delete(root);
}

void insight() {
    system("cls || clear");
    int back;

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
        fprintf(stderr, "Erro ao analisar JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    cJSON *produtos = cJSON_GetObjectItemCaseSensitive(root, "produtos");
    if (!cJSON_IsArray(produtos)) {
        fprintf(stderr, "Formato de JSON inválido\n");
        cJSON_Delete(root);
        return;
    }

    time_t now = time(NULL);
    struct tm tm_now = *localtime(&now);

    printf("Produtos e tempo restante para validade:\n");

    cJSON *produto = NULL;
    cJSON_ArrayForEach(produto, produtos) {
        cJSON *validade = cJSON_GetObjectItemCaseSensitive(produto, "validade");
        cJSON *nome = cJSON_GetObjectItemCaseSensitive(produto, "nome");

        if (cJSON_IsString(validade) && (validade->valuestring != NULL)) {
            struct tm tm_validade = {0};
            sscanf(validade->valuestring, "%d-%d-%d", &tm_validade.tm_year, &tm_validade.tm_mon, &tm_validade.tm_mday);
            tm_validade.tm_year -= 1900;  // Ajustar ano
            tm_validade.tm_mon -= 1;      // Ajustar mês de 0-11

            time_t validade_time = mktime(&tm_validade);

            double difference = difftime(validade_time, now) / (60 * 60 * 24);

            printf("| Nome: %s\n", nome->valuestring);
            printf("| Dias restantes para validade: %.0f dias\n", difference);
            printf("-----------------------------\n");

            printf("| Para sair digite 1: ");
            scanf("%d", &back);

            if(back == 1){
                financeiro();
            }else{
                printf("| Opção inválida!");
            }
        }
    }

    cJSON_Delete(root);
}

void atualizar_caixa_auto(){
    const char *estoque_file_path = "bd/estoque_list.json";
    const char *caixa_file_path = "bd/caixa.json";
    const char *extrato_file_path = "bd/extrato.json";

    double total_saidas = calcular_total_saidas(estoque_file_path);
    atualizar_caixa(caixa_file_path, total_saidas);

    double total_despesas = calcular_total_despesas(estoque_file_path);
    atualizar_despesas(caixa_file_path, total_despesas);

    atualizar_formas_pagamento(caixa_file_path, extrato_file_path);
    financeiro(printf("Caixa atualizado com sucesso!\n"));

    //calcular_total_estoque(estoque_file_path, caixa_file_path);
}

void financeiro(){
    int option;

    printf("| Controle financeiro do hortifruti\n");
    printf("|----------------------------------\n");
    printf("| Testouraria     - 1\n");
    printf("| Extratos        - 2\n");
    printf("| Insigths        - 3\n");
    printf("| Atualizar caixa - 4\n");
    printf("| Voltar - 5\n");
    printf("|----------------------------------\n");
    printf("| Escolha sua opção: ");
    scanf("%d", &option);

    switch (option){
        case 1:
            tesouraria();
            break;
        case 2:
            extratos();
            break;
        case 3:
            insight();
            break;
        case 4:
            atualizar_caixa_auto();
        case 5:
            dashboard_adm_navigate();
            break;
        default:
            printf("Opção invalida!\n");
            break;
    }
}
//-----------------------------------------------------------------Main----------------------------------------------------------

int main(){
    system("cls || clear");
    setlocale(LC_ALL, "Portuguese");
    setlocale(LC_NUMERIC, "C");
    int choice;

    printf(" ____________________________ \n");
    printf("|  Bem vindo ao Hortifruti!  |\n");
    printf("|----------------------------|\n");
    printf("| O que deseja fazer?        |\n");
    printf("|----------------------------|\n");
    printf("| Login colaborador - 1      |\n");
    printf("| Acesso Gerencial  - 2      |\n");
    printf("| Módulo de Caixa   - 3      |\n");
    printf("| Módulo de Pesagem - 4      |\n");
    printf("| Sair do programa  - 5      |\n");
    printf("|----------------------------|\n");
    printf("| Escolha uma das opções: ");
    scanf("%d", &choice);
    printf("\n\n");

    switch (choice){
        case 1:
            system ("cls || clear");
            login_c();
            break;
        case 2:
            system ("cls || clear");
            login_adm();
            break;
        case 3:
            fluxo_caixa();
            break;
        case 4:
            pesagem();
            break;
        case 5:
            exit(0);
        default:
            printf("Opção inválida!\n\n");
            main();
            break;
    }
    return 0;
}
