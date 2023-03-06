#include "markov_chain.h"
#include <string.h>

#define LINE_LENGTH 1001

/**
* Get random number between 0 and max_number [0, max_number).
* @param max_number maximal number to return (not including)
* @return Random number
*/
int get_random_number(int max_number)
{
    return rand() % max_number;
}


/**
 * Get one random state from the given markov_chain's database.
 * @param markov_chain
 * @return
 */
MarkovNode *get_first_random_node(MarkovChain *markov_chain) {
    if(!markov_chain){
        return NULL;
    }
    LinkedList *database = markov_chain->database;
    Node *current_node = database->first;


    while (true) {
        Node *random_node = current_node;
        int i = get_random_number(database->size);
        for (int j = 1; j <= i; j++) {
            random_node = random_node->next;
        }

        if (markov_chain->is_last(random_node->data->data) == false) {
            return random_node->data;
        }
    }
}


/**
 * Choose randomly the next state, depend on it's occurrence frequency.
 * @param state_struct_ptr MarkovNode to choose from
 * @return MarkovNode of the chosen state
 */
MarkovNode *get_next_random_node(MarkovNode *state_struct_ptr) {
    int i = get_random_number(state_struct_ptr->freq_sum);
    NextNodeCounter *counter_list = state_struct_ptr->counter_list;
    for(int j=0; j<state_struct_ptr->counter_lst_size; j++){
        if(counter_list[j].frequency <= i){
            i -= counter_list[j].frequency;
        }else{
            return counter_list[j].markov_node;
        }
    }
    return NULL;
}


/**
 * Receive markov_chain, generate and print random sentence out of it. The
 * sentence most have at least 2 words in it.
 * @param markov_chain
 * @param first_node markov_node to start with,
 *                   if NULL- choose a random markov_node
 * @param  max_length maximum length of chain to generate
 */
void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length) {
    if(!first_node){
        first_node = get_first_random_node(markov_chain);
    }
    int length = 0;
//    char tweet[LINE_LENGTH];
//    strcpy(tweet, first_node->data);
//    strcat(tweet, " ");
//    markov_chain->print_func(first_node->data);
    MarkovNode *current_node = first_node;
//    MarkovNode *current_node = get_next_random_node(first_node);
    while(length < max_length){
//        strcat(tweet, current_node->data);

        if(markov_chain->is_last(current_node->data)){
            markov_chain->print_func(current_node->data);
          break;
        }
        markov_chain->print_func(current_node->data);
//        strcat(tweet, " ");
        current_node = get_next_random_node(current_node);
        length ++;

    }
//    strcat (tweet, "\n");
//    fprintf(stdout, "%s", tweet);
//    strcpy(tweet, " ");
}


/**
 * Free markov_chain and all of it's content from memory
 * @param markov_chain markov_chain to free
 */
void free_markov_chain(MarkovChain ** ptr_chain){
    MarkovChain *markov_chain = *ptr_chain;
    LinkedList *database = markov_chain->database;
    Node* head = database->first;
    Node *tmp;
    while(head != NULL){
        tmp = head;
        head = head->next;
        markov_chain->free_data(tmp->data->data);
        free(tmp->data->counter_list);
        free(tmp->data);
        free(tmp);
    }
    free(database);
    free(markov_chain);
}


/**
 * Add the second markov_node to the counter list of the first markov_node.
 * If already in list, update it's counter value.
 * @param first_node
 * @param second_node
 * @return success/failure: true if the process was successful, false if in
 * case of allocation error.
 */
bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain) {
    if (!first_node | !second_node) {
        return false;
    }
//    char *searched_word = malloc(sizeof(char)*(strlen(second_node->data)+1));
//    strcpy(searched_word, second_node->data);
    void* searched_word = markov_chain->copy_func(second_node->data);
    if (first_node->counter_list) {
        int i = 0;
        for (; i < first_node->counter_lst_size; i++) {
            void *word = first_node->counter_list[i].markov_node->data;
            if (markov_chain->comp_func(word, searched_word) == 0) {
                first_node->counter_list[i].frequency++;
                first_node->freq_sum++;
                markov_chain->free_data(searched_word);
//                free(searched_word);
                return true;
            }
        }
        NextNodeCounter *tmp = realloc(first_node->counter_list,
                                       sizeof(NextNodeCounter)*
                                       (first_node->counter_lst_size+1));
        if (!tmp) {
            free(searched_word);
            return false;
        }
        first_node->counter_list = tmp;
        first_node->counter_list[i].markov_node = second_node;
        first_node->counter_list[i].frequency = 1;
        first_node->freq_sum++;
        first_node->counter_lst_size++;
//        free(searched_word);
        markov_chain->free_data(searched_word);
        return true;
    } else {
        first_node->counter_list = malloc(sizeof(NextNodeCounter));
        if (!first_node->counter_list) {
//            free(searched_word);
            markov_chain->free_data(searched_word);
            return false;
        }
        first_node->counter_list[0].markov_node = second_node;
        first_node->counter_list[0].frequency = 1;
        first_node->counter_lst_size++;
        first_node->freq_sum = 1;
//        free(searched_word);
        markov_chain->free_data(searched_word);
        return true;
    }

}


/**
* Check if data_ptr is in database. If so, return the markov_node
 * wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return Pointer to the Node wrapping given state, NULL if state not in
 * database.
 */
Node *get_node_from_database(MarkovChain *markov_chain, void *data_ptr) {
    if (!markov_chain || !data_ptr) {
        return NULL;
    }
    if(!(markov_chain->database->first)){
        return NULL;
    }
    void *data = markov_chain->copy_func(data_ptr);
    data_ptr = NULL;
    Node *current_node = markov_chain->database->first;
    while (current_node != NULL) {
        MarkovNode *word_node = current_node->data;
        if (markov_chain->comp_func(word_node->data, data) == 0) {
            markov_chain->free_data(data);
            return current_node;
        }
        current_node = current_node->next;
    }
    markov_chain->free_data(data);
    return NULL;
}


Node *create_new_node(void *data_ptr, MarkovChain *markov_chain) {
    LinkedList *database = markov_chain->database;
    // allocation to data_ptr, as asked in the forum:
    void *searched_word = markov_chain->copy_func(data_ptr);

    MarkovNode *new_markov_node = malloc(sizeof(MarkovNode));
    if (!new_markov_node) {
        return NULL;
    }
    new_markov_node->data = searched_word;
    new_markov_node->has_dot = false;
    if (markov_chain->is_last(new_markov_node->data)) {
        new_markov_node->has_dot = true;
    }

    new_markov_node->counter_list = NULL;
    new_markov_node->counter_lst_size = 0;
    new_markov_node->freq_sum = 0;
    add(database,new_markov_node);
    Node *new_node = database->last;
    return new_node;
}


/**
* If data_ptr in markov_chain, return it's node. Otherwise, create new
 * node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the state to look for
 * @return markov_node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
Node *add_to_database(MarkovChain *markov_chain, void *data_ptr) {
    if (!markov_chain || !data_ptr) {
        return NULL;
    }

    LinkedList *database = markov_chain->database;
    if (database->size == 0) {
        // add new node to end
        Node* new_node = create_new_node(data_ptr, markov_chain);
        return new_node;
    } else {
        //search for node
        Node *found_data_node = get_node_from_database(markov_chain, data_ptr);
        if (!found_data_node) {  //if not found place it in the last node.
            Node* new_node = create_new_node(data_ptr, markov_chain);
            return new_node;
        } else {  //if found good return.
            return found_data_node;
        }
    }
}


