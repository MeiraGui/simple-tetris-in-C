#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "tetris.h"

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #include <timeapi.h>
#else
    #include <termios.h>
    #include <signal.h>
    struct termios orig_termios; // Original terminal settings to restore
#endif

/*
 * Global variables
 */

char running = 1;            // Boolean flag for main loop excecution
char paused = 0;             // Boolean flag for pausing the game

char gameStyle = 1;          // Changes the visuals (0 for Elektronika 60 style, 1 for cleaner)
char showGhost = 1;          // Boolean flag for showing the ghost piece

char line, column;           // Used for positioning the regular piece
char ghLine;                 // Used for positioning the ghost piece
char rot = 0, pieceID = 0;   // Rotation of the piece (0-3) and which one it is (0-6)
char nextID = 0;             // ID used to display the upcoming piece

unsigned char clearedLines = 0; // Counter for every line cleared, for every 10 lines, a level increases
unsigned char level = 0;        // Level the game is on, determines how fast a piece drops

char timer = 0;              // Timer for dropping the piece over time
char hitFloor = 0;           // Boolean flag to check if the piece has hit the floor

char dropCount = 0;          // Counter used to give score for quickly dropping a piece (both soft and hard dropping)
unsigned int  score = 0;     // Score earned when playing the game, fairly self-explanatory

// Empty tile ' .' Filled Tile '[]'
char Grid[32][32] = {};      // The "play area" of the game, only a 20x20 area is displayed

// Based on the Game Boy version
char fallSpeeds[26] = {52, 48, 44, 40, 36, 32, 27, 21, 16, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1};

unsigned long idum = 0;      // Essentially functions as the "seed" of the random float generator

char screenBuffer[4096];     // All of the text is stored in this buffer before being render to the screen at once

int main() {
    initTerminal();

    // Full buffer is enabled so it only prints to the screen when fflush is called
    setvbuf(stdout, screenBuffer, _IOFBF, sizeof(screenBuffer));

    // Framerate variables
    const int FPS = 60;
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

        if (input == 'P') {
            gameStyle ^= 1;
        }

        usleep(1000000 / 6);
    }

    idum = time(NULL); // Sets the seed to the current time

    // Making showGhost the gameStyle ID makes the Eletronika 60 style not have ghost pieces (it's ID 0)
    showGhost = gameStyle;

resetPoint:
    // Clears the title screen
    printf("\e[H\e[J");
    fflush(stdout);

    score = 0; rot = 0; level = 0; clearedLines = 0;
    dropCount = 0; timer = 0;

    // Main loop initialization
    initializeGrid();
    displayGrid();

    line = 10; column = 16; // Initial position of the piece
    ghLine = line; 
    pieceID = randomPieceID();
    nextID = randomPieceID();

    // Main loop
    while (running) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        if (!paused) {
            gameLogic();
        } else {
            unsigned char input = getKeys();
            printf("        Paused!\n");
            printf("(%s'R'%s to reset or %s'Q'%s to exit)\n", highlight[gameStyle], regular[gameStyle], highlight[gameStyle], regular[gameStyle]);
            // Pressing enter unpauses the game
            if (input == 250) {
                paused = 0;
                printf("\e[1J"); // Clears the pause text when the game is unpaused
            } else if (input == 'R') {
                paused = 0;
                goto resetPoint;
            } else if (input == 'Q') {
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
        printf("      %s Game Over!%s\n", gameOver[gameStyle], regular[gameStyle]);
        printf("(%s'R'%s to reset or %s'Q'%s to exit)\n", highlight[gameStyle], regular[gameStyle], highlight[gameStyle], regular[gameStyle]);
        fflush(stdout);

        while(1) {
            unsigned char input = getKeys();
            
            if (input == 'R') {
                running = 1;
                goto resetPoint;
            } else if (input == 'Q') {
                break;
            }
        }
    }

    restoreTerminal();
    printf("\e[39m"); // Restores color to default at exit
    return 0;
}

/*
 * Tetris functions
 */
char offset = 0; // Switches the colors of each letter
void displayTitle() {
    static const char *color[] = {
        "\e[38;5;160m", // Red
        "\e[38;5;172m", // Orange
        "\e[38;5;184m", // Yellow
        "\e[38;5;34m",  // Green
        "\e[38;5;80m",  // Cyan
        "\e[38;5;27m",  // Blue
        "\e[38;5;129m", // Magenta
        "\e[39m"        // Default color
        };
    static const char letter[] = {"Tetris!"};
    
    // H ANSII comand moves the cursor to the top left of the screen
    printf("\e[H");

    printf("        ");
    for (int i = 0; i < 7; i++) {
        printf("%s", color[(i+offset) & 7]);
        printf("%c", letter[i]);
    }

    printf("\e[39m"); // Default color
    printf("\n\n  Press \e[38;5;227mENTER\e[39m to start.");

    printf("\n\nGame style (\e[38;5;227mP\e[39m to swap): ");
    pieceColor('R');

    offset = (offset - 1) & 7;

    fflush(stdout);
}

