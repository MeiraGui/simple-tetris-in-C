#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "tetris.h"

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <termios.h>
    struct termios orig_termios; // Original terminal settings to restore
#endif

/*
 * Global variables
 */

char running = 1;            // Boolean flag for main loop excecution
char paused = 0;             // Boolean flag for pausing the game

char line, column;           // Used for the positioning of the pieces
char rot = 0, pieceID = 0;   // Rotation of the piece (0-3) and which one it is (0-6)
char nextID = 0;             // ID used to display the upcoming piece

unsigned char clearedLines = 0; // Counter for every line cleared, for every 10 lines, a level increases
unsigned char level = 0;        // Level the game is on, determines how fast a piece drops

char timer = 0;              // Timer for dropping the piece over time
char hitFloor = 0;           // Boolean flag to check if the piece has hit the floor

char dropCount = 0;          // Counter used to give score for quickly dropping a piece (both soft and hard dropping)
int  score = 0;              // Score earned when playing the game, fairly self-explanatory

// Empty tile ' .' Filled Tile '[]'
char Grid[32][32] = {};      // The "play area" of the game, only a 20x20 area is displayed

// Based on the Game Boy version
char fallSpeeds[21] = {53, 49, 45, 41, 37, 33, 28, 22, 17, 11, 10, 9, 8, 7, 6, 6, 5, 5, 4, 4, 3};

unsigned long idum = 0;      // Essentially functions as the "seed" of the random float generator

// Pseudo-random random float generator from "Numerical Recipes in C" (page 285)
float randomFloat() {
	unsigned long itemp;
	
	static unsigned long jflone = 0x3f800000;
	static unsigned long jflmsk = 0x007fffff;
	
	idum = 1664525L*idum + 1013904223L;
	itemp = jflone | (jflmsk & idum);
	
	return (*(float *)&itemp) - 1.0f;	
}

int main() {
    initTerminal();

    // Framerate variables
#ifdef _WIN32
    // Windows timing runs slower, so we double the framerate to get close to Linux's speed
    const int FPS = 60;
#else
    const int FPS = 30;
#endif
    const int FRAME_DELAY = 1000000 / FPS; // microseconds per frame
    struct timespec start_time, end_time;
    
    // Title screen
    while (1) {
        displayTitle();
        unsigned char input = getKeys();

        // Enter press check
        if (input == 250) {
            break;
        }

        usleep(1000000 / 6);
    }

    idum = time(NULL); // Sets the seed to the current time

resetPoint:
    // Clears the title screen
    printf("\e[H\e[J");
    fflush(stdout);

    score = 0; rot = 0; level = 0; clearedLines = 0;

    // Main loop initialization
    initializeGrid();
    displayGrid();

    line = 10; column = 16; // Initial position of the piece
    pieceID = ((char)(randomFloat() * 7));
    nextID = ((char)(randomFloat() * 7));

    // Main loop
    while (running) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);
    
        if (!paused) {
            gameLogic();
        } else {
            unsigned char input = getKeys();
            printf("        Paused!\n");
            printf("(\e[38;5;227m'R'\e[39m to reset or \e[38;5;227m'Q'\e[39m to exit)\n");
            // Pressing enter unpauses the game
            if (input == 250) {
                paused = 0;
                printf("\e[1J"); // Clears the pause text when the game is unpaused
            } else if (input == 'r' || input == 'R') {
                paused = 0;
                goto resetPoint;
            } else if (input == 'q' || input == 'Q') {
                running = 2; // Set to 2 to not go into the game over code
                break;
            }
        }

        renderFrame(1);

        // Framerate regulation
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000 + 
                          (end_time.tv_nsec - start_time.tv_nsec) / 1000;
        
        if (elapsed_us < FRAME_DELAY) {
            usleep(FRAME_DELAY - elapsed_us);
        }
    }

    // Game Over
    if (running != 2) {
        printf("      \e[38;5;160m Game Over!\e[39m\n");
        printf("(\e[38;5;227m'R'\e[39m to reset or \e[38;5;227m'Q'\e[39m to exit)\n");

        while(1) {
            unsigned char input = getKeys();
            
            if (input == 'r' || input == 'R') {
                running = 1;
                goto resetPoint;
            } else if (input == 'q' || input == 'Q') {
                break;
            }
        }
    }

    restoreTerminal();
    return 0;
}

/*
 * Tetris functions
 */
