#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define TABLE_SIZE 1024

// ------------------ Procesare fișier intrare ------------------
struct Sale {
    char *sale_date;
    int product_id;
    char *product_name;
    char *product_category;
    char *product_subcategory;
    float unit_price;
    int quantity_sold;
    char *sale_country;
    char *sale_city;

    struct Sale* next;
};

struct Sale* createNode(char *sale_date, int product_id, char *product_name, char *product_category, char *product_subcategory, float unit_price, int quantity_sold, char *sale_country, char *sale_city) {
    struct Sale *newSale = (struct Sale*)malloc(sizeof(struct Sale));
    if (newSale == NULL) {
        fprintf(stderr, "Eroare la alocarea memoriei\n");
        exit(1);
    }
    newSale->sale_date = strdup(sale_date);
    newSale->product_id = product_id;
    newSale->product_name = strdup(product_name);
    newSale->product_category = strdup(product_category);
    newSale->product_subcategory = strdup(product_subcategory);
    newSale->unit_price = unit_price;
    newSale->quantity_sold = quantity_sold;
    newSale->sale_country = strdup(sale_country);
    newSale->sale_city = strdup(sale_city);
    newSale->next = NULL;
    return newSale;
}

void append(struct Sale **head, char *sale_date, int product_id, char *product_name, char *product_category, char *product_subcategory, float unit_price, int quantity_sold, char *sale_country, char *sale_city) {
    struct Sale *newSale = createNode(sale_date, product_id, product_name, product_category, product_subcategory, unit_price, quantity_sold, sale_country, sale_city);
    if (*head == NULL) {
        *head = newSale;
    } else {
        struct Sale *lastSale = *head;
        while (lastSale->next != NULL) {
            lastSale = lastSale->next;
        }
        lastSale->next = newSale;
    }
}

float totalSales(struct Sale *head) {
    float total = 0;
    while (head != NULL) {
        total += head->unit_price * head->quantity_sold;
        head = head->next;
    }
    return total;
}

void freeList(struct Sale *head) {
    struct Sale *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->sale_date);
        free(temp->product_name);
        free(temp->product_category);
        free(temp->product_subcategory);
        free(temp->sale_country);
        free(temp->sale_city);
        free(temp);
    }
}

// ------------------ Procesare încasări după lună și an ------------------
typedef struct vanzariTotale {
    int an, luna;
    double suma;
    struct vanzariTotale *next;
} vanzariTotale;

int hashFunction(int an, int luna) {
    return (an * 100 + luna) % TABLE_SIZE;
}

int insertInHashTable(vanzariTotale **hashTable, int an, int luna, double suma) {
    int index = hashFunction(an, luna);
    vanzariTotale *current = hashTable[index];

    while (current != NULL) {
        if (current->an == an && current->luna == luna) {
            current->suma += suma;
            return 0;
        }
        current = current->next;
    }

    vanzariTotale *newRow = (vanzariTotale*)malloc(sizeof(vanzariTotale));
    if (newRow == NULL) {
        fprintf(stderr, "Eroare la alocarea memoriei\n");
        return -1;
    }
    newRow->an = an;
    newRow->luna = luna;
    newRow->suma = suma;
    newRow->next = hashTable[index];
    hashTable[index] = newRow;

    return 0;
}

int compareSales(const void *a, const void *b) {
    vanzariTotale *saleA = *(vanzariTotale**)a;
    vanzariTotale *saleB = *(vanzariTotale**)b;

    if (saleA->an != saleB->an) {
        return saleA->an - saleB->an;
    }
    return saleA->luna - saleB->luna;
}

void calculateRevenue(struct Sale *head, vanzariTotale **hashTable) {
    struct Sale *current = head;
    while (current != NULL) {
        char dateCopy[20];
        strcpy(dateCopy, current->sale_date);

        char* token = strtok(dateCopy, "-");
        if (token != NULL) {
            int an = atoi(token);
            token = strtok(NULL, "-");
            if (token != NULL) {
                int luna = atoi(token);
                double suma = current->quantity_sold * current->unit_price;
                insertInHashTable(hashTable, an, luna, suma);
            }
        }
        current = current->next;
    }
}

