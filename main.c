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

#define NUM_BOARDS_FITNESS 3

//These are important game parameters, and you may tweak them to get better results.
//Keep in mind that making larger some of these numbers will take more time for the algorithm to compute.
#define NUM_SIMULATIONS_AGENT 3 //Number of simulations each agent will run in order to determine its fitness.
#define SPLIT_VALUE_DNA 0.5 //Relative place of the split in the DNA in the process of reproduction. (Value between 0 and 1.)

//All possible states of a board square.
#define WALL 2
#define OBJECT 3
#define NOTHING 1

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

//Board structure
struct board {
    int board_height;
    int board_width;
    int num_objects;

    int ** matrix;
};

//Function declarations
void initialize_states(int level, int state);
void setup_movements_int_state(int state);
int power(int num, int pow);
void setup_game_properties(int * board_width, int * board_height, int * population_size, double * mutation_probability, int * num_generations, int * min_object_num, int * max_object_num);
void please_enter_prompt(char * variable_name, int * variable_value);
int setup_board(int min_num_objects, int max_num_objects);

void print_board();
void print_line(int size, char symbol);
void generate_random_dna(int* dna);
void *generate_random_agents(void* arg);
void print_dna(int* dna);
void print_agents(struct agent* agents, int size);
void *calculate_fitness(void* arg);
int encode_array_int(const int* arr, int len);
void decode_int_array(int* arr, int len, int num);
int find_state(int x_val, int y_val);

void run_agent(struct agent *agent);
int binary_state_search(int state);
int evaluate_trait(struct agent *agent, int trait);

int try_up(struct agent *agent);
int try_right(struct agent *agent);
int try_down(struct agent *agent);
int try_left(struct agent *agent);

//Global variables
//Array of all possible states the agent can be in.
//Number of row is the place in the DNA sequence.
//Columns go like this: UP, RIGHT, DOWN, LEFT, CURRENT. (States of these squares.)
//Changed in the last version. This is now a vector such that the number in the column represents encoding of the states.
int movements[NUM_POSSIBLE_STATES];
int num_movements_per_board;

struct agent* population;
struct agent* new_population;
struct board game_board;
int main() {
    //Constants for loops.
    int i, j;

    //Threads.
    pthread_t pthread1, pthread2, pthread3, pthread4;

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
    int population_size;
    double mutation_probability;
    int number_of_generations;
    int min_object_number;
    int max_object_number;
    setup_game_properties(&game_board.board_width, &game_board.board_height, &population_size, &mutation_probability, &number_of_generations, &min_object_number, &max_object_number);

    num_movements_per_board = game_board.board_width*game_board.board_height*2;

    //Setting up the board will relevant parameters.
    //----Measuring the time----
    start = clock();
    //--------

    game_board.matrix = (int**) malloc(game_board.board_height*sizeof(int*));
    for (i=0; i < game_board.board_height; i++)
        game_board.matrix[i] = (int*)malloc(game_board.board_width * sizeof(int));
    setup_board(min_object_number, max_object_number);

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
    for (i=0; i<population_size; i++) {
        population[i].dna = (int*)malloc(NUM_POSSIBLE_STATES*sizeof(int));
    }
    //Dividing the process of setting the population to different threads.
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




    //----Measuring the time----
    start = clock();
    //--------

    //-------Main loop-------
    //Calculate fitness
    for (i=0; i<NUM_SIMULATIONS_AGENT; i++) {
        //Dividing the process of finding the fitness of the population to different threads.
        pthread_create(&pthread1,NULL,calculate_fitness,&pthread1_arg);

        pthread_create(&pthread2,NULL,calculate_fitness,&pthread2_arg);

        //Wait for threads to finish.
        pthread_join(pthread1,NULL);
        pthread_join(pthread2,NULL);

        setup_board(min_object_number, max_object_number);
    }

    //----Measuring the time----
    end = clock();
    cpu_time_used = ((double) (end-start))/CLOCKS_PER_SEC;
    //--------

    printf("\nEvaluating fitness of the population took %lf seconds\n\n", cpu_time_used);

    print_agents(population, population_size);


    //------Freeing the Memory------
    //Freeing the memory from malloc usages.
    for (i=0;i<game_board.board_height;i++) {
        free(game_board.matrix[i]);
    }
    free(game_board.matrix);

    //Freeing the dna variables.
    for (i=0;i<population_size;i++) {
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
        struct agent* current_agent = &population[i];
        run_agent(current_agent);
        (*current_agent).fitness+=(((double)(*current_agent).points)/(game_board.num_objects*10))/NUM_BOARDS_FITNESS;
    }
}

void run_agent(struct agent *agent) {
    int state, dna_trait;

    for (int i=0; i<num_movements_per_board; i++) {
        state = find_state((*agent).x_place, (*agent).y_place);
        dna_trait = binary_state_search(state);
        (*agent).points += evaluate_trait(agent, dna_trait);
    }
}

