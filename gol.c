#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_PERIOD 4

/*Redefine alive and dead. Change boolean expressions for alive >= 0*/
#define DEAD -1
#define NEW 0

typedef struct{
    int** cells;
    int width;
    int height;
}game_board;

void print_board(game_board board,int printX);
game_board create_board(int width,int height);
int count_neighbours(int cell_row,int cell_col, game_board board);
void play_round(game_board current_board,game_board next_board);
void copy_board(game_board source_board,game_board dest_board);
void read_file (char* file_name,game_board board);
int board_cmp(game_board board1,game_board board2);
int calc_period(int cycle,int stored_pos);

void main(int argc, char *argv[]){
    
    /*Get the arguments from the command line*/
    int width = atoi(argv[2]); /*Converts string to int*/
    int height = atoi(argv[3]);
    int rounds = atoi(argv[4]);

    /*Check arguments*/
    if (!(argc == 5||argc == 6)){
        fprintf(stderr, "Not enough arguments");
        exit(-1);
    } else if (width<0||width>78) {
        fprintf(stderr, "Invalid width");
        exit(-1);
    } else if (height<0||height>50) {
        fprintf(stderr, "Invalid height");
        exit(-1);
    } else if (rounds<0) {
        fprintf(stderr, "Invalid rounds");
        exit(-1);
    }

    int printX = 0;
    if (argc == 6){
        printX = atoi(argv[5]);
    }

    /*Create two boards, one current and one for the next round*/
    game_board current_board = create_board(width,height);
    game_board next_board = create_board(width,height);

    /*Create an array that stores previous boards*/
    game_board history[MAX_PERIOD];
    
    read_file(argv[1],current_board);

    /*For each round/generation, print the board, determine the next board,
     *then copy the next board over to the current board. Exit if the current board copies any previous boards*/
    int round;
    int cycle = 0; /*Maximum of 4 boards are saved to history to detect up to a period of 4. 
                    *A variable is needed to rewrite the history array*/
    for (round=0;round<rounds;round++){

        game_board hist_board = create_board(width,height);

        print_board(current_board,printX);

        /*Check for period*/
        int n;
        if (round == 0){
            /*No period can be detected on the first round*/
        } else if (round == 1||round == 2||round == 3){ /*The first few rounds do not have 4 history boards*/
            /*Loop through the history boards and check if boards match*/
            for (n=0;n<round;n++){
                if (board_cmp(history[n],current_board) == 1){
                    printf("\nPeriod detected (%i): exiting",calc_period(cycle,n));
                    exit(1);
                }
            }
        } else if (round > 3){
            for (n=0;n<4;n++){
                if (board_cmp(history[n],current_board) == 1){
                    printf("\nPeriod detected (%i): exiting",calc_period(cycle,n));
                    exit(1);
                }
            }
        }

        copy_board(current_board,hist_board);
        history[cycle] = hist_board;
        play_round(current_board,next_board);
        copy_board(next_board,current_board);

        printf("\n");
        if (round == (rounds-1)){ /*If final round, print finished*/
            printf("Finished");
        } else {
            printf("\n");
        }

        /*Loops 0-3*/
        if (cycle < (MAX_PERIOD - 1)){
            cycle++;
        } else {
            cycle = 0;
        }
    }
}

/*Calculates period*/
int calc_period(int cycle,int stored_pos){
    if (cycle - stored_pos < 1){
        return ((cycle + MAX_PERIOD) - stored_pos);
    } else {
        return (cycle - stored_pos);
    }
}

/*Create new board with zeros*/
game_board create_board(int width,int height){
    int row;
    int col;
    int h;

    game_board new_board;

    new_board.width = width;
    new_board.height = height;
    new_board.cells = malloc(height*sizeof(int*));
    for(h=0;h<height;h++) {
        new_board.cells[h] = malloc(width*sizeof(int));
    }
    for(row=0;row<height;row++){
        for(col=0;col<width;col++){
            new_board.cells[row][col] = DEAD;
        }
    }
    return new_board;
}

