#ifndef TETRIS_H
#define TETRIS_H

/*
 * Enums
 */

// Each color has a number ID to acess the pieceColor lookup table
enum {
    MAG, // Magenta
    BLU, // Blue
    RED, // Red
    YLW, // Yellow
    GRN, // Green
    ORG, // Orange
    CYN, // Cyan
    GHO  // Ghost piece ID
};

// Game states to switch between
enum {
    TITLE_STATE,
    HIGHSCORE_STATE,
    GAME_STATE,
    PAUSE_STATE,
    GAMEOVER_STATE
};

/*
 * Piece arrays 
 */

const char T_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', MAG, MAG, MAG,
                                  ' ', ' ', MAG, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', MAG, ' ',
                                  ' ', MAG, MAG, ' ',
                                  ' ', ' ', MAG, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', MAG, ' ',
                                  ' ', MAG, MAG, MAG,
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', MAG, ' ',
                                  ' ', ' ', MAG, MAG,
                                  ' ', ' ', MAG, ' ',
                                  ' ', ' ', ' ', ' ',},
                          };

const char J_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', BLU, BLU, BLU,
                                  ' ', ' ', ' ', BLU,
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', BLU, ' ',
                                  ' ', ' ', BLU, ' ',
                                  ' ', BLU, BLU, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', BLU, ' ', ' ',
                                  ' ', BLU, BLU, BLU,
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', BLU, BLU,
                                  ' ', ' ', BLU, ' ',
                                  ' ', ' ', BLU, ' ',
                                  ' ', ' ', ' ', ' ',},
                          };

const char Z_piece[4][4][4] = {  {' ', ' ', ' ', ' ', 
                                  ' ', RED, RED, ' ',
                                  ' ', ' ', RED, RED,
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', RED, ' ',
                                  ' ', RED, RED, ' ',
                                  ' ', RED, ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', RED, RED, ' ',
                                  ' ', ' ', RED, RED,
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', RED,
                                  ' ', ' ', RED, RED,
                                  ' ', ' ', RED, ' ',
                                  ' ', ' ', ' ', ' ',},
                          };

const char O_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', YLW, YLW, ' ',
                                  ' ', ' ', ' ', ' ',},
                          };

const char S_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', GRN, GRN,
                                  ' ', GRN, GRN, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', GRN, ' ', ' ',
                                  ' ', GRN, GRN, ' ',
                                  ' ', ' ', GRN, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', GRN, GRN,
                                  ' ', GRN, GRN, ' ',
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', GRN, ' ',
                                  ' ', ' ', GRN, GRN,
                                  ' ', ' ', ' ', GRN, 
                                  ' ', ' ', ' ', ' ',},
                          };
                          
const char L_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ORG, ORG, ORG,
                                  ' ', ORG, ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ORG, ORG, ' ',
                                  ' ', ' ', ORG, ' ',
                                  ' ', ' ', ORG, ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ORG,
                                  ' ', ORG, ORG, ORG,
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ORG, ' ',
                                  ' ', ' ', ORG, ' ',
                                  ' ', ' ', ORG, ORG,
                                  ' ', ' ', ' ', ' ',},
                          };

const char I_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  CYN, CYN, CYN, CYN,
                                  ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', CYN, ' ',
                                  ' ', ' ', CYN, ' ',
                                  ' ', ' ', CYN, ' ',
                                  ' ', ' ', CYN, ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ',
                                  CYN, CYN, CYN, CYN,
                                  ' ', ' ', ' ', ' ',},

                                 {' ', CYN, ' ', ' ',
                                  ' ', CYN, ' ', ' ',
                                  ' ', CYN, ' ', ' ',
                                  ' ', CYN, ' ', ' ',},
                          };                          

const char (*const pieceTable[])[4][4] = {T_piece, J_piece, Z_piece, O_piece, S_piece, L_piece, I_piece};

/*
 * Visual arrays
 */