void getRevenueByYearMonth(struct Sale *head) {
    vanzariTotale *hashTable[TABLE_SIZE] = {NULL};
    calculateRevenue(head, hashTable);

    // Extragerea tuturor vânzărilor din tabela hash într-un array pentru sortare
    vanzariTotale **salesArray = NULL;
    int salesCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        vanzariTotale *current = hashTable[i];
        while (current != NULL) {
            salesCount++;
            salesArray = realloc(salesArray, salesCount * sizeof(vanzariTotale*));
            if (salesArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                return;
            }
            salesArray[salesCount - 1] = current;
            current = current->next;
        }
    }

    if (salesCount == 0) {
        printf("Nu există date de vânzări.\n");
        free(salesArray);
        return;
    }

    // Sortarea după an și lună
    qsort(salesArray, salesCount, sizeof(vanzariTotale*), compareSales);

    printf("\nTotal vânzări pe luni (sortate):\n");
    printf("--------------------------------\n");
    for (int i = 0; i < salesCount; i++) {
        printf("An: %d, Luna: %d, Suma totală: %.2f MDL\n",
               salesArray[i]->an, salesArray[i]->luna, salesArray[i]->suma);
    }

    // Eliberarea memoriei pentru array
    free(salesArray);

    // Eliberarea memoriei pentru tabelul hash
    for (int i = 0; i < TABLE_SIZE; i++) {
        vanzariTotale *current = hashTable[i];
        while (current != NULL) {
            vanzariTotale *temp = current;
            current = current->next;
            free(temp);
        }
    }
}

// ------------------ Structura pentru tabela hash ------------------
typedef struct HashNode {
    char *key;
    float value;
    struct HashNode *next;
} HashNode;

// Funcție hash simplă pentru șiruri
unsigned int hash(const char *str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % TABLE_SIZE;
}

// Inserare sau actualizare în tabela hash
void insertOrUpdate(HashNode **hashTable, const char *key, float value) {
    unsigned int index = hash(key);
    HashNode *current = hashTable[index];

    // Verificăm dacă cheia există deja
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value += value;
            return;
        }
        current = current->next;
    }

    // Cheia nu există, adăugăm un nou nod
    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (newNode == NULL) {
        fprintf(stderr, "Eroare la alocarea memoriei\n");
        return;
    }
    newNode->key = strdup(key);
    newNode->value = value;
    newNode->next = hashTable[index];
    hashTable[index] = newNode;
}

// Eliberarea memoriei pentru tabela hash
void freeHashTable(HashNode **hashTable) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *current = hashTable[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
        hashTable[i] = NULL;
    }
}

// Funcție de comparare pentru produse (descrescător după venit)
int compareProducts(const void *a, const void *b) {
    HashNode *nodeA = *(HashNode**)a;
    HashNode *nodeB = *(HashNode**)b;
    if (nodeB->value > nodeA->value) return 1;
    if (nodeB->value < nodeA->value) return -1;
    return 0;
}

// ------------------ Procesare top 5 produse ------------------
void getTop5Products(struct Sale *head) {
    HashNode *hashTable[TABLE_SIZE] = {NULL};
    struct Sale *current = head;

    // Calculăm veniturile pentru fiecare produs
    while (current != NULL) {
        float revenue = current->unit_price * current->quantity_sold;
        insertOrUpdate(hashTable, current->product_name, revenue);
        current = current->next;
    }

    // Extragem toate produsele într-un array pentru sortare
    HashNode **productsArray = NULL;
    int productsCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashTable[i];
        while (node != NULL) {
            productsCount++;
            productsArray = realloc(productsArray, productsCount * sizeof(HashNode*));
            if (productsArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                freeHashTable(hashTable);
                return;
            }
            productsArray[productsCount - 1] = node;
            node = node->next;
        }
    }

    // Sortăm produsele după venit (descrescător)
    qsort(productsArray, productsCount, sizeof(HashNode*), compareProducts);

    // Afișăm top 5 produse
    printf("\nTop 5 produse după venit:\n");
    printf("------------------------\n");
    for (int i = 0; i < 5 && i < productsCount; i++) {
        printf("%d. %s: %.2f MDL\n", i + 1, productsArray[i]->key, productsArray[i]->value);
    }

    // Eliberăm memoria
    free(productsArray);
    freeHashTable(hashTable);
}

