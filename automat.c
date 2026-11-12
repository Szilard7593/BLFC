#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_STATES 64
#define MAX_ALPHABET 128
#define MAX_TRANSITIONS 2048
#define MAX_NAME_LEN 32
#define LINE_BUF 1024

typedef struct {
    char from[MAX_NAME_LEN];
    char symbol;
    char to[MAX_NAME_LEN];
} Tranzitie;

typedef struct {
    char stari[MAX_STATES][MAX_NAME_LEN];
    int nr_stari;

    char alfabet[MAX_ALPHABET];
    int nr_alfabet;

    char stare_initiala[MAX_NAME_LEN];

    char stari_finale[MAX_STATES][MAX_NAME_LEN];
    int nr_stari_finale;

    Tranzitie tranzitii[MAX_TRANSITIONS];
    int nr_tranzitii;
} Automat;

void curata_linie(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}

bool exista_simbol(const Automat *fa, char c) {
    for (int i = 0; i < fa->nr_alfabet; i++) {
        if (fa->alfabet[i] == c) return true;
    }
    return false;
}

/* Citeste elementele automatului finit din fisier:
 * Linia 1: Stari separate prin spatiu
 * Linia 2: Alfabetul (simboluri)
 * Linia 3: Starea initiala
 * Linia 4: Starile finale separate prin spatiu
 * Liniile 5+: Tranzitii de forma: <stare_sursa> <stare_destinatie> <sir_simboluri>
 */
bool citeste_automat(Automat *fa, const char *nume_fisier) {
    FILE *f = fopen(nume_fisier, "r");
    if (!f) {
        printf("Eroare: Nu pot deschide fisierul '%s'!\n", nume_fisier);
        return false;
    }

    memset(fa, 0, sizeof(Automat));
    char linie[LINE_BUF];

    //  Multimea starilor
    if (fgets(linie, sizeof(linie), f)) {
        curata_linie(linie);
        char *tok = strtok(linie, " ");
        while (tok && fa->nr_stari < MAX_STATES) {
            strncpy(fa->stari[fa->nr_stari++], tok, MAX_NAME_LEN - 1);
            tok = strtok(NULL, " ");
        }
    }

    // Alfabetul
    if (fgets(linie, sizeof(linie), f)) {
        curata_linie(linie);
        for (int i = 0; linie[i] != '\0'; i++) {
            if (linie[i] != ' ' && !exista_simbol(fa, linie[i]) && fa->nr_alfabet < MAX_ALPHABET) {
                fa->alfabet[fa->nr_alfabet++] = linie[i];
            }
        }
    }

    //  Starea initiala
    if (fgets(linie, sizeof(linie), f)) {
        curata_linie(linie);
        char *tok = strtok(linie, " ");
        if (tok) {
            strncpy(fa->stare_initiala, tok, MAX_NAME_LEN - 1);
        }
    }

    // Multimea starilor finale
    if (fgets(linie, sizeof(linie), f)) {
        curata_linie(linie);
        char *tok = strtok(linie, " ");
        while (tok && fa->nr_stari_finale < MAX_STATES) {
            strncpy(fa->stari_finale[fa->nr_stari_finale++], tok, MAX_NAME_LEN - 1);
            tok = strtok(NULL, " ");
        }
    }

    // Tranzitiile
    while (fgets(linie, sizeof(linie), f)) {
        curata_linie(linie);
        if (strlen(linie) == 0) continue;

        char *from = strtok(linie, " ");
        char *to = strtok(NULL, " ");
        char *simboluri = strtok(NULL, " ");

        if (from && to && simboluri) {
            for (int i = 0; simboluri[i] != '\0'; i++) {
                if (fa->nr_tranzitii < MAX_TRANSITIONS) {
                    strncpy(fa->tranzitii[fa->nr_tranzitii].from, from, MAX_NAME_LEN - 1);
                    strncpy(fa->tranzitii[fa->nr_tranzitii].to, to, MAX_NAME_LEN - 1);
                    fa->tranzitii[fa->nr_tranzitii].symbol = simboluri[i];
                    fa->nr_tranzitii++;
                }
            }
        }
    }

    fclose(f);
    return true;
}

// Afisarea elementelor automatului 
void afiseaza_stari(const Automat *fa) {
    printf("Multimea starilor Q = { ");
    for (int i = 0; i < fa->nr_stari; i++) {
        printf("%s%s", fa->stari[i], (i < fa->nr_stari - 1) ? ", " : " ");
    }
    printf("}\n");
}

void afiseaza_alfabet(const Automat *fa) {
    printf("Alfabetul Sigma = { ");
    for (int i = 0; i < fa->nr_alfabet; i++) {
        printf("'%c'%s", fa->alfabet[i], (i < fa->nr_alfabet - 1) ? ", " : " ");
    }
    printf("}\n");
}

void afiseaza_tranzitii(const Automat *fa) {
    printf("Tranzitiile (delta) [%d tranzitii individuale]:\n", fa->nr_tranzitii);
    for (int i = 0; i < fa->nr_tranzitii; i++) {
        printf("  delta(%s, '%c') = %s\n",
               fa->tranzitii[i].from,
               fa->tranzitii[i].symbol,
               fa->tranzitii[i].to);
    }
}

