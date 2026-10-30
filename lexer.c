#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID_LEN 8
#define MAX_SIMBOLURI 500
#define MAX_FIP 1000

/* 1. TABELA DE CODIFICARE A ATOMILOR */
const char *ATOMI[] = {
    "IDENTIFICATOR", "CONSTANTA", // Cod 0 si Cod 1
    // Cuvinte rezervate (2 - 12)
    "int", "float", "char", "struct", "if", "else", "while", "read", "print", "return", "main",
    // Operatori (13 - 28)
    "+", "-", "*", "/", "%", "^", "=", "<<", ">>", "<", "<=", "==", "!=", ">=", ">",
    // Separatori (29 - 35)
    "{", "}", "(", ")", "[", "]", ";"
};
const int NR_ATOMI = sizeof(ATOMI) / sizeof(ATOMI[0]);

/* 2. TABELA DE SIMBOLURI - Unica (2.a), Arbore Binar Ordonat Lexicografic (3.b) */
typedef struct {
    char atom[64];
    int stanga;  // index fiu stang (-1 daca nu are)
    int dreapta; // index fiu drept (-1 daca nu are)
} NodTS;

NodTS TS[MAX_SIMBOLURI];
int nrTS = 0;

int insereazaTS(const char *simbol) {
    if (nrTS == 0) {
        strcpy(TS[0].atom, simbol);
        TS[0].stanga = TS[0].dreapta = -1;
        return nrTS++;
    }

    int curent = 0;
    while (1) {
        int cmp = strcmp(simbol, TS[curent].atom);
        if (cmp == 0) return curent; // Exista deja in TS

        if (cmp < 0) {
            if (TS[curent].stanga == -1) {
                TS[curent].stanga = nrTS;
                break;
            }
            curent = TS[curent].stanga;
        } else {
            if (TS[curent].dreapta == -1) {
                TS[curent].dreapta = nrTS;
                break;
            }
            curent = TS[curent].dreapta;
        }
    }

    strcpy(TS[nrTS].atom, simbol);
    TS[nrTS].stanga = TS[nrTS].dreapta = -1;
    return nrTS++;
}

/* 3. FORMA INTERNA A PROGRAMULUI (FIP) */
typedef struct {
    int codAtom;
    int pozTS;
    char lexem[64];
} ElemFIP;

ElemFIP FIP[MAX_FIP];
int nrFIP = 0;

void adaugaFIP(int cod, int pozTS, const char *lexem) {
    FIP[nrFIP].codAtom = cod;
    FIP[nrFIP].pozTS = pozTS;
    strcpy(FIP[nrFIP].lexem, lexem);
    nrFIP++;
}

/* 4. FUNCTII DE VALIDARE */
int cautaCodAtom(const char *s) {
    for (int i = 2; i < NR_ATOMI; i++)
        if (strcmp(ATOMI[i], s) == 0) return i;
    return -1;
}

int esteConstantaInt(const char *s) {
    int i = 0;
    if (s[0] == '+' || s[0] == '-') i = 1;
    if (!isdigit((unsigned char)s[i])) return 0;
    for (; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

int esteIdentificator(const char *s) {
    if (!isalpha((unsigned char)s[0])) return 0; // Incepe obligatoriu cu litera
    for (int i = 1; s[i]; i++)
        if (!isalnum((unsigned char)s[i]) && s[i] != '_') return 0;
    return 1;
}

/* 5. ANALIZORUL LEXICAL */
int main(int argc, char *argv[]) {
    const char *numeFisier = (argc > 1) ? argv[1] : "prog1.txt";
    FILE *f = fopen(numeFisier, "r");
    if (!f) {
        printf("Eroare: Nu pot deschide fisierul '%s'!\n", numeFisier);
        return 1;
    }

    char linie[256];
    int nrLinie = 0, erori = 0;

    while (fgets(linie, sizeof(linie), f)) {
        nrLinie++;
        for (int i = 0; linie[i]; ) {
            if (isspace((unsigned char)linie[i])) { i++; continue; }

            char token[64] = {0};
            int k = 0;

            // 1. Constante char ('a') sau string ("abc")
            if (linie[i] == '\'' || linie[i] == '"') {
                char sep = linie[i];
                token[k++] = linie[i++];
                while (linie[i] && linie[i] != sep && linie[i] != '\n')
                    token[k++] = linie[i++];
                if (linie[i] == sep) {
                    token[k++] = linie[i++];
                    token[k] = '\0';
                    adaugaFIP(1, insereazaTS(token), token);
                } else {
                    printf("Eroare lexicala la linia %d: constanta caracter/string neinchisa: %s\n", nrLinie, token);
                    erori++;
                }
                continue;
            }

            // 2. Cuvinte rezervate, Identificatori sau Numere
            if (isalnum((unsigned char)linie[i]) || linie[i] == '_') {
                while (isalnum((unsigned char)linie[i]) || linie[i] == '_')
                    token[k++] = linie[i++];
            } else {
                // 3. Operatori si Separatori
                token[k++] = linie[i++];
                // Verificam operatorii de 2 caractere: <<, >>, <=, >=, ==, !=
                if (linie[i] && strchr("=<>", linie[i])) {
                    char op2[3] = {token[0], linie[i], '\0'};
                    if (cautaCodAtom(op2) != -1) token[k++] = linie[i++];
                }
            }
            token[k] = '\0';

            // Clasificare token
            int cod = cautaCodAtom(token);
            if (cod != -1) {
                adaugaFIP(cod, -1, token);
            } else if (esteConstantaInt(token)) {
                adaugaFIP(1, insereazaTS(token), token);
            } else if (esteIdentificator(token)) {
                if (strlen(token) > MAX_ID_LEN) {
                    printf("Eroare lexicala la linia %d: identificatorul '%s' depaseste 8 caractere!\n", nrLinie, token);
                    erori++;
                } else {
                    adaugaFIP(0, insereazaTS(token), token);
                }
            } else {
                printf("Eroare lexicala la linia %d: atom necunoscut sau invalid '%s'!\n", nrLinie, token);
                erori++;
            }
        }
    }
    fclose(f);

    printf("\n=== TABELA DE SIMBOLURI (Arbore Binar Ordonat Lexicografic) ===\n");
    printf("Poz | Simbol       | Stanga | Dreapta\n");
    printf("-------------------------------------\n");
    for (int i = 0; i < nrTS; i++)
        printf("%-3d | %-12s | %-6d | %-7d\n", i, TS[i].atom, TS[i].stanga, TS[i].dreapta);

    printf("\n=== FORMA INTERNA A PROGRAMULUI (FIP) ===\n");
    printf("Cod Atom | Pozitie TS | Atom\n");
    printf("-------------------------------------\n");
    for (int i = 0; i < nrFIP; i++)
        printf("%-8d | %-10d | %s\n", FIP[i].codAtom, FIP[i].pozTS, FIP[i].lexem);

    return 0;
}