int evaluate_trait(struct agent *agent, int trait) {
    //TODO: clean this function
    int collected_points = 0;

    switch ((*agent).dna[trait]) {
        case GO_UP:
            collected_points += try_up(agent);
            break;
        case GO_RIGHT:
            collected_points += try_right(agent);
            break;
        case GO_DOWN:
            collected_points += try_down(agent);
            break;
        case GO_LEFT:
            collected_points += try_left(agent);
            break;
        case GO_RANDOM:
            switch (rand()%4) {
                case 0:
                    collected_points += try_up(agent);
                    break;
                case 1:
                    collected_points += try_right(agent);
                    break;
                case 2:
                    collected_points += try_down(agent);
                    break;
                case 3:
                    collected_points += try_left(agent);
                    break;
            }
            break;
        case PICK_OBJECT:
            if (game_board.matrix[(*agent).x_place][(*agent).y_place]==OBJECT) {
                collected_points += 10;
                game_board.matrix[(*agent).x_place][(*agent).y_place]=NOTHING;
            } else
                collected_points -= 1;
            break;
    }
    return collected_points;
}

int try_up(struct agent *agent) {
    if ((*agent).y_place-1>=0) {
        (*agent).y_place -= 1;
        return 0;
    } else
        return -5;
}

int try_right(struct agent *agent) {
    if ((*agent).x_place+1<game_board.board_width) {
        (*agent).x_place += 1;
        return 0;
    } else
        return -5;
}

int try_down(struct agent *agent) {
    if ((*agent).y_place+1<game_board.board_height) {
        (*agent).y_place += 1;
        return 0;
    } else
        return -5;
}

int try_left(struct agent *agent) {
    if ((*agent).x_place-1>=0) {
        (*agent).x_place -= 1;
        return 0;
    } else
        return -5;
}

int binary_state_search(int state) {
    int start=0, end=NUM_POSSIBLE_STATES-1, mid=(end-start)/2;

    while (start<end) {
        if (movements[mid]==state)
            return mid;
        if (movements[mid]<state) {
            start=mid+1;
            mid=(end+start)/2;
        } else if (movements[mid]>state) {
            end=mid-1;
            mid=(end+start)/2;
        }
    }
    return -1;
}

int find_state(int x_val, int y_val) {
    int state[NUM_VARIABLES];

    //state 0 is the state of the "up" square.
    if (y_val==0) state[0] = WALL;
    else state[0] = game_board.matrix[x_val][y_val-1];

    //state 1 is the state of the "right" square.
    if (x_val==game_board.board_width-1) state[1] = WALL;
    else state[1] = game_board.matrix[x_val+1][y_val];

    //state 2 is the state of the "down" square.
    if (y_val==game_board.board_height-1) state[2] = WALL;
    else state[2] = game_board.matrix[x_val][y_val+1];

    //state 3 is the state of the "left" square.
    if (x_val==0) state[3] = WALL;
    else state[3] = game_board.matrix[x_val-1][y_val];

    //state 4 is the state of the current square.
    state[4] = game_board.matrix[x_val][y_val];
    return encode_array_int(state, NUM_VARIABLES);
}

int encode_array_int(const int* arr, int len) {
    int code = 0;
    for (int i=0;i<len;i++) {
        code = (code*10)+arr[i];
    }
    return code;
}

void decode_int_array(int* arr, int len, int num) {
    int i=len-1;
    while (num > 0 && i>=0) {
        arr[i]=num%10;
        num/=10;
        i--;
    }
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

int setup_board(int min_num_objects, int max_num_objects) {
    for (int i=0; i<game_board.board_height; i++) {
        int j=0;
        for (; j<game_board.board_width; j++) {
            game_board.matrix[i][j] = NOTHING;
        }
    }

    int num_objects = max_num_objects==min_num_objects ? max_num_objects : min_num_objects + (rand()%(max_num_objects-min_num_objects));
    game_board.num_objects=num_objects;
    int row, col;
    for (int i=0; i<num_objects; i++) {
        row = rand()%game_board.board_height;
        col = rand()%game_board.board_width;
        if (game_board.matrix[row][col] != NOTHING) i--;
        else game_board.matrix[row][col] = OBJECT;
    }
    return num_objects;
}

void setup_game_properties(int * board_width, int * board_height, int * population_size, double * mutation_probability, int * num_generations, int * min_object_num, int * max_object_num) {
    //TODO: Make the inputs more secure
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
        movements[last_place]=state;
        last_place++;
        return;
    }
    for (int i=1; i<=NUM_VARIATIONS; i++) {
        initialize_states(level+1, (state*10)+i);
    }
}

/*
 * Deprecated function.
void setup_movements_int_state(int state) {
    for (int i=NUM_VARIABLES-1; i>=0; i--) {
        movements[last_place][i] = state%10;
        state /= 10;
    }
}*/

void print_agents(struct agent* agents, int size) {
    for (int i=0; i<size; i++) {
        printf("Agent %d with fitness %lf: ", i, agents[i].fitness);
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

void print_board() {
    print_line((2*game_board.board_width)+2,'_');

    for (int i=0; i<game_board.board_height; i++) {
        printf("|");
        for (int j=0; j<game_board.board_width; j++) {
            printf("%d",game_board.matrix[i][j]);
            if (j!=game_board.board_width-1) printf(",");
        }
        printf("|\n");
    }

    print_line((2*game_board.board_width)+2, '_');
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