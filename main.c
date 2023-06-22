#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define DICT_SIZE 64

char Letters[DICT_SIZE]={"-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"};
char Indexes[128]={0};

struct List {
    struct Dictionary *head;
    struct Dictionary *tail;
};

struct Dictionary {
    struct Dictionary *next;
    char *string;
};

struct Filter {
    bool (*errors)[DICT_SIZE];
    int minLetters[DICT_SIZE];
    int maxLetters[DICT_SIZE];
    char *exactPos;
    bool first;
};

int
checkWord(int maxLen, char *toCheck, char *withCheck, struct Filter *filter, struct List head[DICT_SIZE], char *result,
          struct List deleted[DICT_SIZE]);

bool checkIfCorrect(int maxLetters, char const *temp);

bool shouldFilter(struct Dictionary *temp, struct Filter *filter, int maxChars, bool *failedOnFirstChar);

int applyFilter(struct List head[DICT_SIZE], struct Filter *filter, int maxChars, struct List *deleted);

void append(struct List *result, struct Dictionary *temp) {
    if (result->head == NULL) {
        temp->next = NULL;
        result->head = temp;
        result->tail = temp;
        return;
    }
    result->tail->next = temp;
    temp->next = NULL;
    result->tail = temp;
}

void merge(struct Dictionary *l0, struct Dictionary *l1, struct List *result) {
    while (l0 != NULL || l1 != NULL) {
        struct Dictionary *next0 = NULL;
        struct Dictionary *next1 = NULL;
        if (l0 != NULL) {
            next0 = l0->next;
        }
        if (l1 != NULL) {
            next1 = l1->next;
        }

        if (l0 == NULL) {
            append(result, l1);
            l1 = next1;
        } else if (l1 == NULL) {
            append(result, l0);
            l0 = next0;
        } else if (strcmp(l0->string, l1->string) < 0) {
            append(result, l0);
            l0 = next0;
        } else {
            append(result, l1);
            l1 = next1;
        }
    }
}

void mergeSplit(struct Dictionary *head, struct Dictionary **left, struct Dictionary **right) {
    struct Dictionary *slow = head;
    struct Dictionary *fast = head->next;

    while (fast != NULL) {
        fast = fast->next;
        if (fast != NULL) {
            slow = slow->next;
            fast = fast->next;
        }
    }

    *left = head;
    *right = slow->next;
    slow->next = NULL;
}

void mergeSortSublist(struct Dictionary *head, struct List *result) {
    if ((head == NULL) || (head->next == NULL)) {
        result->head = head;
        result->tail = head;
        return;
    }

    struct Dictionary *left;
    struct Dictionary *right;
    mergeSplit(head, &left, &right);

    struct List leftList = {0};
    struct List rightList = {0};
    mergeSortSublist(left, &leftList);
    mergeSortSublist(right, &rightList);

    merge(leftList.head, rightList.head, result);
}

void mergeSort(struct List *list) {
    struct List sorted = {0};
    mergeSortSublist(list->head, &sorted);
    list->head = sorted.head;
    list->tail = sorted.tail;
}

void initIndex(){
    for(int i = 0; i < DICT_SIZE; i++)
        Indexes[(int)Letters[i]]=i;
}

int getLetterIdx(char letter) {
    return Indexes[(int)letter];
}

void
insert(struct List list[DICT_SIZE], char *temp, int maxChars, struct Filter *filter, struct List deleted[DICT_SIZE]) {
    if (checkIfCorrect(maxChars, temp)) {
        struct Dictionary *tempor = malloc(sizeof(struct Dictionary));
        tempor->string = malloc(sizeof(char) * (maxChars + 1));
        strcpy(tempor->string, temp);

        struct List *currList;
        int firstCharIdx = getLetterIdx(tempor->string[0]);
        if (filter->first || !shouldFilter(tempor, filter, maxChars, NULL)) {
            currList = &list[firstCharIdx];
        } else {
            currList = &deleted[firstCharIdx];
        }

        tempor->next = currList->head;
        currList->head = tempor;
        if (currList->tail == NULL)
            currList->tail = tempor;
    }
}

bool search(struct Dictionary *head, char *temp) {
    while (head) {
        if (strcmp(head->string, temp) == 0) {
            return true;
        }
        head = head->next;
    }
    return false;
}

bool searchInLists(struct List head[DICT_SIZE], struct List deleted[DICT_SIZE], char *temp) {
    if (search(head[getLetterIdx(temp[0])].head, temp))
        return true;
    if (search(deleted[getLetterIdx(temp[0])].head, temp))
        return true;
    return false;
}

void print(struct List list[DICT_SIZE]) {
    for (int i = 0; i < DICT_SIZE; i++) {
        mergeSort(&list[i]);
        struct Dictionary *temp = list[i].head;
        while (temp) {
            printf("%s\n", temp->string);
            temp = temp->next;
        }
    }
}

