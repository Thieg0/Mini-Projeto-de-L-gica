#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Enumeração para os tipos de expressão lógica
typedef enum {
    SYMBOL,     // Símbolo proposicional (P, Q, R, ...)
    AND,        // Conjunção (P & Q)
    OR,         // Disjunção (P v Q)
    IMPLIES,    // Implicação (P -> Q)
    NOT         // Negação (~P)
} ExprType;

// Estrutura para representar uma expressão lógica
typedef struct Expression {
    ExprType type;
    char symbol; // Somente utilizado se type == SYMBOL
    struct Expression *left; // Subexpressão esquerda
    struct Expression *right; // Subexpressão direita
} Expression;

// Função para criar uma expressão lógica a partir de uma string
Expression *createExpression(char *str);

// Funções para aplicar as regras de inferência
Expression *modusPonens(Expression *premise, Expression *conclusion);
Expression *modusTollens(Expression *premise, Expression *conclusion);
Expression *silogismoHipotetico(Expression *premise1, Expression *premise2);
Expression *silogismoDisjuntivo(Expression *premise1, Expression *premise2);
Expression *introducaoDoAnd(Expression *expr1, Expression *expr2);
Expression *eliminacaoDoAnd(Expression *expr);

// Funções para aplicar as regras de equivalência
Expression *applyEquivalences(Expression *expr);

// Função para verificar a validade de uma expressão usando as regras de inferência
bool checkValidity(Expression *premises[], int numPremises, Expression *conclusion);

// Função auxiliar para verificar se duas expressões são idênticas
bool areExpressionsEqual(Expression *expr1, Expression *expr2);

// Função para traduzir expressões em linguagem natural para expressões em linguagem simbólica
char *translateToSymbolic(char *naturalLanguage);

// Função para mostrar a tradução da linguagem natural para a linguagem lógica
void showTranslation(char *naturalLanguage, char *symbolic);

int main() {
    printf("Digite as sentenças das premissas, uma por linha (digite uma linha em branco para terminar):\n");

    // Lendo as premissas
    Expression *premises[100]; // Array para armazenar as premissas
    int numPremises = 0;
    char input[100];
    while (true) {
        fgets(input, sizeof(input), stdin);
        if (input[0] == '\n') break; // Se a linha estiver em branco, termina a leitura das premissas
        char symbolic[100];
        char *translated = translateToSymbolic(input);
        showTranslation(input, translated);
        premises[numPremises++] = createExpression(translated);
    }

    printf("Digite a sentença da conclusão:\n");

    // Lendo a conclusão
    fgets(input, sizeof(input), stdin);
    char symbolic[100];
    char *translated = translateToSymbolic(input);
    showTranslation(input, translated);
    Expression *conclusion = createExpression(translated);

    // Simplifica as premissas e a conclusão usando regras de equivalência
    for (int i = 0; i < numPremises; i++) {
        premises[i] = applyEquivalences(premises[i]);
    }
    conclusion = applyEquivalences(conclusion);

    // Verificando a validade do argumento
    if (checkValidity(premises, numPremises, conclusion)) {
        printf("O argumento é válido!\n");
    } else {
        printf("O argumento não é válido.\n");
    }

    // Liberando a memória alocada para as expressões
    for (int i = 0; i < numPremises; i++) {
        free(premises[i]);
    }
    free(conclusion);

    return 0;
}

Expression *createExpression(char *str) {
    Expression *expr = malloc(sizeof(Expression));
    // Se a string contiver apenas um símbolo, trata-se de uma folha
    if (strlen(str) == 2) {
        expr->type = SYMBOL;
        expr->symbol = str[0];
        expr->left = NULL;
        expr->right = NULL;
        return expr;
    }
    // Remove o caractere de nova linha da string
    str[strcspn(str, "\n")] = 0;
    // Se a string contiver um operador binário, como AND, OR, IMPLIES ou NOT
    if (strstr(str, "&") != NULL) {
        expr->type = AND;
    } else if (strstr(str, "v") != NULL) {
        expr->type = OR;
    } else if (strstr(str, "->") != NULL) {
        expr->type = IMPLIES;
    } else if (strstr(str, "~") != NULL) {
        expr->type = NOT;
    }
    // Divide a string em duas partes, dependendo do operador encontrado
    char *token;
    if (expr->type == AND || expr->type == OR || expr->type == IMPLIES || expr->type == NOT) {
        if (expr->type == NOT) {
            token = strtok(str, "~");
            expr->right = createExpression(token);
        } else {
            token = strtok(str, "&v->~");
            expr->left = createExpression(token);
            token = strtok(NULL, "&v->~");
            expr->right = createExpression(token);
        }
    }
    return expr;
}

