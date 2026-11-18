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
    #include <sys/ioctl.h>
    #include <stdarg.h>
    struct termios orig_termios; // Original terminal settings to restore
#endif

/*
 * Global variables
 */

char running = 1;              // Boolean flag for main loop excecution
char gameState = TITLE_STATE;  // Changes the state the game is in

char gameStyle = 1;            // Changes the visuals (0 for Elektronika 60 style, 1 for cleaner)
char showGhost = 1;            // Boolean flag for showing the ghost piece

char line, column;             // Used for positioning the regular piece
char ghLine;                   // Used for positioning the ghost piece
char rot = 0, pieceID = 0;     // Rotation of the piece (0-3) and which one it is (0-6)
char nextID = 0;               // ID used to display the upcoming piece

unsigned char clearedLines = 0; // Counter for every line cleared, for every 10 lines, a level increases
unsigned char level = 0;        // Level the game is on, determines how fast a piece drops

char timer = 0;                // Timer for dropping the piece over time
char hitFloor = 0;             // Boolean flag to check if the piece has hit the floor

char dropCount = 0;            // Counter used to give score for quickly dropping a piece (both soft and hard dropping)
unsigned int  score = 0;       // Score earned when playing the game, fairly self-explanatory

// Empty tile ' .' Filled Tile '[]'
char Grid[32][32] = {};        // The "play area" of the game, only a 20x20 area is displayed

unsigned long idum = 0;        // Essentially functions as the "seed" of the random float generator

char placement = -1;           // The placement of the player, if they get a new high score

HighScore highScores[10] = {}; // Array of the top 10 high scores, used for both displaying and ordering them

char screenBuffer[8192];       // All of the text is stored in this buffer before being render to the screen at once
int currWidth = 0;             // The current width of the terminal, used for horizontal padding
int currHeight = 0;            // The current height of the terminal, used to stop the game if the terminal is too small

// Terminal size constants
#define MIN_WIDTH 44
#define MIN_HEIGHT 30

// Framerate constants
#define FPS 60
#define FRAME_DELAY 1000000 / FPS // microseconds per frame

int main() {
    initTerminal();
    initSaveFile();

    // Full buffer is enabled so it only prints to the screen when fflush is called
    setvbuf(stdout, screenBuffer, _IOFBF, sizeof(screenBuffer));

    struct timespec start_time, end_time;
    
    idum = time(NULL); // Sets the seed to the current time

    int prevWidth = 0;

    while (running) {
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        getTerminalSize(&currWidth, &currHeight);
        // Clears the screen if the window size changes to (mostly) avoid visual artifacts when resizing
        if (prevWidth != currWidth) {
            printf("\e[H\e[J");
            prevWidth = currWidth;
        } else {
            printf("\e[H"); // H ANSI comand moves the cursor to the top left of the screen
        }

        // Checks if the screen is too small to avoid text wrapping
        if (currWidth < MIN_WIDTH || currHeight < MIN_HEIGHT) {
            printf("\e[HThe window is too small!\e[J");
            fflush(stdout);
            usleep(FRAME_DELAY);
            continue;
        }

        switch (gameState) {
            case TITLE_STATE:      titleLoop();     break;
            case HIGHSCORE_STATE:  highScoreLoop(); break;
            case GAME_STATE:       gameLoop();      break;
            case PAUSE_STATE:      pauseLoop();     break;
            case GAMEOVER_STATE:   gameOverLoop();  break;
        }

        // Framerate regulation
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        long elapsed_us = (end_time.tv_sec - start_time.tv_sec) * 1000000 + 
                          (end_time.tv_nsec - start_time.tv_nsec) / 1000;
        
        if (elapsed_us < FRAME_DELAY) {
            usleep(FRAME_DELAY - elapsed_us);
        }
    }

    restoreTerminal();
    printf("\e[39m"); // Restores color to default at exit
    return 0;
}

/*
 * Tetris functions
 */

unsigned char titleTimer = 0; // Used to slow down the color cycling of the "Tetris!" text
void titleLoop() {
    titleTimer++;
    
    displayTitle();

    unsigned char input = getKeys();

    // Enter press check
    if (input == 250) {
        resetGame();
        gameState = GAME_STATE;
    }

    if (input == 'P') {
        gameStyle ^= 1;
    }

    if (input == 'H') {
        gameState = HIGHSCORE_STATE;
    }
}

