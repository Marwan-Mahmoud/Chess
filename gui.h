#pragma once

#include "chess.h"
#include <fcntl.h>
#include <stdio.h>
#include <windows.h>

#define WHITE_BACKGROUND BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY
#define BLACK_BACKGROUND FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

struct button {
    char label[5];
    COORD pos;
};

void initGUI(struct state *state);
void setup();
void initBoard();
void refresh(struct state *state);

void printBoard(char board[8][8]);
void printCurrentTurn(int counter, int turn);
void printTakenPieces(int numTaken, char taken[30]);
void printMessage(char *message);
void printPiece(char p, int t);

void drawButton(struct button b, int hover);
void drawPieceButton(char piece, COORD coord, int hover);
void drawActiveSquare(int x, int y);

void getCoor(char board[8][8], int turn, COORD *boardPosition);
char getPromotionPiece(int turn);