bool checkValidity(Expression *premises[], int numPremises, Expression *conclusion) {
    // Verifica se cada premissa implica diretamente a conclusão
    for (int i = 0; i < numPremises; i++) {
        Expression *premise = premises[i];
        // Verifica se a premissa implica diretamente a conclusão
        if (areExpressionsEqual(premise, conclusion)) {
            return true;
        }
    }

    // Verifica se é possível inferir a conclusão a partir das premissas usando regras de inferência
    for (int i = 0; i < numPremises; i++) {
        Expression *premise = premises[i];
        // Tenta aplicar as regras de inferência
        Expression *newExpr = NULL;
        switch (premise->type) {
            case IMPLIES:
                newExpr = modusPonens(premise, conclusion);
                if (newExpr != NULL) return true;
                newExpr = modusTollens(premise, conclusion);
                if (newExpr != NULL) return true;
                break;
            case AND:
                newExpr = eliminacaoDoAnd(premise);
                if (newExpr != NULL && areExpressionsEqual(newExpr, conclusion)) return true;
                break;
            case OR:
                // Para o silogismo disjuntivo, precisamos verificar todas as premissas
                for (int j = 0; j < numPremises; j++) {
                    Expression *otherPremise = premises[j];
                    if (otherPremise != premise) {
                        newExpr = silogismoDisjuntivo(premise, otherPremise);
                        if (newExpr != NULL && areExpressionsEqual(newExpr, conclusion)) return true;
                    }
                }
                break;
            default:
                break;
        }
    }
    
    // Se não foi possível inferir a conclusão, retorna falso
    return false;
}

Expression *modusPonens(Expression *premise, Expression *conclusion) {
    if (premise->type == IMPLIES && conclusion->type == SYMBOL &&
        areExpressionsEqual(premise->right, conclusion)) {
        return premise->left;
    }
    return NULL;
}

Expression *modusTollens(Expression *premise, Expression *conclusion) {
    if (premise->type == IMPLIES && conclusion->type == NOT &&
        areExpressionsEqual(premise->left, conclusion->right)) {
        return createExpression("~");
    }
    return NULL;
}

Expression *silogismoHipotetico(Expression *premise1, Expression *premise2) {
    if (premise1->type == IMPLIES && premise2->type == IMPLIES &&
        areExpressionsEqual(premise1->right, premise2->left)) {
        return createExpression(premise1->left->symbol == premise2->right->symbol ? premise1->right : "~");
    }
    return NULL;
}

Expression *silogismoDisjuntivo(Expression *premise1, Expression *premise2) {
    if (premise1->type == OR && premise2->type == NOT &&
        areExpressionsEqual(premise1->left, premise2->right)) {
        return premise1->right;
    } else if (premise1->type == OR && premise2->type == NOT &&
               areExpressionsEqual(premise1->right, premise2->right)) {
        return premise1->left;
    }
    return NULL;
}

Expression *introducaoDoAnd(Expression *expr1, Expression *expr2) {
    return createExpression("&");
}

Expression *eliminacaoDoAnd(Expression *expr) {
    if (expr->type == AND) {
        return expr->left;
    }
    return NULL;
}

bool areExpressionsEqual(Expression *expr1, Expression *expr2) {
    if (expr1 == NULL && expr2 == NULL) {
        return true;
    }
    if (expr1 == NULL || expr2 == NULL) {
        return false;
    }
    if (expr1->type != expr2->type) {
        return false;
    }
    if (expr1->type == SYMBOL && expr2->type == SYMBOL && expr1->symbol == expr2->symbol) {
        return true;
    }
    return areExpressionsEqual(expr1->left, expr2->left) && areExpressionsEqual(expr1->right, expr2->right);
}

Expression *applyEquivalences(Expression *expr) {
    // Aplica as regras de equivalência
    switch (expr->type) {
        // Equivalência Básica
        case AND:
        case OR:
            // Comutatividade
            if (expr->left->type == SYMBOL && expr->right->type == SYMBOL) {
                if (expr->left->symbol > expr->right->symbol) {
                    Expression *temp = expr->left;
                    expr->left = expr->right;
                    expr->right = temp;
                }
            }
            // Associatividade
            if (expr->left->type == expr->type) {
                Expression *temp = expr->left->right;
                expr->left->right = expr->right;
                expr->right = temp;
            }
            break;
        case IMPLIES:
            // Transposição
            if (expr->right->type == NOT) {
                Expression *temp = expr->left;
                expr->left = expr->right->right;
                expr->right->right = temp;
                expr->type = OR;
                expr->right->type = NOT;
            }
            break;
        case NOT:
            // Dupla Negação
            if (expr->right->type == NOT) {
                Expression *temp = expr->right->right;
                free(expr->right);
                expr->type = SYMBOL;
                expr->symbol = temp->symbol;
                free(temp);
            }
            break;
        default:
            break;
    }
    return expr;
}

char *translateToSymbolic(char *naturalLanguage) {
    char *symbolic = (char *)malloc(sizeof(char) * 100);

    // Inicializa a string simbólica como vazia
    strcpy(symbolic, "");

    // Lista de regras de tradução
    char *translations[][2] = {
        {" então ", " -> "},
        {" e ", " & "},
        {" ou ", " v "},
        {" não é verdade que ", "~"},
        // Mais regras podem ser adicionadas aqui conforme necessário
        {NULL, NULL} // Marca de fim da lista
    };

    // Aplica as regras de tradução
    for (int i = 0; translations[i][0] != NULL; i++) {
        char *match = strstr(naturalLanguage, translations[i][0]);
        while (match != NULL) {
            // Substitui a correspondência pela tradução
            strncpy(symbolic + strlen(symbolic), naturalLanguage, match - naturalLanguage);
            strcpy(symbolic + strlen(symbolic), translations[i][1]);
            naturalLanguage = match + strlen(translations[i][0]);
            match = strstr(naturalLanguage, translations[i][0]);
        }
    }

    // Copia o restante da string naturalLanguage para symbolic
    strcpy(symbolic + strlen(symbolic), naturalLanguage);

    return symbolic;
}

void showTranslation(char *naturalLanguage, char *symbolic) {
    printf("Para a linguagem lógica: %s", symbolic);
}