void highScoreLoop() {
    printf("\e[H\e[J");

    // New High Score
    if (placement != -1) {
        // Moves all positions after the placement down
        for (int i = 9; i > placement; i--) {
            highScores[i] = highScores[i-1];
        }

        printf("\e[H\e[J");

        // Displays records from the array
        for (int j = 0; j < 2; j++) {
            printPadded(14, "\e[38;5;227mNew High Score!\n\n");
            for (int i = 0; i < 10; i++) {
                if (i == placement) {
                    if (j == 0) {
                        // The first time drawing, make the name empty, and place the score and level the player got
                        printPadded(44,"\e[38;5;227m%2d. \e[39m%s Score: \e[38;5;227m%-9d \e[39mLevel: \e[38;5;227m%d\n", 
                            (i+1), "            ", score, level);
                    } else {
                        // The second time, stop early so the player writes the text directly on where it should be
                        printPadded(44,"\e[38;5;227m%2d. \e[39m", (i+1));
                        highScoreEntry(); 
                        goto loopExit;
                    }
                } else {
                    printPadded(44,"\e[38;5;227m%2d. \e[39m%s Score: \e[38;5;227m%-9d \e[39mLevel: \e[38;5;227m%d\n", 
                        (i+1), highScores[i].name, highScores[i].score, highScores[i].level);
                }
            }
            printf("\n");
            printPadded(30, "\e[39mType your name and press \e[38;5;227mENTER\e[39m.");
            printf("\e[H");
        }
        loopExit:
    } else { // Regular Display
        printPadded(12, "\e[38;5;227mHigh scores:\n\n");

        // Displays records from the array
        for (int i = 0; i < 10; i++) {
            printPadded(44, "\e[38;5;227m%2d. \e[39m%s Score: \e[38;5;227m%-9d \e[39mLevel: \e[38;5;227m%d\n", 
                (i+1), highScores[i].name, highScores[i].score, highScores[i].level);
        }
        printf("\n");
        printPadded(60, "\e[39mPress \e[38;5;227mENTER\e[39m to start a game or \e[38;5;227m'Q'\e[39m to go back to the title.");
        fflush(stdout);

        unsigned char input = getKeys();
        if (input == 'Q') {
            gameState = TITLE_STATE;
            printf("\e[H\e[J");
            return;
        }

        // Enter press check
        if (input == 250) {
            resetGame();
            gameState = GAME_STATE;
            return;
        }
    }
}

// Based on the Game Boy version
char fallSpeeds[26] = {52, 48, 44, 40, 36, 32, 27, 21, 16, 10, 9, 8, 7, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 1, 1};
void gameLoop() {
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
            gameState = GAMEOVER_STATE;
            ghLine = line; // Fixes a slight visual bug with the ghost piece on game over
            renderFrame(1);
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

    randomPieceID(); // Advances the RNG every frame for some extra variety
    renderFrame(1);
}

void pauseLoop() {
    renderFrame(1);
    printPadded(6, "Paused!\n");
    printPadded(28, "(%s'R'%s to reset or %s'Q'%s to exit)\n", highlight[gameStyle], regular[gameStyle], highlight[gameStyle], regular[gameStyle]);
    fflush(stdout);

    unsigned char input = getKeys();
    
    // Pressing enter unpauses the game
    if (input == 250) {
        gameState = GAME_STATE;
        printf("\e[1J"); // Clears the pause text when the game is unpaused
        return;
    } else if (input == 'R') {
        resetGame();
        gameState = GAME_STATE;
        return;
    } else if (input == 'Q') {
        running = 0;
        return;
    }
}

void gameOverLoop() {
    renderFrame(1);
    printPadded(10, "%sGame Over!%s\n", gameOver[gameStyle], regular[gameStyle]);
    printPadded(26, "%sENTER%s to check high scores.\n", highlight[gameStyle], regular[gameStyle]);
    fflush(stdout);

    unsigned char input = getKeys();
        
    // Enter press check
    if (input == 250) {
        // Placement check for high score
        for (int i = 0; i < 10; i++) {
            if (score > highScores[i].score) {
                placement = i;
                break;
            }
        }
        
        gameState = HIGHSCORE_STATE;
        return;
    }  
}

