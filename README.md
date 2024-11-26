# Documentação do Sistema

# **Introdução**

Este documento apresenta a estrutura de um sistema feito para um hortifruti, onde possui seus respectivos módulos, sendo esses:

* Módulo de servidor (Gerenciamento de todos os outros módulos, raiz principal do projeto)  
* Módulo de caixa (Consiste em um registrador de vendas)  
* Módulo de pesagem (Registra o peso e o preço do produto em uma etiqueta que poderá ser paga posteriormente)

## **Instalação**

Os códigos fontes e demais pastas necessárias para a completa utilização do software está no github (plataforma de hospedagem de código fonte), do integrante responsável pelo desenvolvimento do software.

* link para o repositório: [https://github.com/johnatas-eugeniob/PIM---Software.git](https://github.com/johnatas-eugeniob/PIM---Software.git)

Para seguir com a instalação é necessário acessar o repositório e escolher realizar o download da pasta .rar do projeto. A partir deste ponto e com sua IDE devidamente instalada e pronta, é possível compilar e utilizar o software.

# **Biblioteca**

Na pasta gerada após extrair a pasta .rar baixada da internet, pode ser vista uma pasta chamada “lib”, essa pasta com a única biblioteca que não é nativa da linguagem C, a biblioteca cJSON. A biblioteca é representada por dois arquivos que estão localizados nessa pasta, sendo eles: “cJSON.c” e “cJSON.h". Esses arquivos serão referenciados na nossa main para que possamos usufruir das funcionalidades presentes na biblioteca.

# **Main**

O módulo principal controla o fluxo do sistema, incluindo funcionalidades administrativas como controles de colaboradores, funcionários, produtos e fornecedores.

Utilizando a biblioteca cJSON mencionada anteriormente, é possível acompanhá-la ao longo de todo o conteúdo, já que, a estrutura onde é guardada as informações e dados é composta 100% por arquivos com a extensão JSON, uma vez que é uma extensão que possui as características bem próximas a um banco de dados, por conta disso decidimos utilizá-la.

### **Gerenciamento de pessoas e produtos**

Para o gerenciamento de pessoas e produtos o software segue uma estrutura CRUD, que consiste em Create (Criar), Read (ler/visualizar), Update (Atualizar)e Delete (Deletar).

Exemplo de código referente a **CRUD** com fornecedor:  
*cJSON \*new\_forn \= cJSON\_CreateObject();*  
*cJSON\_AddItemToObject(new\_forn, "id", cJSON\_CreateNumber(next\_id));*  
*cJSON\_AddItemToObject(new\_forn, "username", cJSON\_CreateString(username));*  
*cJSON\_AddItemToObject(new\_forn, "cpf", cJSON\_CreateNumber(cpf));*  
*cJSON\_AddItemToObject(new\_forn, "endereco", cJSON\_CreateString(addres));*  
*cJSON\_AddItemToObject(new\_forn, "telefone", cJSON\_CreateNumber(telefone));*  
*cJSON\_AddItemToObject(new\_forn, "email", cJSON\_CreateString(email));*  
*cJSON\_AddItemToArray(cad\_forn, new\_forn);*  
        

Esta estrutura e formas semelhantes a essa serão encontradas ao longo do software.

### **Atualizações de conteúdo no caixa**

Parte importante para a visualização do setor financeiro, pois tem as principais funções responsáveis por atualizar os números do documento em tempo real no momento em que é solicitado a atualização do caixa, disponível no menu gerencial \-\> controle financeiro \-\> atualizar caixa e tesouraria.

### **Extratos e insights**

Ainda no setor financeiro é possível verificarmos todos os extratos de compras feitas contendo o identificador do extrato, o item comprado, o valor e o método de pagamento.

Para os insights (compreensão/entendimento) nós basicamente levamos ao entendimento e análise do gerente a validade das levas de todos os produtos cadastrados no sistema, visando solucionar um das principais problemáticas de hortifrutis em geral que consiste no desperdício de produtos que estão próximos ou já passaram da validade.

# **Pesagem**

O módulo de pesagem gerencia a leitura, cálculo e registro de pesagens para produtos. No módulo de pesagem, essas informações são pegas e estruturadas em um arquivo JSON chamado “etiqueta.json” localizado na pasta “bd”, como todos os outros arquivos json que de alguma forma servem como algo semelhante a um banco de dados, especificamente armazenagem de informações para serem manipuladas posteriormente.

### **Registro de Pesagem**

Permite registrar a pesagem de um produto, calcular o preço total e gerar uma nova etiqueta.

Exemplo de código:  
*cJSON \*nova\_etiqueta \= cJSON\_CreateObject();*  
*cJSON\_AddNumberToObject(nova\_etiqueta, "id\_etiqueta", new\_etiqueta\_id);*  
*cJSON\_AddNumberToObject(nova\_etiqueta, "id\_produto", id);*  
*cJSON\_AddStringToObject(nova\_etiqueta, "produto", cJSON\_GetObjectItemCaseSensitive(prod, "nome")-\>valuestring);*  
*cJSON\_AddNumberToObject(nova\_etiqueta, "peso", qtd);*  
*cJSON\_AddNumberToObject(nova\_etiqueta, "preco\_calculado", total);*  
*cJSON\_AddStringToObject(nova\_etiqueta, "data", data\_etiqueta);*  
*cJSON\_AddBoolToObject(nova\_etiqueta, "validada", false);*  
*cJSON\_AddItemToArray(etiquetas, nova\_etiqueta);*

### **Carregamento de Dados JSON**

Os dados de produtos e etiquetas são carregados a partir de arquivos JSON para permitir sua manipulação dentro do programa.

Exemplo de código:

*FILE \*file \= fopen("bd/estoque\_list.json", "r");*  
*if (\!file) {*  
    *fprintf(stderr, "Não foi possível abrir o arquivo de produtos\!*  
*");*  
    *return;*  
*}*  
*fseek(file, 0, SEEK\_END);*  
*long length \= ftell(file);*  
*fseek(file, 0, SEEK\_SET);*  
*char \*data \= malloc(length \+ 1);*  
*fread(data, 1, length, file);*  
*data\[length\] \= '\\0';*  
*fclose(file);*

Este bloco de código e semelhantes podem ser encontrados com facilidade na documentação da biblioteca, precisamente em “fuzzing \-\> afl.c” (local no repositório da biblioteca no github). Não só neste local mas também espalhados por todo o repositório.        

## **Caixa e Finalização de Venda**

O módulo de caixa é responsável por centralizar as vendas e consequentemente oferecer o métodos de pagamento, gerenciamento do troco, calcular o valor total de todos os produtos inseridos, e posteriormente, registrar no meu arquivo “estoque.json” uma saída para o produto diminuindo a quantidade total deste produto no estoque.

**Exemplo de código:**

*switch (choice\_pagamento) {*  
    *case 1:*  
        *strcpy(forma\_pagamento, "Crédito/Débito");*  
        *break;*  
    *case 2:*  
        *strcpy(forma\_pagamento, "PIX");*  
        *break;*  
    *case 3:*  
        *strcpy(forma\_pagamento, "Dinheiro");*  
        *printf("| Valor pago: ");*  
        *scanf("%f", \&valor\_pago);*  
        *troco \= valor\_pago \- total;*  
        *break;*  
*}*


### **Atualização de Estoque**

A cada venda, o estoque é atualizado para refletir a quantidade restante de produtos.

Exemplo de código:

*cJSON \*quantidade\_disponivel \= cJSON\_GetObjectItemCaseSensitive(produto, "quantidade\_disponivel");*  
*if (cJSON\_IsNumber(quantidade\_disponivel)) {*  
    *quantidade\_disponivel-\>valueint \-= cJSON\_GetObjectItemCaseSensitive(etiqueta, "peso")-\>valueint;*  
*}*

# **Conclusão**

A partir dos itens apresentados acima, podemos compreender as funcionalidades do sistema desenvolvido e utilizá-lo da melhor forma para que através deste seus usuários finais possam tirar o maior proveito e eficiência de seus respectivos processos.