struct Filter *filterConstructor(int maxLetters) {
    struct Filter *filter = malloc(sizeof(struct Filter));
    filter->errors = malloc(sizeof(filter->errors[0]) * (maxLetters));
    memset(filter->errors, true, sizeof(filter->errors[0]) * (maxLetters));
    for (int i = 0; i < DICT_SIZE; i++) {
        filter->minLetters[i] = -1;
        filter->maxLetters[i] = maxLetters + 1;
    }
    filter->first = true;
    filter->exactPos = calloc(maxLetters + 1, sizeof(char));
    return filter;
}

void freeFilter(struct Filter *filter) {
    free(filter->errors);
    free(filter->exactPos);
    free(filter);
}

void freeList(struct Dictionary *head) {
    while (head) {
        struct Dictionary *temp = head->next;
        free(head->string);
        free(head);
        head = temp;
    }
}

void join(struct List list[DICT_SIZE], struct List filtered[DICT_SIZE]) {
    for (int i = 0; i < DICT_SIZE; i++) {
        if (filtered[i].head == NULL)
            continue;

        filtered[i].tail->next = list[i].head;
        list[i].head = filtered[i].head;
        if (list[i].tail == NULL)
            list[i].tail = filtered[i].tail;
        filtered[i].head = NULL;
        filtered[i].tail = NULL;
    }
}

void removeFromList(struct List *list, struct List *filtered, struct Dictionary **elem, struct Dictionary *prec) {
    struct Dictionary *temp = (*elem);
    if (temp->next == NULL)
        list->tail = prec;
    (*elem) = (*elem)->next;
    temp->next = filtered->head;
    filtered->head = temp;
    if (filtered->tail == NULL)
        filtered->tail = temp;
}

bool checkIfCorrect(int maxLetters, char const *temp) {
    return true;
}

void countLetters(char *withCheck, int maxLen, int *letters) {
    for (int i = 0; i < maxLen; i++) {
        letters[getLetterIdx(withCheck[i])]++;
    }
}

bool shouldFilter(struct Dictionary *temp, struct Filter *filter, int maxChars, bool *failedOnFirstChar) {
    bool flag = false;

    int letterCounter[DICT_SIZE] = {0};
    countLetters(temp->string, maxChars, letterCounter);

    if (failedOnFirstChar)
        *failedOnFirstChar = false;

    for (int i = 0; (i < maxChars) && (flag != true); i++) {
        int index = getLetterIdx(temp->string[i]);
        if (!(filter->errors[i][index])) {
            flag = true;
        } else if (((letterCounter[index] > filter->maxLetters[index]) && letterCounter[index] == 1)) {
            flag = true;
        } else if ((filter->exactPos[i] != 0) && (filter->exactPos[i] != temp->string[i])) {
            flag = true;
        }
        if (i == 0 && flag && failedOnFirstChar)
            (*failedOnFirstChar) = true;
        if (letterCounter[index] > filter->maxLetters[index]) {
            flag = true;
        }
    }
    for (int j = 0; (j < DICT_SIZE) && (flag != true); j++) {
        if (letterCounter[j] < filter->minLetters[j]) {
            flag = true;
        }
    }
    return flag;
}

void nukeList(struct List *list, struct List *deleted) {
    if (deleted->head) {
        deleted->tail->next = list->head;
        deleted->tail = list->tail;
    } else {
        deleted->head = list->head;
        deleted->tail = list->tail;
    }
    list->head = NULL;
    list->tail = NULL;
}

int applyFilter(struct List list[DICT_SIZE], struct Filter *filter, int maxChars, struct List deleted[DICT_SIZE]) {
    int remainings=0;
    for (int firstCharIdx = 0; firstCharIdx < DICT_SIZE; firstCharIdx++) {
        struct Dictionary **temp = &(list[firstCharIdx].head);
        struct Dictionary *prec = NULL;
        while (*temp) {
            bool failedOnFirstChar;
            bool flag = shouldFilter(*temp, filter, maxChars, &failedOnFirstChar);
            if (flag && failedOnFirstChar) {
                nukeList(&list[firstCharIdx], &deleted[firstCharIdx]);
                break;
            }
            if (flag) {
                removeFromList(&list[firstCharIdx], &deleted[firstCharIdx], temp, prec);
            } else {
                prec = *temp;
                temp = &((*temp)->next);
                remainings++;
            }
        }
    }
    return remainings;
}