void gameLogic() {
    if (hitFloor) {
        char newID = randomPieceID();
        
        checkLines();
        
        // If the 'hitFloor' flag is set, a new piece is created at the top of the screen
        line = 10; column = 16; rot = 0;
        
        // Tries to ensure the new piece that drops is different from both visible pieces
        for (int i = 0; i < 5; i++) {
            if (newID == pieceID || newID == nextID) {
                newID = randomPieceID();
            } else {
                break;
            }
        }

        pieceID = nextID;
        nextID = newID;
        hitFloor = 0; dropCount = 0;
        // Check if there are tiles where the piece spawns, if there are, it's game over
        if (checkBoundaries(0) != 0) {
            running = 0;
            ghLine = line; // Fixes a slight visual bug with the ghost piece on game over
            return;
        }
    } else {
        // If the hitFloor flag isn't set, move the piece normally
        clearPiece(0); // Regular piece
        clearPiece(1); // Ghost piece
    }

    // Every 10 lines the level increases
    level = clearedLines / 10;

    handleInput(); // Input handling
    
    // Moves the ghost piece, essentially "hard dropping" it to the lowest position
    ghLine = line; 

    while (checkBoundaries(1) == 0) {
        ghLine++;
    }
    ghLine--;

    // Every couple of frames the piece automatically moves down a line
    if (timer++ > fallSpeeds[level]) {
        timer = 0;
        line++;

        // Finding a non empty tile while moving down means a floor has been hit
        if (checkBoundaries(0) != 0) {
            line--;
            dropCount--; // - 1 because it counts 1 extra line for the floor collision check
            hitFloor = 1;
            if (dropCount > 0) score += dropCount;
            dropCount = 0;
        }
    }
}

// Modified version of the Pseudo-random random float generator from "Numerical Recipes in C" (page 285),
// "Numerical Recipes in C" (page 285), to use integers instead of floats
char randomPieceID() {
    idum = 1664525L*idum + 1013904223L;

    // The high bits are the most random, so we use those
    unsigned int highBits = (unsigned int)idum >> 16;

    return (char)(highBits % 7);
}

void renderFrame(char flag) {
    // H ANSII comand moves the cursor to the top left of the screen
    printf("\e[H");

    // the flag is to not display the piece after it drops, to not have issues with clearing lines
    if (flag) {
        if (showGhost) displayPiece(1); // Ghost piece
        displayPiece(0); // Regular piece
        // Used to show the center of the regular piece
        //Grid[line][column] = '{';
        //Grid[line][column+1] = '}';
    }
    displayNextPiece(); // Next piece
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
    printf("   %sNext%s\n", highlight[gameStyle], regular[gameStyle]);
    printf("%s", visualsTable[gameStyle][0]); // Next outline top
    for (int i = 4; i < 6; i++) {
        printf("%s", visualsTable[gameStyle][1]); // Next outline middle
        for (int j = 4; j < 12; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("%s", regular[gameStyle]); // Default color
        printf("%s", visualsTable[gameStyle][1]); // Next outline middle
        if (i == 4) printf(" Score: %s%d%s", highlight[gameStyle], score, regular[gameStyle]);
        if (i == 5) printf(" Level: %s%d%s", highlight[gameStyle], level, regular[gameStyle]);
        printf("\n");
    }
    printf("%s", visualsTable[gameStyle][2]); // Next outline bottom
    printf("%s", visualsTable[gameStyle][3]); // Dividing line

    for (int i = 10; i < 30; i++) {
        // Non piece tiles can either be bright blue like the IBM PC version or a bright green
        printf("%s", playArea[gameStyle]);
        printf("%s", visualsTable[gameStyle][4]); // Play area left
        for (int j = 6; j < 26; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("%s", visualsTable[gameStyle][5]); // Play area right
    }
    printf("%s", visualsTable[gameStyle][6]); // Play area bottom
    printf("%s", regular[gameStyle]); // Default color
#endif
}

void pieceColor(char color) {
    char flag = 0;

    // Check to make the ghost piece show up as a transparent block
    if (color == 'H') {
        flag = 1;
        color = Grid[line][column]; // Color is copied for the current piece so it isn't white
    }

    if (gameStyle) {
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
        printf(flag ? "░░" : "██");
    } else {
        printf("\e[38;5;46m"); // Bright green
        printf("[]");
    }

    // Non piece tiles can either be bright blue like the IBM PC version or a bright green
    printf("%s", playArea[gameStyle]);
}

void handleInput() {       
    switch(getKeys()) {
        case 250: // Enter press
            paused = 1;
            return;
        case 251: // Up
            // Hard dropping
            dropCount = 0;
            while (checkBoundaries(0) == 0) {
                line++;
                dropCount++;
            }
            line--;
            // Setting the timer to the limit is a bit janky but ensures
            // the floor hit check is done immediately while hard dropping
            timer += fallSpeeds[level] + 1;
            break;
        case 252: // Down
            // Soft dropping
            timer += fallSpeeds[level] + 1; // Same thing as hard dropping 
            dropCount++; // Holding down increases the dropCount
            break;
        case 253: // Right
            column += 2;
            if (checkBoundaries(0) != 0) {
                column -= 2;
            }
            break;
        case 254: // Left
            column -= 2;
            if (checkBoundaries(0) != 0) {
                column += 2;
            }
            break;
        case 'Z':
            rot++;
            rot &= 3;
            if (checkBoundaries(0) != 0) {
                    rot--;
            }
            break;
        case 'X':
            rot--;
            rot &= 3;
            if (checkBoundaries(0) != 0) {
                    rot++;
            }
            break;
        case 'G':
            // Doing an XOR with the gameStyle variable directly makes it so
            // the Eletronika 60 style (ID 0) can't toggle showGhost on 
            showGhost ^= gameStyle;
            break;
    }

    // ANDed to 3 to make sure the rotations don't get out of bounds
    rot &= 3;

    // ANDed to 0x1F to make sure the pieces wrap around if they get out of bounds
    line &= 0x1F;
    column &= 0x1F;

    return;
}

#define centerLine 2
#define centerColumn 2

// 0 = regular piece, 1 = ghost piece
void displayPiece(char flag) {    
    char dLine = flag ? ghLine : line;

    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            char pieceChar = flag ? 'H' : pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)];
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                Grid[dLine+i][column+j] = pieceChar;
                Grid[dLine+i][column+(j+1)] = pieceChar;
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

// 0 = regular piece, 1 = ghost piece
void clearPiece(char flag) {  
    char cLine = flag ? ghLine : line;
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                Grid[cLine+i][column+j] = ' ';
                Grid[cLine+i][column+(j+1)] = '.';
            }
        }
    }
}

