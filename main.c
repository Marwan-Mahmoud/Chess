#include <stdlib.h>
#include <io.h>
#include "chess.h"
#include "gui.h"
#include "arraylist.h"

struct state state;
ArrayList *undoList;

char whitePieces[6] = {'P','R','N','B','Q','K'};
char blackPieces[6] = {'p','r','n','b','q','k'};


void initGame() {
    char initialBoard[8][8] = {
        {'r','n','b','q','k','b','n','r'},
        {'p','p','p','p','p','p','p','p'},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {'P','P','P','P','P','P','P','P'},
        {'R','N','B','Q','K','B','N','R'}};

    memcpy(state.board, initialBoard, sizeof(initialBoard));
    state.counter = 0;
    state.turn = WHITE;
    state.check = 0;
    state.numTaken = 0;
    for (int i = 0; i < 6; i++)
        state.castling[i] = 0;

    undoList = createArrayList();
    add(undoList, state);
}

void move(int x1, int y1, int x2, int y2, char pieceSwap) {
    char temp = state.board[y2][x2];
    if (state.board[y2][x2] != ' ')
        state.taken[state.numTaken++] = state.board[y2][x2];
    state.board[y2][x2] = state.board[y1][x1];
    state.board[y1][x1] = ' ';
    state.turn = !state.turn;
    state.check = isCheck();
    state.turn = !state.turn;
    if (state.check) {
        state.board[y1][x1] = state.board[y2][x2];
        state.board[y2][x2] = temp;
        if (state.board[y2][x2] != ' ')
            state.numTaken--;
        return;
    }

    if(x1 == 0 && y1 ==0)
        state.castling[0] = 1;
    else if(x1 == 4 && y1 ==0)
        state.castling[1] = 1;
    else if(x1 == 7 && y1 ==0)
        state.castling[2] = 1;
    else if(x1 == 0 && y1 ==7)
        state.castling[3] = 1;
    else if(x1 == 4 && y1 ==7)
        state.castling[4] = 1;
    else if(x1 == 7 && y1 ==7)
        state.castling[5] = 1;

    if (pieceSwap)
        state.board[y2][x2] = pieceSwap;

    state.check = isCheck();
    state.turn = !state.turn;
    state.counter++;
    refresh(&state);
    add(undoList, state);
}

void castle(enum move_type type){
    int y = state.turn == WHITE ? 7 : 0;
    state.turn = !state.turn;
    switch (type) {
        case CASTLE_SHORT:
            for (int y1 = 0; y1 < 8; y1++) {
                for (int x1 = 0; x1 < 8; x1++) {
                    if (valid(x1, y1, 6, y) || valid(x1, y1, 5, y)) {
                        state.turn = !state.turn;
                        return;
                }
            }
            }
            state.turn = !state.turn;
            state.board[y][4] = ' ';
            state.board[y][5] = state.turn == WHITE ? 'R' : 'r';
            state.board[y][6] = state.turn == WHITE ? 'K' : 'k';
            state.board[y][7] = ' ';
            break;
        case CASTLE_LONG:
            for (int y1 = 0; y1 < 8; y1++) {
                for (int x1 = 0; x1 < 8; x1++) {
                    if (valid(x1, y1, 2, y) || valid(x1, y1, 3, y)) {
                        state.turn = !state.turn;
                        return;
                }
            }
            }
            state.turn = !state.turn;
            state.board[y][4] = ' ';
            state.board[y][3] = state.turn == WHITE ? 'R' : 'r';
            state.board[y][2] = state.turn == WHITE ? 'K' : 'k';
            state.board[y][0] = ' ';
            break;
    }
    state.check = isCheck();
    state.turn = !state.turn;
    state.counter++;
    refresh(&state);
    add(undoList, state);
}

enum move_type valid(int x1, int y1, int x2, int y2) {
    if (x1 == -1 || y1 == -1 || x2 == -1 || y2 == -1)
        return INVALID;

    if (checkSameColor(state.board[y1][x1], state.board[y2][x2]))
        return INVALID;
    
    switch (state.turn) {
        case WHITE:
            switch (state.board[y1][x1]) {
                case 'P':
                    return checkPawnMove(x1, y1, x2, y2);
                case 'R':
                    return checkRookMove(x1, y1, x2, y2);
                case 'N':
                    return checkKnightMove(x1, y1, x2, y2);
                case 'B':
                    return checkBishopMove(x1, y1, x2, y2);
                case 'Q':
                    return checkQueenMove(x1, y1, x2, y2);
                case 'K':
                    return checkKingMove(x1, y1, x2, y2);
            }
            break;
        case BLACK:
            switch (state.board[y1][x1]) {
                case 'p':
                    return checkPawnMove(x1, y1, x2, y2);
                case 'r':
                    return checkRookMove(x1, y1, x2, y2);
                case 'n':
                    return checkKnightMove(x1, y1, x2, y2);
                case 'b':
                    return checkBishopMove(x1, y1, x2, y2);
                case 'q':
                    return checkQueenMove(x1, y1, x2, y2);
                case 'k':
                    return checkKingMove(x1, y1, x2, y2);
            }
            break;
    }
    return INVALID;
}

