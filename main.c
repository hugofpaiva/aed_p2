#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#define minus_inf -1000000000 // a very small integer
#define plus_inf +1000000000  // a very large integer
int count_array;              //Variável para ver quantos espaços do array estamos a usar

typedef struct file_data
{                  // public data
    long word_pos; // zero-based
    long word_num; // zero-based
    char word[64];
    // private data
    FILE *fp;
    long current_pos; // zero-based
} file_data_t;

typedef struct link_ele
{
    char word[64];
    long count;            //contador de palavras
    long tdist;            //total da soma das distâncias (em relação ao contador de palavras geral)
    long tdistp;           //total da soma das distâncias (em relação à posição dos indices)
    long dmin;             //distância mínima (em relação ao contador de palavras geral)
    long dmax;             //distância máxima (em relação ao contador de palavras geral)
    long dminp;            //distância mínima (em relação à posição dos indices)
    long dmaxp;            //distância máxima (em relação à posição dos indices)
    long last;             //última posição (em relação ao contador de palavras geral)
    long first;            //primeira posição (em relação ao contador de palavras geral)
    long lastp;            //última posição (em relação à posição dos indices)
    long firstp;           //primeira posição (em relação à posição dos indices)
    struct link_ele *next; // Pointer para a próxima palavra
} link_ele;

typedef struct tree_node
{
    struct tree_node *left;   // pointer to the left branch (a sub-tree)
    struct tree_node *right;  // pointer to the right branch (a sub-tree)
    struct tree_node *parent; // optional
    char word[64];
    long count;  //contador de palavras
    long tdist;  //total da soma das distâncias (em relação ao contador de palavras geral)
    long tdistp; //total da soma das distâncias (em relação à posição dos indices)
    long dmin;   //distância mínima (em relação ao contador de palavras geral)
    long dmax;   //distância máxima (em relação ao contador de palavras geral)
    long dminp;  //distância mínima (em relação à posição dos indices)
    long dmaxp;  //distância máxima (em relação à posição dos indices)
    long last;   //última posição (em relação ao contador de palavras geral)
    long first;  //primeira posição (em relação ao contador de palavras geral)
    long lastp;  //última posição (em relação à posição dos indices)
    long firstp; //primeira posição (em relação à posição dos indices)
    long data;   // the data item (we use an int here, but it can be anything)
} tree_node;

unsigned int hash_function(const char *str, unsigned int s)
{ // for 32-bit unsigned integers, s should be smaller that 16777216u
    unsigned int h;
    for (h = 0u; *str != '\0'; str++)
        h = (256u * h + (0xFFu & (unsigned int)*str)) % s;
    return h;
}

