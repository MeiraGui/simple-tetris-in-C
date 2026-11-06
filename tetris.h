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
 * Visual arrays
 */
// Elektronika 60 style visuals are based on the original version of Tetris
const char *eletronika[] = {"\n",   // Next outline top
                            " ",    // Next outline middle
                            "\n",   // Next outline bottom
                            "",     // Dividing line
                            "<!",   // Play area left
                            "!>\n", // Play area right
                            "<!====================!>\n  \\/\\/\\/\\/\\/\\/\\/\\/\\/\\/\n", // Play area bottom
                        };

     const char *clean[] = {"┌────────┐\n",             // Next outline top
                            "│",                        // Next outline middle
                            "└────────┘\n",             // Next outline bottom
                            "──────────────────────\n", // Dividing line
                            "║",                        // Play area left
                            "║\n",                      // Play area right
                            "╚════════════════════╝\n", // Play area bottom
                        };

const char **visualsTable[] = {eletronika, clean};

// Arrays that switch colors, first are all bright green, second ones are colorful
const char   *regular[] = {"\e[38;5;46m", "\e[39m"};
const char *highlight[] = {"\e[38;5;46m", "\e[38;5;227m",};
const char  *playArea[] = {"\e[38;5;46m", "\e[38;5;68m"};
const char  *gameOver[] = {"\e[38;5;46m", "\e[38;5;160m"};

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
void displayPiece(char flag);
void displayNextPiece();
void clearPiece(char flag);
char checkBoundaries(char flag);
void checkLines();
void clearLine(int line);
void moveLinesDown(int linesFound);

// Terminal functions
void initTerminal();
void restoreTerminal();
unsigned char getKeys();

#endif