int
checkWord(int maxLen, char *toCheck, char *withCheck, struct Filter *filter, struct List head[DICT_SIZE], char *result,
          struct List deleted[DICT_SIZE]) {
    int letterCounter[DICT_SIZE] = {0};
    int remainings=0;
    int control[DICT_SIZE] = {0};
    countLetters(withCheck, maxLen, control);
    countLetters(toCheck, maxLen, letterCounter);
    filter->first = false;
    for (int pos = 0; pos < maxLen; pos++) {
        int i = getLetterIdx(toCheck[pos]);
        if (control[i] == 0) {
            filter->errors[pos][i] = false;
        }
        if (letterCounter[i] > control[i]) {
            filter->maxLetters[i] = control[i];
        }
        if (letterCounter[i] > filter->minLetters[i]) {
            filter->minLetters[i] = MIN(letterCounter[i], control[i]);
        }
    }
    for (int pos = 0; pos < maxLen; pos++) {
        if (toCheck[pos] == withCheck[pos]) {
            result[pos] = '+';
            filter->exactPos[pos] = toCheck[pos];
            control[getLetterIdx(toCheck[pos])]--;
        }
    }
    for (int i = 0; i < maxLen; i++) {
        if (result[i] != '+') {
            if (control[getLetterIdx(toCheck[i])] > 0) {
                result[i] = '|';

                filter->errors[i][getLetterIdx(toCheck[i])] = false;
                control[getLetterIdx(toCheck[i])] -= 1;
            } else {
                result[i] = '/';
                filter->errors[i][getLetterIdx(toCheck[i])] = false;
            }
        }
    }
    remainings = applyFilter(head, filter, maxLen, deleted);
    return remainings;
}

int play(struct List list[DICT_SIZE], int maxChars, struct Filter *filter, struct List deleted[DICT_SIZE]) {
    bool checkInsert = false;
    bool flag = false;
    int maxTries = 0;
    int remainings;
    char result[maxChars * 10 + 1];
    memset(result, 0, sizeof(result));
    char toGuess[maxChars + 1];
    char temp[(maxChars + 1) < 32 ? 32 : (maxChars + 1)];
    if (scanf("%s", toGuess)) {}
    if (scanf("%d", &maxTries)) {}
    while (maxTries > 0) {
        if (scanf("%s", temp)) {}
        if (!checkInsert) {
            if ((strcmp(temp, "+inserisci_inizio") == 0)) {
                checkInsert = true;
                flag = true;
                while (strcmp(temp, "+inserisci_fine") != 0) {
                    if (scanf("%s", temp)) {}
                    if(strcmp(temp, "+inserisci_fine") != 0)
                        insert(list, temp, maxChars, filter, deleted);
                }
            }
            if (strcmp(temp, "+stampa_filtrate") == 0) {
                print(list);
                flag = true;
            }
        }

        if (strcmp(temp, "+inserisci_fine") == 0) {
            if (!checkInsert)
                return -1;
            flag = true;
            checkInsert = false;
        }
        if (!flag) {
            if (strcmp(temp, toGuess) == 0)
                return 1;

            if (!searchInLists(list, deleted, temp)) {
                printf("not_exists\n");
            } else {
                remainings = checkWord(maxChars, temp, toGuess, filter, list, result, deleted);
                printf("%s\n", result);
                memset(result, 0, sizeof(result));
                printf("%d\n", remainings);
                maxTries--;
            }
        } else {
            flag = false;
        }

    }
    return 0;
}

int main() {
    int retVal = -1;
    struct List list[DICT_SIZE] = {0};
    struct List deleted[DICT_SIZE] = {0};
    struct Filter *filter;
    int maxChars;
    int result;
    bool checkInsert = false;
    bool checker = false;
    initIndex();
    if (scanf("%d", &maxChars)) {}
    char temp[(maxChars + 1) < 32 ? 32 : (maxChars + 1)];
    filter = filterConstructor(maxChars);
    do {
        if (scanf("%s", temp) == EOF) {
            retVal = 0;
            goto cleanup;
        }
        if (!checkInsert) {
            if ((strcmp(temp, "+nuova_partita") == 0)) {
                checker = true;
                join(list, deleted);
                freeFilter(filter);
                filter = filterConstructor(maxChars);
                result = play(list, maxChars, filter, deleted);
                switch (result) {
                    case (-1):
                        return 0;
                    case (0):
                        printf("ko\n");
                        break;
                    case (1):
                        printf("ok\n");
                        break;
                }
            }
            if ((strcmp(temp, "+inserisci_inizio") == 0)) {
                if (scanf("%s", temp)) {}
                checkInsert = true;
                checker = true;
            }
            if (strcmp(temp, "+stampa_filtrate") == 0) {
                print(list);
                checker = true;
            }
        }
        if (strcmp(temp, "+inserisci_fine") == 0) {
            if (!checkInsert) {
                retVal = 0;
                goto cleanup;
            }
            checkInsert = false;
            checker = true;
        } else if ((strlen(temp) == maxChars) || (checkInsert)) {
            insert(list, temp, maxChars, filter, deleted);
        } else if (!checker) {
            retVal = 0;
            goto cleanup;
        }
        checker = false;
    } while (!feof(stdin));

    cleanup:
#ifndef EVAL
    for(int i = 0; i < DICT_SIZE; i++) {
        freeList(list[i].head);
        freeList(deleted[i].head);
    }
    freeFilter(filter);
#endif
    return retVal;
}