void add_node(tree_node **words, file_data_t *f, int size)
{
    int index = hash_function(f->word, size);
    tree_node *actual = words[index];
    if (actual != NULL) //se já existir um elemento na ordered binary tree
    {
        if (strcmp(actual->word, f->word) == 0)
        { // se for igual
            long tempdist = f->word_num - actual->last;
            long tempdistp = f->current_pos - actual->lastp;
            actual->tdist = actual->tdist + tempdist;
            actual->tdistp = actual->tdistp + tempdistp;
            if (tempdist < actual->dmin)
                actual->dmin = tempdist;
            if (tempdist > actual->dmax)
                actual->dmax = tempdist;
            if (tempdistp < actual->dminp)
                actual->dminp = tempdistp;
            if (tempdistp > actual->dmaxp)
                actual->dmaxp = tempdistp;
            actual->count++;
            actual->last = f->word_num;
            actual->lastp = f->current_pos;
        }
        else
        { //se não for igual vou percorrer os next elements para ver se há algum igual
            bool found = false;
            while (actual != NULL) // Enquanto a palavra não for encontra e as duas ligações abaixo não forem nulas
            {
                if (strcmp(f->word, actual->word) < 0 && actual->left != NULL) //palavra atual é mais pequena que a deste node
                    actual = actual->left;

                else if (strcmp(f->word, actual->word) < 0 && actual->right != NULL) //palavra atual é mais pequena que a deste node
                    actual = actual->right;

                else if (strcmp(f->word, actual->word) == 0)
                { // se for igual
                    long tempdist = f->word_num - actual->last;
                    long tempdistp = f->current_pos - actual->lastp;
                    actual->tdist = actual->tdist + tempdist;
                    actual->tdistp = actual->tdistp + tempdistp;
                    if (tempdist < actual->dmin)
                        actual->dmin = tempdist;
                    if (tempdist > actual->dmax)
                        actual->dmax = tempdist;
                    if (tempdistp < actual->dminp)
                        actual->dminp = tempdistp;
                    if (tempdistp > actual->dmaxp)
                        actual->dmaxp = tempdistp;
                    actual->count++;
                    actual->last = f->word_num;
                    actual->lastp = f->current_pos;
                    found = true;
                    break;
                }
                else
                    break;
            }

            if (!found) //verificar que nenhum foi encontrado
            {
                tree_node *temp = malloc(sizeof(tree_node));
                strcpy(temp->word, f->word);
                temp->first = f->word_num;
                temp->count = 1;
                temp->last = f->word_num;
                temp->lastp = f->current_pos;
                temp->firstp = f->word_pos;
                temp->parent = actual;
                temp->dmin = plus_inf;                 //indicativo que a distância ainda não foi alterada
                temp->dmax = minus_inf;                //indicativo que a distância ainda não foi alterada
                temp->dminp = plus_inf;                //indicativo que a distância ainda não foi alterada
                temp->dmaxp = minus_inf;               //indicativo que a distância ainda não foi alterada
                if (strcmp(f->word, actual->word) < 0) //palavra atual é mais pequena que a deste node
                    actual->left = temp;

                else if (strcmp(f->word, actual->word) < 0) //palavra atual é mais pequena que a deste node
                    actual->right = temp;
            }
        }
    }
    else
    { //definir a root
        tree_node *new = malloc(sizeof(tree_node));
        strcpy(new->word, f->word);
        new->parent = NULL;
        new->left = NULL;
        new->right = NULL;
        new->count = 0;
        new->dmin = plus_inf;   //indicativo que a distância ainda não foi alterada
        new->dmax = minus_inf;  //indicativo que a distância ainda não foi alterada
        new->dminp = plus_inf;  //indicativo que a distância ainda não foi alterada
        new->dmaxp = minus_inf; //indicativo que a distância ainda não foi alterada
        new->first = f->word_num;
        new->count++;
        new->last = f->word_num;
        new->lastp = f->current_pos;
        new->firstp = f->word_pos;
        words[index] = new;
    }
}