char offset = 0; // Switches the colors of each letter
void displayTitle() {
    const char color[] = {'R', 'O', 'Y', 'G', 'C', 'B', 'M', 'D'};
    const char letter[] = {'T', 'e', 't', 'r', 'i', 's', '!', '\0'};
    
    // H ANSII comand moves the cursor to the top left of the screen
    printf("\e[H");

    printf("        ");
    for (int i = 0; i < 7; i++) {
        switch(color[(i+offset)&7]) {
            case 'R': printf("\e[38;5;160m"); break; // Red
            case 'G': printf("\e[38;5;34m");  break; // Green
            case 'Y': printf("\e[38;5;184m"); break; // Yellow
            case 'B': printf("\e[38;5;27m");  break; // Blue
            case 'M': printf("\e[38;5;129m"); break; // Magenta
            case 'C': printf("\e[38;5;80m");  break; // Cyan
            case 'O': printf("\e[38;5;172m"); break; // Orange
            default:  printf("\e[39m");       break; // Default color
        }
        printf("%c", letter[i]);
    }

    printf("\e[39m"); // Default color
    printf("\n  Press \e[38;5;227mENTER\e[39m to start.");

    offset = (offset - 1) & 7;

    fflush(stdout);
}

void gameLogic() {
    if (hitFloor) {
        char newID = ((char)(randomFloat() * 7));
        
        checkLines();
        
        // If the 'hitFloor' flag is set, a new piece is created at the top of the screen
        line = 10; column = 16; rot = 0;
        
        // Tries to ensure the new piece that drops is different from both visible pieces
        for (int i = 0; i < 5; i++) {
            if (newID == pieceID || newID == nextID) {
                newID = ((char)(randomFloat() * 7));
            } else {
                break;
            }
        }

        pieceID = nextID;
        nextID = newID;
        hitFloor = 0; dropCount = 0;
        // Check if there are tiles where the piece spawns, if there are, it's game over
        if (checkBoundaries() != 0) {
            running = 0;
            return;
        }
    } else {
        // If the hitFloor flag isn't set, move the piece normally
        clearPiece();
    }

    // Every 10 lines the level increases
    level = clearedLines / 10;

    handleInput(); // Input handling
    
    // Every couple of frames the piece automatically moves down a line
    if (timer++ > fallSpeeds[level]) {
        timer = 0;
        line++;

        // Finding a non empty tile while moving down means a floor has been hit
        if (checkBoundaries() != 0) {
            line--;
            hitFloor = 1;
            if (dropCount > 1) score += dropCount;
        }
    }
}

void renderFrame(char flag) {
    // H ANSII comand moves the cursor to the top left of the screen
    printf("\e[H");

    // the flag is to not display the piece after it drops, to not have issues with clearing lines
    if (flag) displayPiece();
    displayNextPiece();
    // Used to show the center of the piece
    //Grid[line][column] = '{';
    //Grid[line][column+1] = '}';
    displayGrid();

    fflush(stdout);
}

void initializeGrid() {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j += 2) {
            Grid[i][j] = '!';
            Grid[i][j+1] = '!';
        }
    }

    for (int i = 8; i < 30; i++) {
        for (int j = 6; j < 26; j += 2) {
            Grid[i][j] = ' ';
            Grid[i][j+1] = '.';
        }
    }
}

void displayGrid() {
#if 0
    // shows whole grid including OOB
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j += 2) {
            printf("%c%c", Grid[i][j],Grid[i][j+1]);
        }
        printf("\n");
    }
