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

// Função para verificar a validade de uma expressão usando as regras de inferência
bool checkValidity(Expression *premises[], int numPremises, Expression *conclusion);

// Função auxiliar para verificar se duas expressões são idênticas
bool areExpressionsEqual(Expression *expr1, Expression *expr2);

int main() {
    printf("Digite as sentenças das premissas, uma por linha (digite uma linha em branco para terminar):\n");

    // Lendo as premissas
    Expression *premises[100]; // Array para armazenar as premissas
    int numPremises = 0;
    char input[100];
    while (true) {
        fgets(input, sizeof(input), stdin);
        if (input[0] == '\n') break; // Se a linha estiver em branco, termina a leitura das premissas
        premises[numPremises++] = createExpression(input);
    }

    printf("Digite a sentença da conclusão:\n");

    // Lendo a conclusão
    fgets(input, sizeof(input), stdin);
    Expression *conclusion = createExpression(input);

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
    // Se a string contiver um operador binário, como AND, OR ou IMPLIES
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
    // Aplica as regras de inferência
    for (int i = 0; i < numPremises; i++) {
        Expression *premise = premises[i];
        // Verifica se a premissa implica diretamente a conclusão
        if (areExpressionsEqual(premise, conclusion)) {
            return true;
        }
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