void add_ele(link_ele **words, file_data_t *f, int size)
{
    int index = hash_function(f->word, size);
    link_ele *actual = words[index];
    if (actual != NULL) //se já existir um elemento na linked list daquele index
    {
        if (strcmp(actual->word, f->word) == 0)
        { // se for igual
            long tempdist = f->word_num - actual->last;
            long tempdistp = f->current_pos - actual->lastp;
            actual->tdist = actual->tdist + tempdist;
            actual->tdistp = actual->tdistp + tempdistp;
            if (tempdist < actual->dmin)
                actual->dmin = tempdist;
            if (tempdist > actual->dmax)
                actual->dmax = tempdist;
            if (tempdistp < actual->dminp)
                actual->dminp = tempdistp;
            if (tempdistp > actual->dmaxp)
                actual->dmaxp = tempdistp;
            actual->count++;
            actual->last = f->word_num;
            actual->lastp = f->current_pos;
        }
        else
        { //se não for igual vou percorrer os next elements para ver se há algum igual
            bool found = false;
            while (actual->next != NULL)
            {
                actual = actual->next;
                if (strcmp(actual->word, f->word) == 0)
                { // se for igual
                    long tempdist = f->word_num - actual->last;
                    long tempdistp = f->current_pos - actual->lastp;
                    actual->tdist = actual->tdist + tempdist;
                    actual->tdistp = actual->tdistp + tempdistp;
                    if (tempdist < actual->dmin)
                        actual->dmin = tempdist;
                    if (tempdist > actual->dmax)
                        actual->dmax = tempdist;
                    if (tempdistp < actual->dminp)
                        actual->dminp = tempdistp;
                    if (tempdistp > actual->dmaxp)
                        actual->dmaxp = tempdistp;
                    actual->count++;
                    actual->last = f->word_num;
                    actual->lastp = f->current_pos;
                    found = true;
                    break;
                }
            }
            if (!found) //verificar que nenhum foi encontrado
            {
                link_ele *temp = malloc(sizeof(link_ele));
                strcpy(temp->word, f->word);
                temp->first = f->word_num;
                temp->count = 1;
                temp->last = f->word_num;
                temp->lastp = f->current_pos;
                temp->firstp = f->word_pos;
                temp->next = NULL;
                temp->dmin = plus_inf;   //indicativo que a distância ainda não foi alterada
                temp->dmax = minus_inf;  //indicativo que a distância ainda não foi alterada
                temp->dminp = plus_inf;  //indicativo que a distância ainda não foi alterada
                temp->dmaxp = minus_inf; //indicativo que a distância ainda não foi alterada
                actual->next = temp;
            }
        }
    }
    else
    {
        count_array++;
        link_ele *new = malloc(sizeof(link_ele));
        strcpy(new->word, f->word);
        new->next = NULL;
        new->count = 0;
        new->dmin = plus_inf;   //indicativo que a distância ainda não foi alterada
        new->dmax = minus_inf;  //indicativo que a distância ainda não foi alterada
        new->dminp = plus_inf;  //indicativo que a distância ainda não foi alterada
        new->dmaxp = minus_inf; //indicativo que a distância ainda não foi alterada
        new->first = f->word_num;
        new->count++;
        new->last = f->word_num;
        new->lastp = f->current_pos;
        new->firstp = f->word_pos;
        words[index] = new;
    }
}

link_ele **resize_link(link_ele **words, int *size)
{
    int newsize = 2 * (*size);
    link_ele **words_temp = (link_ele *)calloc(newsize, sizeof(link_ele *));
    for (int i = 0; i < (*size); i++)
    {
        if (words[i] != NULL)
        {
            int index = hash_function(words[i]->word, newsize);
            words_temp[index] = words[i];
        }
    }
    *size = 2 * (*size);
    return words_temp;
}