void resetGame() {
    // Making showGhost the gameStyle ID makes the Eletronika 60 style not have ghost pieces (it's ID 0)
    showGhost = gameStyle;

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

char offset = 0;                // Switches the "base color" of the letters
char changed[] = {0,0,0,0,0,0}; // Which line has had its color changed
char cont = 0;                  // Used to change each value in the array
void displayTitle() {
    char isBlock = 0;

    for (int i = 0; i < 6; i++) {
        //printf("%d", changed[i]);
        printPadded(44, "");
        for (int j = 0; j < 45; j++) {
            if (letter[i][j] == 7 && isBlock == 0) { 
                printf("%s", color[(offset + changed[i]) & 7]);
                isBlock = 1;
            } else if (letter[i][j] != 7 && isBlock != 0) { 
                printf("\e[38;5;227m");
                isBlock = 0;
            }
            printf("%s", block[letter[i][j]]);
        }
    }

    printf("\e[39m\n"); // Default color
    printPadded(20, "Press \e[38;5;227mENTER\e[39m to start.\n");
    printPadded(30, "Press \e[38;5;227m'H'\e[39m to view high scores.\n");
    printPadded(26, "Game style (\e[38;5;227mP\e[39m to swap): "); pieceColor(RED);

    printf("\n\n");
    printPadded(32, "\e[39mOriginal game by \e[38;5;227mAlexey Pajitnov\e[39m.\n");
    printPadded(18, "Remade by \e[38;5;227mMeiraGui\e[39m.");

    // titleTimer slows down the rate the colors move
    if ((titleTimer % 10) == 0) {
        changed[cont++] ^= 1;
    }

    if (cont > 5) {
        cont = 0;
        for (int i = 0; i < 6; i++) { 
            changed[i] = 0; // Resets the array
        }
        offset = (offset + 1) & 7; // Moves onto the next color
    }

    fflush(stdout);
}

void displayGrid() {
#if 0
    // shows whole grid including OOB
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j += 2) {
            if (Grid[i][j] < 8) {
            printf("%d%d", Grid[i][j],Grid[i][j+1]);
            } else {
            printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("\n");
    }
#else
    char length = gameStyle ? 22 : 24;

    printPadded(length, "   %sNext%s\n", highlight[gameStyle], regular[gameStyle]);
    printPadded(length, "%s", visualsTable[gameStyle][0]); // Next outline top
    for (int i = 3; i < 5; i++) {
        printPadded(length, "%s", visualsTable[gameStyle][1]); // Next outline middle
        for (int j = 4; j < 12; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("%s", regular[gameStyle]); // Default color
        printf("%s", visualsTable[gameStyle][1]); // Next outline middle
        if (i == 3) printf(" Score: %s%d%s", highlight[gameStyle], score, regular[gameStyle]);
        if (i == 4) printf(" Level: %s%d%s", highlight[gameStyle], level, regular[gameStyle]);
        printf("\n");
    }
    printPadded(length, "%s", visualsTable[gameStyle][2]); // Next outline bottom
    if (gameStyle) printPadded(length, "%s", visualsTable[gameStyle][3]); // Dividing line

    for (int i = 10; i < 30; i++) {
        // Non piece tiles can either be bright blue like the IBM PC version or a bright green
        printf("%s", playArea[gameStyle]);
        printPadded(length, "%s", visualsTable[gameStyle][4]); // Play area left
        for (int j = 6; j < 26; j += 2) {
            if (Grid[i][j] != ' ') { 
                pieceColor(Grid[i][j]);
            } else {
                printf("%c%c", Grid[i][j],Grid[i][j+1]);
            }
        }
        printf("%s", visualsTable[gameStyle][5]); // Play area right
    }
    printPadded(length, "%s", visualsTable[gameStyle][6]); // Play area bottom
    if (!gameStyle) printPadded(20, "%s", visualsTable[gameStyle][7]); // Play area bottom
    printf("%s", regular[gameStyle]); // Default color
#endif
}

void renderFrame(char flag) {
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

// Modified version of the Pseudo-random random float generator from "Numerical Recipes in C" (page 285),
// "Numerical Recipes in C" (page 285), to use integers instead of floats
char randomPieceID() {
    char result;
    
    do {
        idum = 1664525L*idum + 1013904223L;
        // The high bits are the most random, so we use those
        unsigned int  highBits = (unsigned int)idum >> 16;

        // AND 7 makes it a more even distribution, but because there are
        // only 7 pieces we roll again if the AND results in a 7
        result = (char)(highBits & 7);
    } while (result == 7);

    return result;
}

void pieceColor(char colorID) {
    char flag = 0;

    static const char *pieceColors[] = {
        "\e[38;5;129m", // Magenta
        "\e[38;5;27m",  // Blue
        "\e[38;5;160m", // Red
        "\e[38;5;184m", // Yellow
        "\e[38;5;34m",  // Green
        "\e[38;5;172m", // Orange
        "\e[38;5;80m",  // Cyan
        "\e[39m"        // Default color (padding for safety)
    };

    // Check to make the ghost piece show up as a transparent block
    if (colorID == GHO) {
        flag = 1;
        colorID = pieceID; // Color is copied for the current piece so the ghost piece is properly colored
    }

    if (gameStyle) {
        printf("%s", pieceColors[colorID & 7]);
        printf(flag ? "░░" : "██");
    } else {
        printf("\e[38;5;46m"); // Bright green
        printf("[]");
    }

    // Non piece tiles can either be bright blue like the IBM PC version or a bright green
    printf("%s", playArea[gameStyle]);
}

#define centerLine 1
#define centerColumn 2

// 0 = regular piece, 1 = ghost piece
void displayPiece(char flag) {    
    char dLine = flag ? ghLine : line;

    for (int i = -1; i < 3; i++) {
        for (int j = -4; j < 4; j += 2) {
            char pieceChar = flag ? GHO : pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)];
            if (pieceTable[pieceID][rot][centerLine+i][centerColumn+(j/2)] != ' ') {
                Grid[dLine+i][column+j] = pieceChar;
                Grid[dLine+i][column+(j+1)] = pieceChar;
            }
        }
    }
}

void displayNextPiece() {   
    for (int i = -1; i < 3; i++) {
        for (int j = -4; j < 4; j += 2) {
            Grid[3+i][8+j] = pieceTable[nextID][0][centerLine+i][centerColumn+(j/2)];
            Grid[3+i][8+(j+1)] = pieceTable[nextID][0][centerLine+i][centerColumn+(j/2)];
        }
    }
}

// 0 = regular piece, 1 = ghost piece
void clearPiece(char flag) {  
    char cLine = flag ? ghLine : line;
    for (int i = -1; i < 3; i++) {
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
    
    for (int i = -1; i < 3; i++) {
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

// flag is used to change the direction bias:
// 0 has a left bias, 1 has a right bias
char tryWallKick(char flag) {
    static const char yOffsets[] = {0,  0, 1, 1, 1};
    static const char xOffsets[2][5] = {{-2, 2, 0, -2, 2},
                                        {2, -2, 0, 2, -2}};
    
    //printf("%s", flag ? "right!": "left!");

    for (int i = 0; i < 5; i++) {
        line += yOffsets[i];
        column += xOffsets[flag][i];
        if (checkBoundaries(0) == 0) {
            return 0;
        }
        line -= yOffsets[i];
        column -= xOffsets[flag][i];
    }

    return 1; // If all wall kick attempts fail, return 1 to revert the rotation
}

// Stores the found lines so only the lines above them are moved downward
char foundLinesArr[4] = {};

void checkLines() {
    char linesFound = 0;
    
    // Only checks the 4 lines around the center, since those are the only ones that can have cleared lines
    for (int i = -1; i < 3; i++) {
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
    printf("\e[H");
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

void handleInput() {
    switch(getKeys()) {
        case 250: // Enter press
            gameState = PAUSE_STATE;
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
            rot--;
            rot &= 3;
            if (checkBoundaries(0) != 0) {
                rot += tryWallKick(0); // Left bias
            }
            break;
        case 'X':
            rot++;
            rot &= 3;
            if (checkBoundaries(0) != 0) {
                rot -= tryWallKick(1); // Right bias
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

/*
 * Save and High score functions
*/

void initSaveFile() {
    FILE *file = fopen("tetris.sav", "r+b");

    // If the file doesn't exist, create a new one
    if (file == NULL) {
        file = fopen("tetris.sav", "wb");

        if (file == NULL) {
            perror("Error creating save!");
            exit(1);
        }

        // 10 Empty high scores are created
        HighScore dummy = {.score = 0, .name = "            ", .level = 0};

        for (int i = 0; i < 10; i++) {
            if (fwrite(&dummy, sizeof(HighScore), 1, file) != 1) {
                perror("Write Fail!\n");
                exit(1);
            }
        }

        fclose(file);
        file = fopen("tetris.sav", "r+b");
        if (file == NULL) {
            perror("Error re-opening save!");
            exit(1);
        }
    }
        // Copies the 10 instances to an array
        if (fread(highScores, sizeof(HighScore), 10, file) != 10) {
            perror("Error loading the 10 highScores!\n");
            exit(1);
        }

    fclose(file);
}

void highScoreEntry() {
    char name[13] = {};
    char count = 0;

    #ifdef _WIN32
        // Code page has to be changed so characters with accents (such as "á") are only 1 byte
        SetConsoleCP(1252); SetConsoleOutputCP(1252);
    #endif

    printf("\e[?25h"); // Reactivates cursor
    while (1) {
        unsigned char key = getKeysName();

        if (key == 250 && count > 0) {
            break;
        } else if (key == 249) {
            if (count > 0) {    
                count--;
                printf("\b \b");
            }
            continue;
        } else if (count < 12) {
            if (key >= ' ' && key <= '~') {
                name[count++] = key;
                putchar(key);
                fflush(stdout);
            } else {
                fflush(stdout);
            }
        }

        usleep(FRAME_DELAY); // Delay on the loop to not overuse the CPU
    }

    name[count] = '\0';

    printf("\e[?25l"); // Disables cursor again

    HighScore newEntry = {.score = score, .name = ' ', .level = level};

    snprintf(newEntry.name, sizeof(newEntry.name), "%-*s", 12, name); // pads all remaining characters with ' '

    highScores[placement] = newEntry;

    FILE *file = fopen("tetris.sav", "r+b");
    if (file == NULL) {
        perror("Error re-opening save!");
        exit(1);
    }

    // After the array has been modified, write it to the save
    fwrite(highScores, sizeof(HighScore), 10, file);

    fclose(file);

    #ifdef _WIN32
        SetConsoleCP(65001); SetConsoleOutputCP(65001); // After the name is typed, the code page is reverted
    #endif

    placement = -1; // Placement is reset to -1 on exit
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
        perror("Error getting output handle!\n");
        exit(1);
    }

    // Gets the current mode of the console
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        perror("Error getting console mode!\n");
        exit(1);
    }

    // Enables the virtual terminal prcessing flag for ANSI support
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        perror("Error activating ANSI support!\n");
        exit(1);
    }

    // Code page is changed to support the "extended ASCII" characters
    if (!SetConsoleCP(65001) || !SetConsoleOutputCP(65001)) {
        perror("Error setting code page!");
        exit(1);
    }

    timeBeginPeriod(1); // Extra timing precision

    printf("\e[?25l"); // Disables flashing cursor for cleaner display
}

// Enables the cursor and returns the timing precision to normal
void restoreTerminal() {
    timeEndPeriod(1);
    printf("\e[?25h"); // Reactivates cursor on exit
    fflush(stdout);
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

// Used for normalizing inputs for the high score screen
unsigned char getKeysName() {
    unsigned char input;
    
    if (_kbhit()) {
        input = _getch();
        // Enter press check
        if (input == 13) {
            return 250;
        }

        // Backspace check
        if (input == '\b') {
            return 249;
        }

        return input;
    }

    return 0;
}

void getTerminalSize(int *width, int *height) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return;
    }

    // Default terminal sizes as a failsafe
    *width = 80;
    *height = 24;
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
}

// Changes the terminal mode to the regular procedure, reading inputs as buffers
void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\e[?25h"); // Reactivates cursor on exit
    fflush(stdout);
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

// Used for normalizing inputs for the high score screen
unsigned char getKeysName() {
    unsigned char input = getchar();

    // Enter press check
    if (input == '\n') {
        return 250;
    }

    // Backspace check
    if (input == 127) {
        return 249;
    }

    return input;
}

void getTerminalSize(int *width, int *height) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0 && w.ws_row > 0) {
        *width = w.ws_col;
        *height = w.ws_row;
        return;
    }

    // Default terminal sizes as a failsafe
    *width = 80;
    *height = 24;
}
#endif

// Prints text to the screen padded using the length parameter as a base,
// length should be the amount of characters used in the line that's being padded (ideally an even number)
void printPadded(char length, const char *format, ...) {
    int padding = currWidth > length ? (currWidth - length) / 2 : 0;
    
    printf("%*s", padding, "");

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}
