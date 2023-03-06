#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define DECIMAL_BASE 10
#define PARAMETERS_COUNT_MSG "Usage: There should be 2 variables."
#define FILE_PATH_ERROR "Error: Cannot open file, check file path."
#define MARKOV_CHAIN_ALLOCATION_FAILURE "Allocation failure: markov chain"
#define DATABASE_ALLOCATION_FAILURE "Allocation failure: database"
/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case there
    // is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database)
{
  printf("%s", error_msg);
  if (database != NULL)
  {
    free_markov_chain(database);
  }
  return EXIT_FAILURE;
}

int compare_cells (void *data_1, void *data_2)
{
  Cell *cell_1 = (Cell *) data_1;
  Cell *cell_2 = (Cell *) data_2;
  return (cell_1->number - cell_2->number);
}

void print_cell (void *data)
{
  Cell *cell = (Cell *) data;
  if (cell->ladder_to != EMPTY)
  {
    printf ("[%d]-ladder to %d -> ", cell->number, cell->ladder_to);
    return;
  }
  if (cell->snake_to != EMPTY)
  {
    printf ("[%d]-snake to %d -> ", cell->number, cell->snake_to);
    return;
  }
  if(cell->number == BOARD_SIZE)
  {
    printf ("[%d]", cell->number);
    return;
  }
  printf ("[%d] -> ", cell->number);
}

void *copy_cell (void *data)
{
  Cell *cell = (Cell *) data;
  Cell *new_cell = malloc (sizeof (Cell));
  if (new_cell)
  {
    new_cell->number = cell->number;
    new_cell->snake_to = cell->snake_to;
    new_cell->ladder_to = cell ->ladder_to;
    return (void *) new_cell;
  }
  //todo: do i need to free new cell?
  return NULL;
}

void free_cell (void *data)
{
  Cell *cell = (Cell *) data;
  free (cell);
}

bool is_last_cell (void *data)
{
  Cell *cell = (Cell *) data;
  if (cell->number == BOARD_SIZE)
  {
    return true;
  }
  return false;
}

static int create_board(Cell *cells[BOARD_SIZE])
{
  for (int i = 0; i < BOARD_SIZE; i++)
  {
    cells[i] = malloc(sizeof(Cell));
    if (cells[i] == NULL)
    {
      for (int j = 0; j < i; j++) {
        free(cells[j]);
      }
      handle_error(ALLOCATION_ERROR_MASSAGE,NULL);
      return EXIT_FAILURE;
    }
    *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
  }

  for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
  {
    int from = transitions[i][0];
    int to = transitions[i][1];
    if (from < to)
    {
      cells[from - 1]->ladder_to = to;
    }
    else
    {
      cells[from - 1]->snake_to = to;
    }
  }
  return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database (MarkovChain *markov_chain)
{
  Cell* cells[BOARD_SIZE];
  if(create_board(cells) == EXIT_FAILURE)
  {
    return EXIT_FAILURE;
  }
  MarkovNode *from_node = NULL, *to_node = NULL;
  size_t index_to;
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    add_to_database(markov_chain, cells[i]);
  }

  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    from_node = get_node_from_database(markov_chain,cells[i])->data;

    if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
    {
      index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
      to_node = get_node_from_database(markov_chain, cells[index_to])
          ->data;
      add_node_to_counter_list(from_node, to_node, markov_chain);
    }
    else
    {
      for (int j = 1; j <= DICE_MAX; j++)
      {
        index_to = ((Cell*) (from_node->data))->number + j - 1;
        if (index_to >= BOARD_SIZE)
        {
          break;
        }
        to_node = get_node_from_database(markov_chain, cells[index_to])
            ->data;
        add_node_to_counter_list(from_node, to_node, markov_chain);
      }
    }
  }
  // free temp arr
  for (size_t i = 0; i < BOARD_SIZE; i++)
  {
    free(cells[i]);
  }
  return EXIT_SUCCESS;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main (int argc, char *argv[])
{
  if(argc != 3){
    fprintf (stdout, PARAMETERS_COUNT_MSG);
    return EXIT_FAILURE;
  }
  unsigned int seed = strtol(argv[1], NULL, DECIMAL_BASE);
  int num_of_routes = (int) strtol (argv[2], NULL,
                                    DECIMAL_BASE);
  srand (seed);

  //create markov chain:
  MarkovChain *markov_chain = malloc(sizeof(MarkovChain));
  if (!markov_chain)
  {
    fprintf(stdout, MARKOV_CHAIN_ALLOCATION_FAILURE);
    return EXIT_FAILURE;
  }
  LinkedList *tmp = malloc(sizeof(LinkedList));
  if (!tmp)
  {
    free (markov_chain);
    fprintf (stdout, DATABASE_ALLOCATION_FAILURE);
    return EXIT_FAILURE;
  }

  markov_chain->database = tmp;
  markov_chain->database->first = NULL;
  markov_chain->database->last = NULL;
  markov_chain->database->size = 0;
  /** function pointers */
  markov_chain->comp_func = &compare_cells;
  markov_chain->print_func = &print_cell;
  markov_chain->copy_func = &copy_cell;
  markov_chain->free_data = &free_cell;
  markov_chain->is_last = &is_last_cell;

  if (fill_database (markov_chain))
  {
    free_markov_chain (&markov_chain);
    return EXIT_FAILURE;
  }

  int count = 1;
  while (count <= num_of_routes)
  {
    MarkovNode *first_node = markov_chain->database->first->data;
    fprintf(stdout, "Random Walk %d: ", count);
    generate_random_sequence (markov_chain, first_node,
                              MAX_GENERATION_LENGTH);
    fprintf (stdout, "\n");
    count++;
  }

  free_markov_chain(&markov_chain);
  return EXIT_SUCCESS;



}
