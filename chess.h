#pragma once

#define WHITE 0
#define BLACK 1

#define WHITE_KING 0b1
#define LEFT_WHITE_ROOK 0b10
#define RIGHT_WHITE_ROOK 0b100
#define BLACK_KING 0b1000
#define LEFT_BLACK_ROOK 0b10000
#define RIGHT_BLACK_ROOK 0b100000

struct state {
    char board[8][8];
    int counter;
    int turn;
    int check;

    // Taken pieces
    int numTaken;
    char taken[30];

    // Castling rights
    int movedPieces;

    // Draw conditions
    int passiveMoves;
};

enum move_type {
    INVALID,
    NORMAL,
    CASTLE_SHORT,
    CASTLE_LONG
};

void initGame();
void startGameLoop();

int move(int x1, int y1, int x2, int y2, char pieceSwap);
int castle(enum move_type type);
enum move_type valid(int x1, int y1, int x2, int y2);
enum move_type checkPawnMove(int x1, int y1, int x2, int y2);
enum move_type checkKnightMove(int x1, int y1, int x2, int y2);
enum move_type checkBishopMove(int x1, int y1, int x2, int y2);
enum move_type checkRookMove(int x1, int y1, int x2, int y2);
enum move_type checkQueenMove(int x1, int y1, int x2, int y2);
enum move_type checkKingMove(int x1, int y1, int x2, int y2);
int checkSameColor(char p1, char p2);
int isPromotion(int x1, int y1, int x2, int y2);

int checkGameStatus();
int noLegalMoves();
int draw();
int isCheck();

int isWhitePiece(char piece);
int isBlackPiece(char piece);

void undo();
void save();
void load();
