#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

//Definitions
//Basic parameters of the game.
#define NUM_POSSIBLE_STATES 243 //3 to power 5.
#define NUM_VARIABLES 5 //number of relevant inputs, here: up, right, down, left and current square.
#define NUM_VARIATIONS 3 //number of possible states of a board square.
#define NUM_MOVEMENTS 7

//These are important game parameters, and you may tweak them to get better results.
//Keep in mind that making larger some of these numbers will take more time for the algorithm to compute.
#define NUM_SIMULATIONS_AGENT 3 //Number of simulations each agent will run in order to determine its fitness.
#define SPLIT_VALUE_DNA 0.5 //Relative place of the split in the DNA in the process of reproduction. (Value between 0 and 1.)

//All possible states of a board square.
#define WALL 1
#define OBJECT 2
#define NOTHING 0

//All possible movements.
#define GO_UP 0
#define GO_RIGHT 1
#define GO_DOWN 2
#define GO_LEFT 3
#define GO_RANDOM 4
#define PICK_OBJECT 5
#define DO_NOTHING 6

//Possible costs.
#define COST_GO_WALL -5
#define COST_PICK_NOTHING -1
#define COST_PICK_OBJECT 10

//Agent structure
struct agent {
    int x_place;
    int y_place;
    int points;
    int objects_collected;
    double fitness;

    int* dna; //int array of size NUM_POSSIBLE_STATES
};
#define RANDOM_POSITION 0


//Function declarations
void initialize_states(int level, int state);
void setup_movements_int_state(int state);
int power(int num, int pow);
void setup_game_properties(int * board_width, int * board_height, int * population_size, double * mutation_probability, int * num_generations, int * min_object_num, int * max_object_num);
void please_enter_prompt(char * variable_name, int * variable_value);
void setup_board(int ** board, int board_width, int board_height, int min_num_objects, int max_num_objects);

void print_board(int ** board, int board_width, int board_height);
void print_line(int size, char symbol);
void generate_random_dna(int* dna);
void *generate_random_agents(void* arg);
void print_dna(int* dna);
void print_agents(struct agent* agents, int size);
void *calculate_fitness(void* arg);

//Global variables
//Array of all possible states the agent can be in.
//Number of row is the place in the DNA sequence.
//Columns go like this: UP, RIGHT, DOWN, LEFT, CURRENT. (States of these squares.)
int movements[NUM_POSSIBLE_STATES][NUM_VARIABLES];

struct agent* population;
struct agent* new_population;
int main() {
    //Variables for time measurements.
    clock_t start, end;
    double cpu_time_used;

    //Welcome message and initializing all possible agent states.
    printf("Hello, and Welcome to the Object Picking Genetic Algorithm.\n"
           "It will evolve to get around a matrix with objects and pick all of them\n");
    printf("Please wait for the system to initialize.\n");
    initialize_states(0,0);
    printf("Initialization was done successfully.\n");

    //Getting the user to set all relevant variables to run the simulation.
    int board_width;
    int board_height;
    int population_size;
    double mutation_probability;
    int number_of_generations;
    int min_object_number;
    int max_object_number;
    setup_game_properties(&board_width, &board_height, &population_size, &mutation_probability, &number_of_generations, &min_object_number, &max_object_number);

    //Setting up the board will relevant parameters.
    //----Measuring the time----
    start = clock();
    //--------

    int* board[board_height];
    for (int i=0; i < board_height; i++)
        board[i] = (int*)malloc(board_width * sizeof(int));
    setup_board(board, board_width, board_height, min_object_number, max_object_number);

    //----Measuring the time----
    end = clock();
    cpu_time_used = ((double) (end-start))/CLOCKS_PER_SEC;
    //--------

    printf("\nSetting up the game board took %lf seconds\n\n", cpu_time_used);

    //Generating the agents and running the algorithm.

    //----Measuring the time----
    start = clock();
    //--------

    //Allocating enough memory for the desired amount of population.
    population = (struct agent*) malloc(population_size*sizeof(struct agent));
    for (int i=0; i<population_size; i++) {
        population[i].dna = (int*)malloc(NUM_POSSIBLE_STATES*sizeof(int));
    }
    //Dividing the process of setting the population to different threads.
    pthread_t pthread1, pthread2;
    int pthread1_size = population_size/2;
    int pthread1_start = 0;
    int pthread1_arg[2] = {pthread1_start, pthread1_size};
    pthread_create(&pthread1,NULL,generate_random_agents,&pthread1_arg);

    int pthread2_size = population_size-pthread1_size;
    int pthread2_start = pthread1_size;
    int pthread2_arg[2] = {pthread2_start, pthread2_size};
    pthread_create(&pthread2,NULL,generate_random_agents,&pthread2_arg);

    //Wait for threads to finish.
    pthread_join(pthread1,NULL);
    pthread_join(pthread2,NULL);

    //----Measuring the time----
    end = clock();
    cpu_time_used = ((double) (end-start))/CLOCKS_PER_SEC;
    //--------

    printf("\nSetting up the population took %lf seconds\n\n", cpu_time_used);

    //print_agents(population, population_size);


    //------Freeing the Memory------
    //Freeing the memory from malloc usages.
    for (int i=0;i<board_height;i++) {
        free(board[i]);
    }
    //Freeing the dna variables.
    for (int i=0;i<population_size;i++) {
        free(population[i].dna);
    }
    //Freeing the population array.
    free(population);
    return 0;
}