// ------------------ Procesare categorii ------------------
void afisareTopCategorii(struct Sale *head) {
    HashNode *hashTable[TABLE_SIZE] = {NULL};
    struct Sale *current = head;

    // Calculăm veniturile pentru fiecare categorie
    while (current != NULL) {
        float revenue = current->unit_price * current->quantity_sold;
        insertOrUpdate(hashTable, current->product_category, revenue);
        current = current->next;
    }

    // Extragem toate categoriile într-un array pentru sortare
    HashNode **categoriesArray = NULL;
    int categoriesCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashTable[i];
        while (node != NULL) {
            categoriesCount++;
            categoriesArray = realloc(categoriesArray, categoriesCount * sizeof(HashNode*));
            if (categoriesArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                freeHashTable(hashTable);
                return;
            }
            categoriesArray[categoriesCount - 1] = node;
            node = node->next;
        }
    }

    // Sortăm categoriile după venit (descrescător)
    qsort(categoriesArray, categoriesCount, sizeof(HashNode*), compareProducts);

    // Afișăm categoriile sortate după venit
    printf("\nCategorii de produse după venit:\n");
    printf("------------------------------\n");
    for (int i = 0; i < categoriesCount; i++) {
        printf("%d. %s: %.2f MDL\n", i + 1, categoriesArray[i]->key, categoriesArray[i]->value);
    }

    // Eliberăm memoria
    free(categoriesArray);
    freeHashTable(hashTable);
}

// ------------------ Structură pentru orașe și țări ------------------
typedef struct CityCountry {
    char *city;
    char *country;
    float revenue;
    struct CityCountry *next;
} CityCountry;

// ------------------ Top orașe ------------------
void afisareTopOrase(struct Sale *head) {
    HashNode *hashTable[TABLE_SIZE] = {NULL};
    struct Sale *current = head;

    // Calculăm veniturile pentru fiecare oraș
    while (current != NULL) {
        float revenue = current->unit_price * current->quantity_sold;
        insertOrUpdate(hashTable, current->sale_city, revenue);
        current = current->next;
    }

    // Extragem toate orașele într-un array pentru sortare
    HashNode **citiesArray = NULL;
    int citiesCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = hashTable[i];
        while (node != NULL) {
            citiesCount++;
            citiesArray = realloc(citiesArray, citiesCount * sizeof(HashNode*));
            if (citiesArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                freeHashTable(hashTable);
                return;
            }
            citiesArray[citiesCount - 1] = node;
            node = node->next;
        }
    }

    // Sortăm orașele după venit (descrescător)
    qsort(citiesArray, citiesCount, sizeof(HashNode*), compareProducts);

    // Afișăm top orașe
    printf("\nOrașe cu cele mai mari vânzări:\n");
    printf("-----------------------------\n");
    for (int i = 0; i < 10 && i < citiesCount; i++) {
        printf("%d. %s: %.2f MDL\n", i + 1, citiesArray[i]->key, citiesArray[i]->value);
    }

    // Eliberăm memoria
    free(citiesArray);
    freeHashTable(hashTable);
}