#else
    printf("   \e[38;5;227mNext\e[39m\n");
    printf(NEXT_TOP);
    for (int i = 4; i < 6; i++) {
        printf(NEXT_MID);
        for (int j = 4; j < 12; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("\e[39m"); // Default color
        printf(NEXT_MID);
        if (i == 4) printf(" Score: \e[38;5;227m%d\e[39m", score);
        if (i == 5) printf(" Level: \e[38;5;227m%d\e[39m", level);
        printf("\n");
    }
    printf(NEXT_BOTTOM);
    printf(DIV_LINE);

    for (int i = 10; i < 30; i++) {
        printf("\e[38;5;68m"); // Bright blue BG
        printf(MAIN_LEFT);
        for (int j = 6; j < 26; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf(MAIN_RIGHT);
    }
    printf(MAIN_BOTTOM);
    printf("\e[39m"); // Default color
#endif
}

void pieceColor(char color) {
    switch(color) {
        case 'R': printf("\e[38;5;160m"); break; // Red
        case 'G': printf("\e[38;5;34m");  break; // Green
        case 'Y': printf("\e[38;5;184m"); break; // Yellow
        case 'B': printf("\e[38;5;27m");  break; // Blue
        case 'M': printf("\e[38;5;129m"); break; // Magenta
        case 'C': printf("\e[38;5;80m");  break; // Cyan
        case 'O': printf("\e[38;5;172m"); break; // Orange
        default:  printf("\e[39m");       break; // Default color
    }
#if BLOCK_STYLE
    printf("██");
#else
    printf("[]");
#endif
    printf("\e[38;5;68m"); // Bright blue for the non piece tiles like the original IBM PC version
}

void handleInput() {   
    char downPressed = 0;
    
    switch(getKeys()) {
        case 250: // Enter press
            paused = 1;
            return;
        case 251: // Up
            // Hard dropping
            dropCount = 0;
            while (checkBoundaries() == 0) {
                line++;
                dropCount++;
            }
            line--;
            // Setting the timer to the limit is a bit janky but ensures
            // the floor hit check is done immediately while hard dropping
            timer += fallSpeeds[level] + 1;
            if (dropCount > 1) score += dropCount;
            break;
        case 252: // Down
            // Soft dropping
            timer += fallSpeeds[level] + 1; // Same thing as hard dropping 
            downPressed = 1;
            break;
        case 253: // Right
            column += 2;
            if (checkBoundaries() != 0) {
                column -= 2;
            }
            break;
        case 254: // Left
            column -= 2;
            if (checkBoundaries() != 0) {
                column += 2;
            }
            break;
        case 'z': case 'Z':
            rot++;
            rot &= 3;
            if (checkBoundaries() != 0) {
                    rot--;
            }
            break;
        case 'x': case 'X':
            rot--;
            rot &= 3;
            if (checkBoundaries() != 0) {
                    rot++;
            }
            break;
    }

    // ANDed to 3 to make sure the rotations don't get out of bounds
    rot &= 3;

    // ANDed to 0x1F to make sure the pieces wrap around if they get out of bounds
    line &= 0x1F;
    column &= 0x1F;

    if (downPressed == 1) {
        //printf("down pressed!\n");
        dropCount++;
    } else {
        //printf("down not pressed!\n");
        dropCount = 0;
    }
    
#ifndef _WIN32
    // There can be some issues with holding down on linux so the input buffer is cleared
    tcflush(STDIN_FILENO, TCIFLUSH);
#endif

    return;
}

#define centerLine 2
#define centerColumn 2

void displayPiece() {    
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                Grid[line+i][column+j] = pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)];
                Grid[line+i][column+(j+1)] = pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)];
            }
        }
    }
}

void displayNextPiece() {   
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            Grid[4+i][8+j] = pieceTable[nextID][0][centerLine+i][centerColumn+(j/2)];
            Grid[4+i][8+(j+1)] = pieceTable[nextID][0][centerLine+i][centerColumn+(j/2)];
        }
    }
}

void clearPiece() {  
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                Grid[line+i][column+j] = ' ';
                Grid[line+i][column+(j+1)] = '.';
            }
        }
    }
}

char checkBoundaries() { 
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                if (Grid[line+i][column+j] != ' ') {
                    return 1;
                }
            }
        }
    }

    return 0;
}

char foundLinesArr[4] = {};

void checkLines() {
    char linesFound = 0;
    
    for (int i = -2; i < 2; i++) {
        // Check to not remove the OOB tiles under the play area
        if (line+i > 29) {
            break;
        }
        for (int j = 6; j < 26; j += 2) {
            
            if (Grid[line+i][j] == ' ') {
                break;
            }
            if (j == 24) {
                clearLine(line+i);
                linesFound++;
                clearedLines++;
                // Line - 1 because the line that was cleared will never be moved down, since it has no tiles
                foundLinesArr[linesFound] = line+i - 1;
            }
        }
    }

    // Score changes based on line count and level, like the NES version
    switch (linesFound) {
        case 1: score += 40 * (level + 1);   break;
        case 2: score += 100 * (level + 1);  break;
        case 3: score += 300 * (level + 1);  break;
        case 4: score += 1200 * (level + 1); break;
    }

    // Pauses briefly to showcase the cleared lines before moving down and to give slight breathing room
    renderFrame(0);
    usleep(1000000 / 5);
    // The input buffer is cleared to avoid issues with holding down during this pause
#ifdef _WIN32
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#else
    tcflush(STDIN_FILENO, TCIFLUSH);
#endif

    if (linesFound) {
        moveLinesDown(linesFound);
    }
}