// Elektronika 60 style visuals are based on the original version of Tetris
const char *eletronika[] = {"          ",   // Next outline top
                            " ",    // Next outline middle
                            "          ",   // Next outline bottom
                            "<!",   // Play area left
                            "!>", // Play area right
                            "<!====================!>\n",      // Play area bottom
                            "\\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\n" // Play area bottom 2
                        };

     const char *clean[] = {"┌────────┐",               // Next outline top
                            "│",                        // Next outline middle
                            "└────────┘",               // Next outline bottom
                            "║",                        // Play area left
                            "║",                        // Play area right
                            "╚════════════════════╝\n", // Play area bottom
                            "╔════════════════════╗\n", // Play area top
                        };

const char **visualsTable[] = {eletronika, clean};

// Arrays that switch colors, first are all bright green, second ones are colorful
const char   *regular[] = {"\e[38;5;46m", "\e[39m"};
const char *highlight[] = {"\e[38;5;46m", "\e[38;5;227m",};
const char  *playArea[] = {"\e[38;5;46m", "\e[38;5;68m"};
const char  *gameOver[] = {"\e[38;5;46m", "\e[38;5;160m"};

const char *color[] = {
        "\e[38;5;27m",  // Blue
        "\e[38;5;34m",  // Green
        "\e[38;5;80m",  // Cyan
        "\e[38;5;160m", // Red
        "\e[38;5;129m", // Magenta
        "\e[38;5;172m", // Orange
        "\e[38;5;184m", // Yellow
        "\e[39m"        // Default color
    };

const char *block[] = {
        " ",
        "║",
        "╗",
        "╝",
        "╚",
        "╔",
        "═",
        "█",
        "\n"
    };

// The numbers here are indexes to the characters defined in the "block" array above
char letter[6][45] = {7,7,7,7,7,7,2,0,7,7,7,7,7,7,2,0,7,7,7,7,7,7,2,0,7,7,7,7,7,7,2,0,0,7,7,2,0,7,7,7,7,7,7,2,8,
                      0,0,7,7,5,6,3,0,7,7,5,6,6,6,3,0,0,0,7,7,5,6,3,0,7,7,5,6,6,7,7,2,0,7,7,1,0,7,7,5,6,6,6,3,8,
                      0,0,7,7,1,0,0,0,7,7,7,7,7,1,0,0,0,0,7,7,1,0,0,0,7,7,7,7,7,7,5,3,0,7,7,1,0,7,7,7,7,7,7,2,8,
                      0,0,7,7,1,0,0,0,7,7,5,6,6,3,0,0,0,0,7,7,1,0,0,0,7,7,5,6,6,7,7,2,0,7,7,1,0,4,6,6,6,7,7,1,8,
                      0,0,7,7,1,0,0,0,7,7,7,7,7,7,1,0,0,0,7,7,1,0,0,0,7,7,1,0,0,7,7,1,0,7,7,1,0,7,7,7,7,7,7,1,8,
                      0,0,4,6,3,0,0,0,4,6,6,6,6,6,3,0,0,0,4,6,3,0,0,0,4,6,3,0,0,4,6,3,0,4,6,3,0,4,6,6,6,6,6,3,8,};

/*
 * Structs
*/

typedef struct {
    unsigned int score;
    char name[13]; // 12 characters + null terminator
    char level;
} HighScore;

/*
 * Function declarations 
 */

/* Tetris functions */

// Main loops
void titleLoop();
void highScoreLoop();
void gameLoop();
void pauseLoop();
void gameOverLoop();
// Reset and initialization
void resetGame();
void initializeGrid();
// Save and High scores
void initSaveFile();
void highScoreEntry();
// Main display functions
void displayTitle();
void displayGrid();
void displayLeftColumns(char line);
void displayRightColumns(char line);
void renderFrame(char flag);
// Piece display and movement
char randomPieceID();
void pieceColor(char color);
void displayPiece(char flag);
void displayNextPiece();
void clearPiece(char flag);
char checkBoundaries(char flag);
char tryWallKick(char flag);
// Line clear functions
void checkLines();
void clearLine(int line);
void moveLinesDown(int linesFound);
// Input
void handleInput();

/* Save and High Score functions */
void initSaveFile();
void highScoreEntry();

/* Terminal functions */

#ifndef _WIN32
void handleSignal(); // Linux only
#endif
void initTerminal();
void restoreTerminal();
unsigned char getKeys();
unsigned char getKeysName();
int getTerminalWidth();
void getTerminalSize(int *width, int *height);
void printPadded(char length, const char *format, ...);

#endif