void get_info_link(link_ele **words, int size)
{
    char name[64];
    printf("Insert word for info: ");
    scanf("%[^\n]", name);
    fflush(stdin);
    //get info about a word
    int index = hash_function(name, size);
    link_ele *actual = words[index];
    bool found = false;
    if (actual != NULL)
    {
        while (actual->next != NULL)
        {
            if (strcmp(actual->word, name) == 0) //Double check, pode vir a ser retirado
            {
                printf("\nInformation about word '%s'\n", actual->word);
                printf("\nCount: %ld\n", actual->count);
                printf("\nPosition (related to the index position of all the text):\n");
                printf("First: %ld\n", actual->first);
                printf("Last: %ld\n", actual->last);
                printf("\nPosition (related to the distinct word counter):\n");
                printf("First: %ld\n", actual->firstp);
                printf("Last: %ld\n", actual->lastp);
                if (actual->count > 1)
                {
                    printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                    printf("Smallest: %ld\n", actual->dminp);
                    printf("Average: %.2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias e não de palavras
                    printf("Largest: %ld\n", actual->dmaxp);
                    printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                    printf("Smallest: %ld\n", actual->dmin);
                    printf("Average: %.2f\n", (float)(actual->tdist) / (actual->count - 1));
                    printf("Largest: %ld\n\n", actual->dmax);
                }
                else
                {
                    printf("\n No distances stats available.\n\n");
                }
                found = true;
                break;
            }
            actual = actual->next;
        }
    }

    if (!found)
    {
        printf("Word %s not found!\n", name);
        exit(0);
    }
}

void get_info_node(tree_node **words, int size)
{
    char name[64];
    printf("Insert word for info: ");
    scanf("%[^\n]", name);
    fflush(stdin);
    //get info about a word
    int index = hash_function(name, size);
    tree_node *actual = words[index];
    bool found = false;
    if (actual != NULL)
    {
        while (actual != NULL)
        {
            if (strcmp(name, actual->word) < 0 && actual->left != NULL) //palavra atual é mais pequena que a deste node
                actual = actual->left;

            else if (strcmp(name, actual->word) > 0 && actual->right != NULL) //palavra atual é maior que a deste node
                actual = actual->right;

            else if (strcmp(name, actual->word) == 0)
            { // se for igual
                printf("\nInformation about word '%s'\n", actual->word);
                printf("\nCount: %ld\n", actual->count);
                printf("\nPosition (related to the index position of all the text):\n");
                printf("First: %ld\n", actual->first);
                printf("Last: %ld\n", actual->last);
                printf("\nPosition (related to the distinct word counter):\n");
                printf("First: %ld\n", actual->firstp);
                printf("Last: %ld\n", actual->lastp);
                if (actual->count > 1)
                {
                    printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                    printf("Smallest: %ld\n", actual->dminp);
                    printf("Average: %.2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias e não de palavras
                    printf("Largest: %ld\n", actual->dmaxp);
                    printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                    printf("Smallest: %ld\n", actual->dmin);
                    printf("Average: %.2f\n", (float)(actual->tdist) / (actual->count - 1));
                    printf("Largest: %ld\n\n", actual->dmax);
                }
                else
                {
                    printf("\n No distances stats available.\n\n");
                }
                found = true;
                break;
            }
            else
                break;
        }
    }
    if (!found)
    {
        printf("Word %s not found!\n", name);
        exit(0);
    }
}

void get_info_node_all(tree_node **words, int size)
{

    bool found = false;
    for (int i = 0; i < size; i++)
    {
        tree_node *actual = words[i];
        tree_node *pre;
        if (actual != NULL)
        {
            while (actual != NULL)
            {
                if (actual->left == NULL)
                {
                    printf("\nInformation about word '%s'\n", actual->word);
                    printf("\nCount: %ld\n", actual->count);
                    printf("\nPosition (related to the index position of all the text):\n");
                    printf("First: %ld\n", actual->first);
                    printf("Last: %ld\n", actual->last);
                    printf("\nPosition (related to the distinct word counter):\n");
                    printf("First: %ld\n", actual->firstp);
                    printf("Last: %ld\n", actual->lastp);
                    if (actual->count > 1)
                    {
                        printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                        printf("Smallest: %ld\n", actual->dminp);
                        printf("Average: %.2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias e não de palavras
                        printf("Largest: %ld\n", actual->dmaxp);
                        printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                        printf("Smallest: %ld\n", actual->dmin);
                        printf("Average: %.2f\n", (float)(actual->tdist) / (actual->count - 1));
                        printf("Largest: %ld\n\n", actual->dmax);
                    }
                    else
                    {
                        printf("\n No distances stats available.\n\n");
                    }
                    found = true;
                    actual = actual->right;
                }
                else
                {
                    /* Find the inorder predecessor of current */
                    pre = actual->left;
                    while (pre->right != NULL && pre->right != actual)
                        pre = pre->right;

                    /* Make current as the right child of its inorder  
               predecessor */
                    if (pre->right == NULL)
                    {
                        pre->right = actual;
                        actual = actual->left;
                    }

                    /* Revert the changes made in the 'if' part to restore  
               the original tree i.e., fix the right child 
               of predecessor */
                    else
                    {
                        pre->right = NULL;
                        printf("\nInformation about word '%s'\n", actual->word);
                        printf("\nCount: %ld\n", actual->count);
                        printf("\nPosition (related to the index position of all the text):\n");
                        printf("First: %ld\n", actual->first);
                        printf("Last: %ld\n", actual->last);
                        printf("\nPosition (related to the distinct word counter):\n");
                        printf("First: %ld\n", actual->firstp);
                        printf("Last: %ld\n", actual->lastp);
                        if (actual->count > 1)
                        {
                            printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                            printf("Smallest: %ld\n", actual->dminp);
                            printf("Average: %.2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias e não de palavras
                            printf("Largest: %ld\n", actual->dmaxp);
                            printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                            printf("Smallest: %ld\n", actual->dmin);
                            printf("Average: %.2f\n", (float)(actual->tdist) / (actual->count - 1));
                            printf("Largest: %ld\n\n", actual->dmax);
                        }
                        else
                        {
                            printf("\n No distances stats available.\n\n");
                        }
                        found = true;
                        actual = actual->right;
                    }
                }
            }
        }
    }
    if (!found)
    {
        printf("No words found!\n");
        exit(0);
    }
}

void get_info_link_all(link_ele **words, int size)
{
    bool found = false;
    char name[64];
    printf("Insert word, or start of it, for info (empty for all): ");
    if (gets(name) != NULL)
    {
        int s_name = strlen(name);
        for (int i = 0; i < size; i++)
        {
            link_ele *actual = words[i];
            if (actual != NULL)
            {
                while (actual->next != NULL)
                {
                    if (strncmp(name, actual->word, s_name) == 0)
                    {
                        found = true;
                        printf("\nInformation about word '%s'\n", actual->word);
                        printf("\nCount: %ld\n", actual->count);
                        printf("\nPosition (related to the index position of all the text):\n");
                        printf("First: %ld\n", actual->first);
                        printf("Last: %ld\n", actual->last);
                        printf("\nPosition (related to the distinct word counter):\n");
                        printf("First: %ld\n", actual->firstp);
                        printf("Last: %ld\n", actual->lastp);
                        if (actual->count > 1)
                        {
                            printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                            printf("Smallest: %ld\n", actual->dminp);
                            printf("Average: %.2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias consecutivas e não de palavras
                            printf("Largest: %ld\n", actual->dmaxp);
                            printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                            printf("Smallest: %ld\n", actual->dmin);
                            printf("Average: %.2f\n", (float)(actual->tdist) / (actual->count - 1));
                            printf("Largest: %ld\n\n", actual->dmax);
                        }
                        else
                        {
                            printf("\nNo distances stats available.\n\n");
                        }
                    }
                    actual = actual->next;
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++)
        {
            link_ele *actual = words[i];
            if (actual != NULL)
            {
                found = true;
                while (actual->next != NULL)
                {
                    printf("\nInformation about word '%s'\n", actual->word);
                    printf("\nCount: %ld\n", actual->count);
                    printf("\nPosition (related to the index position of all the text):\n");
                    printf("First: %ld\n", actual->first);
                    printf("Last: %ld\n", actual->last);
                    printf("\nPosition (related to the distinct word counter):\n");
                    printf("First: %ld\n", actual->firstp);
                    printf("Last: %ld\n", actual->lastp);
                    if (actual->count > 1)
                    {
                        printf("\nDistances beetween consecutive occurrences (related to the index position of all the text):\n");
                        printf("Smallest: %ld\n", actual->dminp);
                        printf("Average: %2f\n", (float)(actual->tdistp) / (actual->count - 1)); //-1 porque quero o numero de distancias e não de palavras
                        printf("Largest: %ld\n", actual->dmaxp);
                        printf("\nDistances beetween consecutive occurrences (related to the distinct word counter):\n");
                        printf("Smallest: %ld\n", actual->dmin);
                        printf("Average: %2f\n", (float)(actual->tdist) / (actual->count - 1));
                        printf("Largest: %ld\n\n", actual->dmax);
                    }
                    else
                    {
                        printf("\nNo distances stats available.\n\n");
                    }

                    actual = actual->next;
                }
            }
        }
    }
    if (!found)
    {
        printf("No words found!\n");
        exit(0);
    }
    fflush(stdin);
}

int open_text_file(char *file_name, file_data_t *fd)
{
    fd->fp = fopen(file_name, "rb");

    if (fd->fp == NULL)
        return -1;
    fd->word_pos = -1;
    fd->word_num = -1;
    fd->word[0] = '\0';
    fd->current_pos = -1;
    return 0;
}

void close_text_file(file_data_t *fd)
{
    fclose(fd->fp);
    fd->fp = NULL;
}

int read_word(file_data_t *fd)
{
    int i, c;
    // skip white spaces
    do
    {
        c = fgetc(fd->fp);
        if (c == EOF)
            return -1;
        fd->current_pos++;
    } while (c <= 32);
    // record word
    fd->word_pos = fd->current_pos;
    fd->word_num++;
    fd->word[0] = (char)c;
    for (i = 1; i < (int)sizeof(fd->word) - 1; i++)
    {
        c = fgetc(fd->fp);
        if (c == EOF)
            break; // end of file
        fd->current_pos++;
        if (c <= 32)
            break; // terminate word
        fd->word[i] = (char)c;
    }
    fd->word[i] = '\0';
    return 0;
}

void usage(char *argv[])
{
    printf("Unknown option\n");
    printf("\nUsage: %s -l -b\n\n", argv[0]);
    printf("-l Initialize program using HashTable with Linked List\n");
    printf("-b Initialize program using HashTable with Ordered Binary Tree\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'l')
    {
        count_array = 0;
        printf("Initializing HashTable with Linked List\n");
        int s_hash = 500;
        link_ele **words = (link_ele *)calloc(s_hash, sizeof(link_ele *)); //cria e anuncia-os como zero(NULL)
        file_data_t *f = malloc(sizeof(file_data_t));
        char file[64];
        printf("Insert filename for stats (e.g.'SherlockHolmes.txt'): ");
        scanf("%[^\n]", file);
        fflush(stdin);
        if (!open_text_file(file, f))
        {
            while (!read_word(f))
            {
                if ((double)count_array / s_hash >= 0.8)
                {
                    words = resize_link(words, &s_hash);
                    count_array = 0;
                }
                add_ele(words, f, s_hash);
            }
            printf("File read successfully!\n");
            close_text_file(f);
        }
        else
        {
            printf("------------------\n");
            printf("Error opening file!\n");
            printf("------------------\n");
            exit(0);
        }
        printf("\n1 - Search for a certain word stats\n2 - Search with a piece of a word or list all words stats\n");
        char option[5];
        printf("\nOption: ");
        scanf("%[^\n]", option);
        fflush(stdin);
        if (strcmp(option, "1") == 0)
            get_info_link(words, s_hash);
        else if (strcmp(option, "2") == 0)
            get_info_link_all(words, s_hash);
        else
        {
            printf("Invalid option");
            exit(0);
        }
    }
    else if (argc == 2 && argv[1][0] == '-' && argv[1][1] == 'b')
    {
        printf("Initializing HashTable with Ordered Binary Tree\n");
        int s_hash = 500;
        tree_node **words = (tree_node *)calloc(s_hash, sizeof(tree_node *)); //cria e anuncia-os como zero(NULL)
        file_data_t *f = malloc(sizeof(file_data_t));
        char file[64];
        printf("Insert filename for stats (e.g.'SherlockHolmes.txt'): ");
        scanf("%[^\n]", file);
        fflush(stdin);
        if (!open_text_file(file, f))
        {
            while (!read_word(f))
            {
                add_node(words, f, s_hash);
            }
            printf("File read successfully!\n");
            close_text_file(f);
        }
        else
        {
            printf("------------------\n");
            printf("Error opening file!\n");
            printf("------------------\n");
            exit(0);
        }
        printf("\n1 - Search for a certain word stats\n2 - Show all words stats\n");
        char option[5];
        printf("\nOption: ");
        scanf("%[^\n]", option);
        fflush(stdin);
        if (strcmp(option, "1") == 0)
            get_info_node(words, s_hash);
        else if (strcmp(option, "2") == 0)
            get_info_node_all(words, s_hash);
        else
        {
            printf("Invalid option");
            exit(0);
        }
    }
    else
    {
        usage(argv);
    }
}