void afiseaza_stari_finale(const Automat *fa) {
    printf("Starea initiala q0 = %s\n", fa->stare_initiala);
    printf("Multimea starilor finale F = { ");
    for (int i = 0; i < fa->nr_stari_finale; i++) {
        printf("%s%s", fa->stari_finale[i], (i < fa->nr_stari_finale - 1) ? ", " : " ");
    }
    printf("}\n");
}

//  Pentru o stare si un simbol, returneaza starea in care se ajunge 
const char* stare_urmatoare(const Automat *fa, const char *stare_curenta, char simbol) {
    const char *gasit = NULL;
    int nr_gasite = 0;
    for (int i = 0; i < fa->nr_tranzitii; i++) {
        if (strcmp(fa->tranzitii[i].from, stare_curenta) == 0 &&
            fa->tranzitii[i].symbol == simbol) {
            gasit = fa->tranzitii[i].to;
            nr_gasite++;
        }
    }
    if (nr_gasite > 1) {
        printf("[Atentie] Automatul este nedeterminist pentru (%s, '%c')!\n", stare_curenta, simbol);
    }
    return gasit;
}

bool este_stare_finala(const Automat *fa, const char *stare) {
    for (int i = 0; i < fa->nr_stari_finale; i++) {
        if (strcmp(fa->stari_finale[i], stare) == 0) return true;
    }
    return false;
}

//  Verifica acceptarea unei secvente 
bool verifica_secventa(const Automat *fa, const char *secventa) {
    char stare_curenta[MAX_NAME_LEN];
    strncpy(stare_curenta, fa->stare_initiala, MAX_NAME_LEN - 1);
    stare_curenta[MAX_NAME_LEN - 1] = '\0';

    for (int i = 0; secventa[i] != '\0'; i++) {
        const char *urm = stare_urmatoare(fa, stare_curenta, secventa[i]);
        if (!urm) {
            return false; // Blocaj: nu exista tranzitie definita
        }
        strncpy(stare_curenta, urm, MAX_NAME_LEN - 1);
        stare_curenta[MAX_NAME_LEN - 1] = '\0';
    }

    return este_stare_finala(fa, stare_curenta);
}

int main(int argc, char *argv[]) {
    const char *fisier = (argc > 1) ? argv[1] : "automat_id.txt";
    Automat fa;

    if (!citeste_automat(&fa, fisier)) {
        return 1;
    }
    printf("Automatul din '%s' a fost incarcat cu succes!\n", fisier);

    int optiune;
    char stare[MAX_NAME_LEN], simbol_str[16], secventa[256], fisier_nou[128];

    do {
        printf("\nMENIU AUTOMAT FINIT \n");
        printf("1. Afiseaza multimea starilor (Q)\n");
        printf("2. Afiseaza alfabetul (Sigma)\n");
        printf("3. Afiseaza tranzitiile (delta)\n");
        printf("4. Afiseaza starea initiala si starile finale (F)\n");
        printf("5. Verifica acceptarea unei secvente\n");
        printf("6. Stare urmatoare pentru delta(stare, simbol)\n");
        printf("7. Incarca alt fisier de automat\n");
        printf("0. Iesire\n");
        printf("Optiune: ");
        if (scanf("%d", &optiune) != 1) break;

        switch (optiune) {
            case 1:
                afiseaza_stari(&fa);
                break;
            case 2:
                afiseaza_alfabet(&fa);
                break;
            case 3:
                afiseaza_tranzitii(&fa);
                break;
            case 4:
                afiseaza_stari_finale(&fa);
                break;
            case 5:
                printf("Introduceti secventa: ");
                scanf("%255s", secventa);
                if (verifica_secventa(&fa, secventa)) {
                    printf("-> Secventa '%s' ESTE ACCEPTATA!\n", secventa);
                } else {
                    printf("-> Secventa '%s' NU este acceptata!\n", secventa);
                }
                break;
            case 6:
                printf("Introduceti starea curenta: ");
                scanf("%31s", stare);
                printf("Introduceti simbolul: ");
                scanf("%15s", simbol_str);
                const char *urm = stare_urmatoare(&fa, stare, simbol_str[0]);
                if (urm) {
                    printf("-> delta(%s, '%c') = %s\n", stare, simbol_str[0], urm);
                } else {
                    printf("-> Nu exista tranzitie din starea '%s' cu simbolul '%c'!\n", stare, simbol_str[0]);
                }
                break;
            case 7:
                printf("Nume fisier: ");
                scanf("%127s", fisier_nou);
                if (citeste_automat(&fa, fisier_nou)) {
                    printf("Automatul din '%s' a fost incarcat cu succes!\n", fisier_nou);
                }
                break;
            case 0:
                printf("La revedere!\n");
                break;
            default:
                printf("Optiune invalida!\n");
        }
    } while (optiune != 0);

    return 0;
}
