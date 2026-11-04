#ifndef TETRIS_H
#define TETRIS_H

/*
 * Piece arrays 
 */
const char T_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'M', 'M', 'M', 
                                  ' ', ' ', 'M', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'M', ' ', 
                                  ' ', ' ', 'M', 'M', 
                                  ' ', ' ', 'M', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'M', ' ', 
                                  ' ', 'M', 'M', 'M', 
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'M', ' ', 
                                  ' ', 'M', 'M', ' ', 
                                  ' ', ' ', 'M', ' ',},
                          };

const char J_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'B', 'B', 'B', 
                                  ' ', ' ', ' ', 'B',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'B', 'B', 
                                  ' ', ' ', 'B', ' ', 
                                  ' ', ' ', 'B', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', 'B', ' ', ' ', 
                                  ' ', 'B', 'B', 'B', 
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'B', ' ', 
                                  ' ', ' ', 'B', ' ', 
                                  ' ', 'B', 'B', ' ',},
                          };

const char Z_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'R', 'R', ' ', 
                                  ' ', ' ', 'R', 'R',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', 'R', 
                                  ' ', ' ', 'R', 'R', 
                                  ' ', ' ', 'R', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'R', 'R', ' ', 
                                  ' ', ' ', 'R', 'R',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', 'R', 
                                  ' ', ' ', 'R', 'R', 
                                  ' ', ' ', 'R', ' ',},
                          };

const char O_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'Y', 'Y', ' ',
                                  ' ', 'Y', 'Y', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'Y', 'Y', ' ',
                                  ' ', 'Y', 'Y', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'Y', 'Y', ' ',
                                  ' ', 'Y', 'Y', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'Y', 'Y', ' ',
                                  ' ', 'Y', 'Y', ' ',},
                          };

const char S_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', ' ', 'G', 'G', 
                                  ' ', 'G', 'G', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'G', ' ', 
                                  ' ', ' ', 'G', 'G', 
                                  ' ', ' ', ' ', 'G',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', ' ', 'G', 'G', 
                                  ' ', 'G', 'G', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'G', ' ', 
                                  ' ', ' ', 'G', 'G', 
                                  ' ', ' ', ' ', 'G',},
                          };
                          
const char L_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  ' ', 'O', 'O', 'O', 
                                  ' ', 'O', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', 'O', ' ', 
                                  ' ', ' ', 'O', ' ', 
                                  ' ', ' ', 'O', 'O',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', 'O', 
                                  ' ', 'O', 'O', 'O', 
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', 'O', 'O', ' ', 
                                  ' ', ' ', 'O', ' ', 
                                  ' ', ' ', 'O', ' ',},
                          };

const char I_piece[4][4][4] = {  {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  'C', 'C', 'C', 'C', 
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', 'C', ' ',
                                  ' ', ' ', 'C', ' ', 
                                  ' ', ' ', 'C', ' ', 
                                  ' ', ' ', 'C', ' ',},

                                 {' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', 
                                  'C', 'C', 'C', 'C', 
                                  ' ', ' ', ' ', ' ',},

                                 {' ', ' ', 'C', ' ',
                                  ' ', ' ', 'C', ' ', 
                                  ' ', ' ', 'C', ' ', 
                                  ' ', ' ', 'C', ' ',},
                          };                          

const char (*pieceTable[])[4][4] = {T_piece, J_piece, Z_piece, O_piece, S_piece, L_piece, I_piece};

/*
 * Function declarations 
 */
// Tetris functions
void displayTitle();
void gameLogic();
void renderFrame(char flag);
void initializeGrid();
void displayGrid();
void pieceColor(char color);
void handleInput();
void displayPiece();
void displayNextPiece();
void clearPiece();
char checkBoundaries();
void checkLines();
void clearLine(int line);
void moveLinesDown(int linesFound);

// Terminal functions
void initTerminal();
void restoreTerminal();
unsigned char getKeys();

/*
 * Visual flags
 */
// Elektronika 60 style visuals are based on the original version of Tetris
#define AREA_STYLE  1  // 0 for Elektronika 60 style, 1 for cleaner
#define BLOCK_STYLE 1  // 0 for Elektronika 60 style bracket blocks, 1 for solid blocks

#if AREA_STYLE
    #define NEXT_TOP    "┌────────┐\n"
    #define NEXT_MID    "│"
    #define NEXT_BOTTOM "└────────┘\n"
    #define DIV_LINE    "──────────────────────\n"
    #define MAIN_LEFT   "║"
    #define MAIN_RIGHT  "║\n"
    #define MAIN_BOTTOM "╚════════════════════╝\n"
#else
    #define NEXT_TOP    "!========!\n"
    #define NEXT_MID    "!"
    #define NEXT_BOTTOM "!========!\n"
    #define DIV_LINE    "------------------------\n"
    #define MAIN_LEFT   "<!"
    #define MAIN_RIGHT  "!>\n"
    #define MAIN_BOTTOM "<!====================!>\n  \\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\n"
#endif


#endif