void *calculate_fitness(void* arg) {
    int* args = (int*)arg;
    int start = args[0];
    int size = args[1];

    for (int i=start;i<start+size;i++) {
        struct agent current_agent = population[i];
    }
}

int run_agent(struct agent* agent, int ** board) {

}

int find_state(int ** board, int board_width, int board_height, int x_val, int y_val) {
    int state = 0;

}

void *generate_random_agents(void* arg) {
    int* args = (int*)arg;
    int start = args[0];
    int size = args[1];

    if (RANDOM_POSITION == 1) {
        //TODO: implement random place generation.
    } else {
        for (int i=start;i<start+size;i++) {
            population[i].x_place = 0;
            population[i].y_place = 0;
            population[i].objects_collected = 0;
            population[i].points = 0;
            population[i].fitness = 0;

            generate_random_dna(population[i].dna);
        }
    }
}

void generate_random_dna(int* dna) {
    for (int i=0;i<NUM_POSSIBLE_STATES;i++) {
        dna[i] = rand()%NUM_MOVEMENTS;
    }
}

void setup_board(int ** board, int board_width, int board_height, int min_num_objects, int max_num_objects) {
    for (int i=0; i<board_height; i++) {
        int j=0;
        for (; j<board_width; j++) {
            board[i][j] = 0;
        }
    }

    int num_objects = max_num_objects==min_num_objects ? max_num_objects : min_num_objects + (rand()%(max_num_objects-min_num_objects));
    int row, col;
    for (int i=0; i<num_objects; i++) {
        row = rand()%board_height;
        col = rand()%board_width;
        if (board[row][col] != 0) i--;
        else board[row][col] = OBJECT;
    }
}

void setup_game_properties(int * board_width, int * board_height, int * population_size, double * mutation_probability, int * num_generations, int * min_object_num, int * max_object_num) {
    printf("We shall now setup the game properties.\n");
    please_enter_prompt("Board width", board_width);

    please_enter_prompt("Board height", board_height);

    please_enter_prompt("Population size", population_size);

    printf("Please, enter Mutation probability:\n");
    scanf("%lf", mutation_probability);

    please_enter_prompt("Desired generations", num_generations);

    please_enter_prompt("Minimal number of objects", min_object_num);

    please_enter_prompt("Maximal number of objects", max_object_num);
}

void please_enter_prompt(char * variable_name, int * variable_value) {
    printf("Please, enter %s:\n", variable_name);
    scanf("%d", variable_value);
}

int last_place;
void initialize_states(int level, int state) {
    if (level==NUM_VARIABLES) {
        setup_movements_int_state(state);
        last_place++;
        return;
    }
    for (int i=1; i<=NUM_VARIATIONS; i++) {
        initialize_states(level+1, (state*10)+i);
    }
}

void setup_movements_int_state(int state) {
    for (int i=NUM_VARIABLES-1; i>=0; i--) {
        movements[last_place][i] = state%10;
        state /= 10;
    }
}

void print_agents(struct agent* agents, int size) {
    for (int i=0; i<size; i++) {
        printf("Agent %d: ", i);
        print_dna(agents[i].dna);
        printf("\n");
    }
}

void print_dna(int* dna) {
    printf("[");
    for (int i=0; i<NUM_POSSIBLE_STATES; i++) {
        printf("%d",dna[i]);
        if (i!=NUM_POSSIBLE_STATES-1) printf(",");
    }
    printf("]");
}

void print_board(int ** board, int board_width, int board_height) {
    print_line((2*board_width)+2,'_');

    for (int i=0; i<board_height; i++) {
        printf("|");
        for (int j=0; j<board_width; j++) {
            printf("%d",board[i][j]);
            if (j!=board_width-1) printf(",");
        }
        printf("|\n");
    }

    print_line((2*board_width)+2, '_');
}

void print_line(int size, char symbol) {
    for (int i=0; i<size; i++) {
        printf("%c",symbol);
    }
    printf("\n");
}

void print_matrix_agent(int ** matrix) {

}

int power(int num, int pow) {
    int result = 1;

    for (int i=0; i<pow; i++) {
        result *= num;
    }

    return result;
}