/*Print board to screen*/
void print_board(game_board board,int printX){
    int row;
    int col;
    int i;
    
    /*Print top boarder*/
    printf("*");
    for (i=0;i<board.width;i++){
        printf("-");
    }
    printf("*\n");
    
    /*Print board contents*/
    for(row=0;row<board.height;row++){
        printf("|");
        for(col=0;col<board.width;col++){
            /*Add new condition to print X if lifespan is more than 10*/
            if (board.cells[row][col] >= 10 || (printX == 1 && board.cells[row][col] >= 0)){
                printf("X");
            } else if (board.cells[row][col] >= 0){
                printf("%i",board.cells[row][col]);
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    };

    /*Print bottom boarder*/
    printf("*");
    for (i=0;i<board.width;i++){
        printf("-");
    }
    printf("*\n");
}

/*Returns the number of live neighbour cells of a given cell*/
int count_neighbours(int cell_row,int cell_col, game_board board){
    int row;
    int col;
    int live_neighbours = 0;
    
    /*If cell is on the edge of the board, skip neighbours that don't exist*/
    for (row=-1;row<2;row++){
        if (row == -1 && cell_row == 0) continue;
        if (row == 1 && cell_row == (board.height-1)) continue;
        for (col=-1;col<2;col++){
            if (col == -1 && cell_col == 0) continue;
            if (col == 1 && cell_col == (board.width-1)) continue;
            if (col == 0 && row == 0) continue; /*Do not check the value of itself*/
            if (board.cells[cell_row + row][cell_col + col] >= 0) live_neighbours++;
        }
    }
    return live_neighbours;    
}

/*Loop through board and determine if each cell will live or die*/
void play_round(game_board current_board,game_board next_board){
    int row;
    int col;
    for(row=0;row<current_board.height;row++){
        for(col=0;col<current_board.width;col++){
            int live_neighbours = count_neighbours(row,col,current_board);
            if (current_board.cells[row][col] >= 0){ /*If cell is living*/
                if (live_neighbours < 2){ /*Dies by underpopulation*/
                    next_board.cells[row][col] = DEAD;
                } else if (live_neighbours == 2||live_neighbours == 3){ /*Continue living and increment lifespan*/
                    next_board.cells[row][col] = (current_board.cells[row][col] + 1);
                } else if (live_neighbours > 3){ /*Dies by overcrowding*/
                    next_board.cells[row][col] = DEAD;
                }
            } else { /*If cell is dead*/
                if (live_neighbours == 3){ /*Dead cell revived*/
                    next_board.cells[row][col] = NEW;
                } else { /*Dead cell continues dead*/
                    next_board.cells[row][col] = DEAD;
                }
            }
        }
    }
}

/*Copy the contents of a board from one to another*/
void copy_board(game_board source_board,game_board dest_board){
    int row;
    int col;
    for(row=0;row<source_board.height;row++){
        for(col=0;col<source_board.width;col++){
            dest_board.cells[row][col] = source_board.cells[row][col];
        }
    }
}

/*Initialise a board based on a file input*/
void read_file (char* file_name,game_board board){
    
    FILE *fptr;
    fptr = fopen(file_name,"r");
    
    if(fptr == NULL) {
        fprintf(stderr,"Error opening file");
        exit(-1);
    }

    /*Read first line which determines the amount of cells*/
    char line[10];
    int count = 0;
    fgets(line,10,fptr);
    sscanf(line,"%i",&count);
    
    /*Read the positions of the initial cells*/
    int i;
    for (i=0;i<count;i++){
        char line[5];
        int cell_row;
        int cell_col;

        fgets(line,10,fptr);
        sscanf(line,"%i %i",&cell_row,&cell_col);

        /*Throw error if cells are placed outside of board*/
        if (cell_col<0||cell_col>(board.width-1)){
            fprintf(stderr,"Invalid cell placement");
            exit(-1);
        } else if (cell_row<0||cell_row>(board.height-1)){
            fprintf(stderr,"Invalid cell placement");
            exit(-1);
        }

        board.cells[cell_row][cell_col] = NEW;
    }
    
    fclose(fptr);
}

/*Compares two boards and returns 1 if they have the same pattern*/
int board_cmp (game_board board1,game_board board2){
    int row;
    int col;
    bool alive1 = false;
    bool alive2 = false;

    /*Compare boards on whether their cells are alive or dead*/
    for(row=0;row<board1.height;row++){
        for(col=0;col<board1.width;col++){
            if (board1.cells[row][col] >= 0){
                alive1 = true;              
            } else {
                alive1 = false;
            }
            if (board2.cells[row][col] >= 0){
                alive2 = true;              
            } else {
                alive2 = false;
            }

            if (alive1 != alive2){
                return false;
            }
        }
    }
    return true;
    
}