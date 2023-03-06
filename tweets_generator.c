
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

#define PARAMETERS_COUNT_MSG "Usage: The should be 3 or 4 variables."
#define FILE_PATH_ERROR "Error: Cannot open file, check file path."
#define MARKOV_CHAIN_ALLOCATION_FAILURE "Allocation failure: markov chain"
#define DATABASE_ALLOCATION_FAILURE "Allocation failure: database"
#define LINE_LENGTH 1001
#define LOWER_ARGC_LIMIT 4
#define UPPER_ARGC_LIMIT 5
#define DECIMAL_BASE 10
#define SEQUENCE_MAX_LENGTH 20

#define NO_INPUT -1
#define TWEET_START_SIZE 20

int count_markov_chain(MarkovChain *markov_chain){
  Node *current_node = markov_chain->database->first;
  int count = 0;
  while(current_node != NULL)
  {
    current_node = current_node->next;
    count++;
  }
  return count;
}


FILE *open_file(char *path) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return NULL;
    }
    return file;
}

int compare_words (void *data_1, void *data_2)
{
    const char *word_1 = (const char *) data_1;
    const char *word_2 = (const char *) data_2;
    return strcmp (word_1, word_2);
}

void print_word (void *data)
{
    const char *word = (const char *) data;
    printf("%s ", word);
}

void* copy_word (void *data)
{
    const char *word = (const char *) data;
    char *new_allocated_word = calloc(sizeof(char), strlen(word) + 1);
    if(new_allocated_word)
    {
        strcpy(new_allocated_word, word);
        return (void *) new_allocated_word;
    }
    return NULL;
}

void free_word (void* data)
{
    char* word = (char *) data;
    free(word);
}

bool is_last_word (void *data)
{
    const char* word = (const char *) data;
    if(strcmp(&word[strlen(word) - 1], ".") == 0){
        return true;
    }
    else{
        return false;
    }
}

int fill_database (FILE *fp, int words_to_read, MarkovChain *markov_chain){
    //read file:
    char line[LINE_LENGTH] = {0};
    int counter = 0;
    if(words_to_read == 0){
        return EXIT_SUCCESS;
    }
    char *word;
    while(fgets(line, LINE_LENGTH, fp)) {
        Node *prev = NULL;
        Node *current_node = NULL;

        word = (char*)strtok(line, " \n\r");
        while (word != NULL) {
            if(strcmp(word, "\n") == 0){
              break;
            }
            if (!(current_node = add_to_database(markov_chain, word))) {
                return EXIT_FAILURE;
            }

            if (prev) {
                add_node_to_counter_list(prev->data, current_node->data, markov_chain);
            }
            prev = current_node;

            word = (char*)strtok(NULL, " \n\r");
            counter ++;
            if(counter == words_to_read && words_to_read != NO_INPUT){
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {
    if (argc < LOWER_ARGC_LIMIT || argc > UPPER_ARGC_LIMIT) {
        fprintf(stdout, PARAMETERS_COUNT_MSG);
        return EXIT_FAILURE;
    }
    //reading argv:
    unsigned int seed = strtol(argv[1], NULL, DECIMAL_BASE);
    long tweets_num = strtol(argv[2], NULL, DECIMAL_BASE);
    char *file_path = argv[3];
    long file_words_num = NO_INPUT;

    srand(seed);

    if (argc == UPPER_ARGC_LIMIT) {
        file_words_num = strtol(argv[4], NULL, DECIMAL_BASE);
    }

    //get file:
    FILE *file;
    if (!(file = open_file(file_path))) {
        fprintf(stdout, FILE_PATH_ERROR);
        return EXIT_FAILURE;
    }
    //create markov chain:
    MarkovChain *markov_chain = malloc(sizeof(MarkovChain));
    if (!markov_chain) {
        fprintf(stdout, MARKOV_CHAIN_ALLOCATION_FAILURE);
        return EXIT_FAILURE;
    }
    LinkedList *tmp = malloc(sizeof(LinkedList));
    if (!tmp) {
        free(markov_chain);
        fprintf(stdout, DATABASE_ALLOCATION_FAILURE);
        return EXIT_FAILURE;
    }
    markov_chain->database = tmp;
    markov_chain->database->first = NULL;
    markov_chain->database->last = NULL;
    markov_chain->database->size = 0;
    /** function pointers */
    markov_chain->comp_func = &compare_words;
    markov_chain->print_func = &print_word;
    markov_chain->copy_func = &copy_word;
    markov_chain->free_data = &free_word;
    markov_chain->is_last = &is_last_word;

    //fill database with file data:
    if (fill_database(file,(int)file_words_num , markov_chain)) {
      free_markov_chain (&markov_chain);
      return EXIT_FAILURE;
    }
//    printf("%d", count_markov_chain (markov_chain));


//    Node* tmp2 = markov_chain->database->first;
//    while(tmp2!=NULL){
//        printf("%s \n",tmp2->data->data);
//        tmp2 = tmp2->next;
//    }
    //create tweet:
    int count = 1;
//    char start[TWEET_START_SIZE];
    while(count <= tweets_num)
    {
      MarkovNode *first_node = get_first_random_node (markov_chain);
//      sprintf(start, "Tweet %d: ", count);
      fprintf(stdout, "Tweet %d: ",count);
      generate_random_sequence (markov_chain, first_node, SEQUENCE_MAX_LENGTH);
      fprintf(stdout, "\n");
      count ++;
    }

    //free markov chain:
    free_markov_chain(&markov_chain);
    return EXIT_SUCCESS;
}