// ------------------ Top orașe pe țări ------------------
void afisareTopOrasePeTari(struct Sale *head) {
    // Folosim un hashtable pentru a stoca toate orașele cu țara lor
    HashNode *countriesTable[TABLE_SIZE] = {NULL};
    CityCountry *cities[TABLE_SIZE] = {NULL};

    struct Sale *current = head;

    // Mai întâi, colectăm toate datele
    while (current != NULL) {
        float revenue = current->unit_price * current->quantity_sold;

        // Creăm o cheie combinată oraș_țară pentru a identifica unic fiecare combinație
        char combined_key[256];
        sprintf(combined_key, "%s_%s", current->sale_city, current->sale_country);

        // Adăugăm sau actualizăm venitul pentru această combinație
        unsigned int idx = hash(combined_key);
        CityCountry *cityEntry = cities[idx];
        int found = 0;

        while (cityEntry != NULL) {
            if (strcmp(cityEntry->city, current->sale_city) == 0 &&
                strcmp(cityEntry->country, current->sale_country) == 0) {
                cityEntry->revenue += revenue;
                found = 1;
                break;
            }
            cityEntry = cityEntry->next;
        }

        if (!found) {
            CityCountry *newEntry = (CityCountry*)malloc(sizeof(CityCountry));
            if (newEntry == NULL) {
                fprintf(stderr, "Eroare la alocarea memoriei\n");
                return;
            }
            newEntry->city = strdup(current->sale_city);
            newEntry->country = strdup(current->sale_country);
            newEntry->revenue = revenue;
            newEntry->next = cities[idx];
            cities[idx] = newEntry;

            // Adăugăm țara în hashtable-ul de țări dacă nu există
            insertOrUpdate(countriesTable, current->sale_country, 0);
        }

        current = current->next;
    }

    // Extragem lista de țări
    HashNode **countriesArray = NULL;
    int countriesCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = countriesTable[i];
        while (node != NULL) {
            countriesCount++;
            countriesArray = realloc(countriesArray, countriesCount * sizeof(HashNode*));
            if (countriesArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                return;
            }
            countriesArray[countriesCount - 1] = node;
            node = node->next;
        }
    }

    // Pentru fiecare țară, găsim orașele și le sortăm după venit
    printf("\nTopul orașelor pentru fiecare țară:\n");
    printf("----------------------------------\n");

    for (int c = 0; c < countriesCount; c++) {
        char *country = countriesArray[c]->key;
        printf("\nȚara: %s\n", country);
        printf("-----------------\n");

        // Colectăm toate orașele pentru această țară
        CityCountry **countryCities = NULL;
        int cityCount = 0;

        for (int i = 0; i < TABLE_SIZE; i++) {
            CityCountry *city = cities[i];
            while (city != NULL) {
                if (strcmp(city->country, country) == 0) {
                    cityCount++;
                    countryCities = realloc(countryCities, cityCount * sizeof(CityCountry*));
                    if (countryCities == NULL) {
                        fprintf(stderr, "Eroare la realocarea memoriei\n");
                        return;
                    }
                    countryCities[cityCount - 1] = city;
                }
                city = city->next;
            }
        }

        // Sortăm orașele după venit (descrescător)
        qsort(countryCities, cityCount, sizeof(CityCountry*),
              (int (*)(const void*, const void*))compareProducts);

        // Afișăm top 3 orașe pentru această țară
        for (int i = 0; i < 3 && i < cityCount; i++) {
            printf("%d. %s: %.2f MDL\n", i + 1, countryCities[i]->city, countryCities[i]->revenue);
        }

        free(countryCities);
    }

    // Eliberăm memoria
    free(countriesArray);
    freeHashTable(countriesTable);

    // Eliberăm memoria pentru orașe
    for (int i = 0; i < TABLE_SIZE; i++) {
        CityCountry *city = cities[i];
        while (city != NULL) {
            CityCountry *temp = city;
            city = city->next;
            free(temp->city);
            free(temp->country);
            free(temp);
        }
    }
}

// ------------------ Tendințe lunare pentru subcategorii ------------------
void afisareTendinteLunareSubcategorii(struct Sale *head) {
    // Structură pentru a stoca datele lunare pentru fiecare subcategorie
    typedef struct {
        int an;
        int luna;
        char *subcategorie;
        float venit;
    } SubcategorieLunara;

    // Tabela hash pentru stocarea datelor
    HashNode *hashTable[TABLE_SIZE] = {NULL};
    struct Sale *current = head;

    // Colectăm datele
    while (current != NULL) {
        char dateCopy[20];
        strcpy(dateCopy, current->sale_date);

        char *token = strtok(dateCopy, "-");
        if (token != NULL) {
            int an = atoi(token);
            token = strtok(NULL, "-");
            if (token != NULL) {
                int luna = atoi(token);
                float revenue = current->unit_price * current->quantity_sold;

                // Creăm o cheie combinată an_luna_subcategorie
                char key[256];
                sprintf(key, "%d_%d_%s", an, luna, current->product_subcategory);

                insertOrUpdate(hashTable, key, revenue);
            }
        }

        current = current->next;
    }

    // Colectăm toate subcategoriile unice
    HashNode *subcategoriesTable[TABLE_SIZE] = {NULL};
    current = head;

    while (current != NULL) {
        insertOrUpdate(subcategoriesTable, current->product_subcategory, 0);
        current = current->next;
    }

    // Extragem toate subcategoriile într-un array
    HashNode **subcategoriesArray = NULL;
    int subcategoriesCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *node = subcategoriesTable[i];
        while (node != NULL) {
            subcategoriesCount++;
            subcategoriesArray = realloc(subcategoriesArray, subcategoriesCount * sizeof(HashNode*));
            if (subcategoriesArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                freeHashTable(subcategoriesTable);
                freeHashTable(hashTable);
                return;
            }
            subcategoriesArray[subcategoriesCount - 1] = node;
            node = node->next;
        }
    }

    // Colectăm toate lunile unice din date
    vanzariTotale *monthsTable[TABLE_SIZE] = {NULL};
    calculateRevenue(head, monthsTable);

    // Extragem toate lunile într-un array pentru sortare
    vanzariTotale **monthsArray = NULL;
    int monthsCount = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        vanzariTotale *month = monthsTable[i];
        while (month != NULL) {
            monthsCount++;
            monthsArray = realloc(monthsArray, monthsCount * sizeof(vanzariTotale*));
            if (monthsArray == NULL) {
                fprintf(stderr, "Eroare la realocarea memoriei\n");
                free(subcategoriesArray);
                freeHashTable(subcategoriesTable);
                freeHashTable(hashTable);
                return;
            }
            monthsArray[monthsCount - 1] = month;
            month = month->next;
        }
    }

    // Sortăm lunile cMDLologic
    qsort(monthsArray, monthsCount, sizeof(vanzariTotale*), compareSales);

    // Afișăm tendințele lunare pentru fiecare subcategorie
    printf("\nTendințe lunare pentru subcategorii de produse:\n");
    printf("--------------------------------------------\n");

    for (int s = 0; s < subcategoriesCount; s++) {
        char *subcategory = subcategoriesArray[s]->key;
        printf("\nSubcategorie: %s\n", subcategory);
        printf("------------------\n");

        for (int m = 0; m < monthsCount; m++) {
            int an = monthsArray[m]->an;
            int luna = monthsArray[m]->luna;

            // Construim cheia pentru acest an_luna_subcategorie
            char key[256];
            sprintf(key, "%d_%d_%s", an, luna, subcategory);

            // Căutăm venitul pentru această combinație
            unsigned int idx = hash(key);
            HashNode *node = hashTable[idx];
            float revenue = 0;

            while (node != NULL) {
                if (strcmp(node->key, key) == 0) {
                    revenue = node->value;
                    break;
                }
                node = node->next;
            }

            printf("An: %d, Luna: %d, Venit: %.2f MDL\n", an, luna, revenue);
        }
    }

    // Eliberăm memoria
    free(subcategoriesArray);
    free(monthsArray);
    freeHashTable(subcategoriesTable);
    freeHashTable(hashTable);

    // Eliberăm memoria pentru luni
    for (int i = 0; i < TABLE_SIZE; i++) {
        vanzariTotale *month = monthsTable[i];
        while (month != NULL) {
            vanzariTotale *temp = month;
            month = month->next;
            free(temp);
        }
    }
}

