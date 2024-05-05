#pragma once

#define WHITE 0
#define BLACK 1

struct state
{
    char board[8][8];
    int counter;
    int turn;
    int check;
    
    // Taken pieces
    int numTaken;
    char taken[30];
    
    // Left black rook, black king, right black rook, left white rook, white king, right white rook
    int castling[6];
};

enum move_type
{
    INVALID,
    NORMAL,
    CASTLE_SHORT,
    CASTLE_LONG
};

void initGame();

void move(int x1, int y1, int x2, int y2, char pieceSwap);
void castle(enum move_type type);
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

int getInput();

int isWhitePiece(char piece);
int isBlackPiece(char piece);

void undo();
void save();
void load();