// 0 = regular piece, 1 = ghost piece
char checkBoundaries(char flag) { 
    char bLine = flag ? ghLine : line;
    
    for (int i = -2; i < 2; i++) {
        for (int j = -4; j < 4; j += 2) {
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                if (Grid[bLine+i][column+j] != ' ') {
                    return 1;
                }
            }
        }
    }

    return 0;
}

// Stores the found lines so only the lines above them are moved downward
char foundLinesArr[4] = {};

void checkLines() {
    char linesFound = 0;
    
    // Only checks the 4 lines around the center, since those are the only ones that can have cleared lines
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
                // Line - 1 because the line that was cleared will never be moved down, since it has no tiles
                foundLinesArr[linesFound] = line+i - 1;
                linesFound++;
                clearedLines++;
            }
        }
    }

    // Score changes based on line count and level, like the NES version
    static const int baseScore[] = {0, 40, 100, 300, 1200};
    score += baseScore[linesFound] * (level + 1);

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
    for (int l = 0; l < linesFound; l++) {
        for (int i = foundLinesArr[l]; i >= 8; i--) {
            for (int j = 6; j < 26; j += 2) {
                // Move every tile down
                Grid[i+1][j] = Grid[i][j];
                Grid[i+1][j+1] = Grid[i][j+1];
                // And then blank out the tile it's currently in
                Grid[i][j] = ' ';
                Grid[i][j+1] = '.';
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

    timeBeginPeriod(1); // Extra timing precision

    printf("\e[?25l"); // Disables flashing cursor for cleaner display
    printf("\e[2J\e[H"); // Clears the screen once at the start
}

// Enables the cursor and returns the timing precision to normal
void restoreTerminal() {
    timeEndPeriod(1);
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

        if (input >= 'a' && input <= 'z') { 
            return input & ~32; // Turns lowercase characters into uppercase
        }

        return input;
    }

    return 0;
}

// Linux
#else
// Reverts the terminal changes if the game is closed early with ctrl C
void handleSignal() {
    restoreTerminal();
    exit(0);
}

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

    // Used to safely restore terminal settings if the game is closed early
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

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
            if (read(STDIN_FILENO, &input, 1) > 0 && input == '[') {
                if (read(STDIN_FILENO, &input, 1) > 0) {
                    switch (input) {  
                        case 'A': return 251; // Up
                        case 'B': return 252; // Down
                        case 'C': return 253; // Right
                        case 'D': return 254; // Left
                    }
                }
            }
        }

        if (input >= 'a' && input <= 'z') { 
            return input & ~32; // Turns lowercase characters into uppercase
        }

        return input;
    }

    return 0;
}
#endif