// Funcția principală a programului
int main() {
    struct Sale *head = NULL;

    FILE *file = fopen("sales.csv", "r");
    if (file == NULL) {
        perror("Eroare la deschiderea fișierului");
        return 1;
    }

    int iter = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        if (iter++ > 0) { // Sărim peste antet
            char* line_copy = strdup(line);
            char* temp = line_copy;

            char* token = strtok(line_copy, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* sale_date = token;

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            int product_id = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* product_name = token;

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* product_category = token;

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* product_subcategory = token;

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            float unit_price = atof(token);

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            int quantity_sold = atoi(token);

            token = strtok(NULL, ",");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* sale_country = token;

            token = strtok(NULL, ",\n");
            if (token == NULL) {
                free(temp);
                continue;
            }
            char* sale_city = token;

            append(&head, sale_date, product_id, product_name, product_category, product_subcategory,
                   unit_price, quantity_sold, sale_country, sale_city);

            free(temp);
        }
    }

    fclose(file);

    int choice;
    do {
        printf("\nMeniu analiza vânzărilor:\n");
        printf("---------------------------\n");
        printf("1. Afișează veniturile totale pe luni și ani\n");
        printf("2. Afișează top 5 produse după venit\n");
        printf("3. Afișează distribuția vânzărilor pe categorii de produse\n");
        printf("4. Afișează orașele cu cele mai mari vânzări\n");
        printf("5. Afișează orașele cu cele mai mari vânzări pentru fiecare țară\n");
        printf("6. Afișează tendințele lunare pentru subcategorii de produse\n");
        printf("0. Ieșire\n");
        printf("---------------------------\n");
        printf("Alege o opțiune: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                getRevenueByYearMonth(head);
                break;
            case 2:
                getTop5Products(head);
                break;
            case 3:
                afisareTopCategorii(head);
                break;
            case 4:
                afisareTopOrase(head);
                break;
            case 5:
                afisareTopOrasePeTari(head);
                break;
            case 6:
                afisareTendinteLunareSubcategorii(head);
                break;
            case 0:
                printf("\nIeșire din program...\n");
                break;
            default:
                printf("\nOpțiune invalidă. Te rog să încerci din nou.\n");
        }
    } while (choice != 0);

    freeList(head);

    return 0;
}