enum move_type checkPawnMove(int x1, int y1, int x2, int y2){
    switch(state.turn){
        case 0:
            if((y2 == y1 - 1) && (x1 == x2) && (state.board[y2][x2] == ' '))
                return 1;
            else if((y2 == y1 - 2) && (y1 == 6) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 - 1; i >= y2; i--){
                    if(!(state.board[i][x1] == ' ')){
                        valid = 0;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            else if((y2 == y1 - 1) && (x2 == x1 + 1)){
                int valid = 0;
                for(int i = 0; i < 6; i++){
                    if(state.board[y2][x2] == blackPieces[i]){
                        valid = 1;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            else if((y2 == y1 - 1) && (x2 == x1 - 1)){
                int valid = 0;
                for(int i = 0; i < 6; i++){
                    if(state.board[y2][x2] == blackPieces[i]){
                        valid = 1;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            break;
        case 1:
            if((y2 == y1 + 1) && (x1 == x2) && (state.board[y2][x2] == ' '))
                return 1;
            else if((y2 == y1 + 2) && (y1 == 1) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 + 1; i <= y2; i++){
                    if(!(state.board[i][x1] == ' ')){
                        valid = 0;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            else if((y2 == y1 + 1) && (x2 == x1 + 1)){
                int valid = 0;
                for(int i = 0; i < 6; i++){
                    if(state.board[y2][x2] == whitePieces[i]){
                        valid = 1;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            else if((y2 == y1 + 1) && (x2 == x1 - 1)){
                int valid = 0;
                for(int i = 0; i < 6; i++){
                    if(state.board[y2][x2] == whitePieces[i]){
                        valid = 1;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            break;
    }
    return 0;
}

enum move_type checkKnightMove(int x1, int y1, int x2, int y2){
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1))
        return NORMAL;

    return INVALID;
}

enum move_type checkBishopMove(int x1, int y1, int x2, int y2){
    if (abs(x2 - x1) == abs(y2 - y1)) {
        int dx = (x2 > x1) ? 1 : -1;
        int dy = (y2 > y1) ? 1 : -1;

        for (int i = 1; i < abs(x2 - x1); i++) {
            if (state.board[y1 + i * dy][x1 + i * dx] != ' ')
                return INVALID;
        }
        return NORMAL;
    }
    return INVALID;
}

enum move_type checkRookMove(int x1, int y1, int x2, int y2){
    int dx = 0, dy = 0;
    if(x1 == x2) dy = (y2 > y1) ? 1 : -1;
    else if(y1 == y2) dx = (x2 > x1) ? 1 : -1;
    else return 0;

    int x = x1 + dx, y = y1 + dy;
    while(x != x2 || y != y2) {
        if(state.board[y][x] != ' ')
            return INVALID;
        
        x += dx;
        y += dy;
    }
    return NORMAL;
}

enum move_type checkQueenMove(int x1, int y1, int x2, int y2){
    return checkBishopMove(x1, y1, x2, y2) || checkRookMove(x1, y1, x2, y2);
}

enum move_type checkKingMove(int x1, int y1, int x2, int y2){
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1) || (dx == 1 && dy == 1))
        return NORMAL;

    if (state.check == 0) {
        switch(state.turn){
            case WHITE:
                if(x1 == 4 && y1 == 7 && x2 == 6 && y2 == 7 && state.castling[4] == 0 && state.castling[5] == 0 && checkRookMove(4, 7, 7, 7))
                    return CASTLE_SHORT;
                else if(x1 == 4 && y1 == 7 && x2 == 2 && y2 == 7 && state.castling[4] == 0 && state.castling[3] == 0 && checkRookMove(4, 7, 0, 7))
                    return CASTLE_LONG;
                break;
            case BLACK:
                if(x1 == 4 && y1 == 0 && x2 == 6 && y2 == 0 && state.castling[1] == 0 && state.castling[2] == 0 && checkRookMove(4, 0, 7, 0))
                    return CASTLE_SHORT;
                else if(x1 == 4 && y1 == 0 && x2 == 2 && y2 == 0 && state.castling[1] == 0 && state.castling[0] == 0 && checkRookMove(4, 0, 0, 0))
                    return CASTLE_LONG;
                break;
        }
    }
    return INVALID;
}

// To prevent killing pieces of the same color
int checkSameColor(char p1, char p2){
    switch (state.turn)
    {
    case WHITE:
        return isWhitePiece(p1) && isWhitePiece(p2);
    case BLACK:
        return isBlackPiece(p1) && isBlackPiece(p2);
    }
    return 0;
}

int isPromotion(int x1, int y1, int x2, int y2){
    switch(state.turn){
        case WHITE:
            return y2 == 0 && state.board[y1][x1] == 'P';
        case BLACK:
            return y2 == 7 && state.board[y1][x1] == 'p';
    }
    return 0;
}

int checkGameStatus()
{
    if (noLegalMoves())
    {
        if (state.check)
            state.turn == WHITE ? printMessage("Black wins") : printMessage("White wins");
        else
            printMessage("Stalemate");
        return 1;
    }

    if (draw())
        return 1;

    return 0;
}

int noLegalMoves() {
    char originalPiece, movedPiece;
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            for(int k = 0; k < 8; k++){
                for(int l = 0; l < 8; l++){
                    if(valid(i, j, k, l)){
                        originalPiece = state.board[j][i];
                        movedPiece = state.board[l][k];
                        
                        // Make the move
                        state.board[j][i] = ' ';
                        state.board[l][k] = originalPiece;
                        state.turn = !state.turn;
                        
                        // Check if the current player is not in check
                        if(!isCheck()){
                            // Undo the move
                            state.board[j][i] = originalPiece;
                            state.board[l][k] = movedPiece;
                            state.turn = !state.turn;
                            return 0;
                        }
                        
                        // Undo the move
                        state.board[j][i] = originalPiece;
                        state.board[l][k] = movedPiece;
                        state.turn = !state.turn;
                    }
                }
            }
        }
    }
    return 1;
}

int draw() {
    int i, j, k, m;
    if(state.numTaken == 29){
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(state.board [j][i] == 'N' || state.board [j][i] == 'n' || state.board [j][i] == 'B' || state.board [j][i] == 'b'){
                    printMessage("Draw");
                    return 1;
                }
            }
        }
    }
    else if(state.numTaken == 28){
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(state.board [j][i] == 'N' || state.board [j][i] == 'B'){
                    for(k = 0; k < 8; k++){
                        for(m = 0; m < 8; m++){
                            if(state.board [m][k] == 'n' || state.board [m][k] == 'b'){
                                printMessage("Draw");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(state.numTaken == 30){
        printMessage("Draw");
        return 1;
    }
    else if(state.counter == 80){
        printMessage("Draw");
        return 1;
    }
    return 0;
}

int isCheck(){
    // Find the king
    char king = state.turn == WHITE ? 'k' : 'K';
    int kingPos[2];
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8 ; x++){
            if(state.board[y][x] == king){
                kingPos[0] = x;
                kingPos[1] = y;
                break;
            }
        }
    }

    // Check if the king is in check
    for (int y1 = 0; y1 < 8; y1++){
        for (int x1 = 0; x1 < 8; x1++){
            if (valid(x1, y1, kingPos[0], kingPos[1]))
                return 1;
        }
    }
    return 0;
}

int getInput(){
    static COORD c1 = {-1, -1};
    static COORD c2 = {-1, -1};
    getCoor(state.board, state.turn, &c2);
    enum move_type type = valid(c1.X, c1.Y, c2.X, c2.Y);
    if (type == NORMAL) {
        char p = '\0';
        if (isPromotion(c1.X, c1.Y, c2.X, c2.Y))
            p = getPromotionPiece(state.turn);

        move(c1.X, c1.Y, c2.X, c2.Y, p);
        c1.X, c1.Y, c2.X, c2.Y = -1;
        return checkGameStatus();
    }
    else if (type == CASTLE_SHORT || type == CASTLE_LONG) {
        castle(type);
        c1.X, c1.Y, c2.X, c2.Y = -1;
        return checkGameStatus();
    }
    else { 
        c1 = c2;
    }
    return 0;
}

int isWhitePiece(char piece) {
    for (int i = 0; i < 6; i++) {
        if (piece == whitePieces[i])
            return 1;
    }
    return 0;
}

int isBlackPiece(char piece) {
    for (int i = 0; i < 6; i++) {
        if (piece == blackPieces[i])
            return 1;
    }
    return 0;
}

void undo(){
    state = pop(undoList);
    refresh(&state);
}

void save() {
    FILE *fp;
    fp = fopen("SaveData.txt", "wb");
    if(fp == NULL)
        exit(1);

    fwrite(&state, sizeof(state), 1, fp);
    fclose(fp);
}

void load() {
    FILE *fp;
    fp = fopen("SaveData.txt", "rb");
    if(fp == NULL)
        exit(1);

    fread(&state, sizeof(state), 1, fp);
    fclose(fp);

    refresh(&state);
    destroyArrayList(undoList);
    undoList = createArrayList();
    add(undoList, state);
}


int main() {
    initGame();
    initGUI(&state);

    while(1){
        if(getInput())
            break;
    }
    destroyArrayList(undoList);

    // To display the last message before termination(e.g. Stalemate, White wins.. etc)
    getchar();
    return 0;
}