void clearLine(int line) {
    // 'i' is the column in this case
    for (int i = 6; i < 26; i += 2) {
        Grid[line][i] = ' ';
        Grid[line][i+1] = '.';
    }
}

void moveLinesDown(int linesFound) {
    for (int i = 0; i < linesFound; i++) {
        for (int i = foundLinesArr[linesFound]; i >= 8; i--) {
            for (int j = 6; j < 26; j += 2) {
                // If a non blank tile is found...
                if (Grid[i][j] != ' ') {
                    // And the tile below the non blank tile is blank...
                    if (Grid[i+1][j] == ' ') {
                        // Move the non blank tile to the empty one
                        Grid[i+1][j] = Grid[i][j];
                        Grid[i+1][j+1] = Grid[i][j+1];
                        // And then blank out the tile its currently in
                        Grid[i][j] = ' ';
                        Grid[i][j+1] = '.';
                    }
                }
            }
        }
    }
}

/*
 * Terminal functions (changes based on OS)
 */

// Windows
#ifdef _WIN32
// Enables ANSI support for the older terminal (Windows console) to work
void initTerminal() {
    // Gets the output handle
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        printf("Error getting output handle!\n");
        exit(1);
    }

    // Gets the current mode of the console
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        printf("Error getting console mode!\n");
        exit(1);
    }

    // Enables the virtual terminal prcessing flag for ANSI support
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        printf("Error activating ANSI support!\n");
        exit(1);
    }

    // Code page is changed to support the "extended ASCII" characters
    if (!SetConsoleCP(65001) || !SetConsoleOutputCP(65001)) {
        printf("Error setting code page!");
        exit(1);
    }

    printf("\e[?25l"); // Disables flashing cursor for cleaner display
    printf("\e[2J\e[H"); // Clears the screen once at the start
}

// Just enables the cursor, as there is no need to set a "raw mode" on Windows
void restoreTerminal() {
    printf("\e[?25h"); // Reactivates cursor on exit
}

// A function to normalize the keys is needed since both OSes handle input differently 
unsigned char getKeys() {
    unsigned char input;

    if (_kbhit()) {
        input = _getch();
        
        // Enter press check
        if (input == 13) {
            return 250;
        }

        // Arrow press check
        if (input == 0 || input == 224) {
            input = _getch();

            switch (input) {
                case 72: return 251; // Up
                case 80: return 252; // Down
                case 77: return 253; // Right
                case 75: return 254; // Left
            }
        }

        return input;
    }

    return 0;
}

// Linux
#else
// Changes the terminal mode to raw so it reads inputs directly
void initTerminal() {
    // Current terminal mode is saved and copied to the new mode struct
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios new_mode = orig_termios;
    // Clearing the ICANON so it doesn't send text as buffers,
    // and ECHO is cleared so it doesn't show the inputs onscreen 
    new_mode.c_lflag &= ~(ICANON | ECHO);
    // Sets the mode immediately with TCSANOW
    tcsetattr(STDIN_FILENO, TCSANOW, &new_mode);
    // Sets the NONBLOCK flag so it doesn't wait for input
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    printf("\e[?25l"); // Disables flashing cursor for cleaner display
    printf("\e[2J\e[H"); // Clears the screen once at the start
}

// Changes the terminal mode to the regular procedure, reading inputs as buffers
void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\e[?25h"); // Reactivates cursor on exit
}

// A function to normalize the keys is needed since both OSes handle input differently 
unsigned char getKeys() {
    unsigned char input;

    while (read(STDIN_FILENO, &input, 1) > 0) { 
        // Enter press check
        if (input == '\n') {
            return 250;
        }

        // Arrow key presses are 'ESC' (ASCII 27), '[' then 'A', 'B', 'C' or 'D'.
        // So to check for arrow keys, we check for those 3 inputs in sequence
        if (input == '\e') {
            input = getchar();

            if (input == '[') {
                input = getchar();

                switch (input) {
                    case 'A': return 251; // Up
                    case 'B': return 252; // Down
                    case 'C': return 253; // Right
                    case 'D': return 254; // Left
                }
            }
        }

        return input;
    }

    return 0;
}
#endif
