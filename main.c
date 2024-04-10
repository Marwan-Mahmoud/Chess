#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>

// Initial Board
char board [8][8] = {
    {'r','n','b','q','k','b','n','r'},
    {'p','p','p','p','p','p','p','p'},
    {'.','-','.','-','.','-','.','-'},
    {'-','.','-','.','-','.','-','.'},
    {'.','-','.','-','.','-','.','-'},
    {'-','.','-','.','-','.','-','.'},
    {'P','P','P','P','P','P','P','P'},
    {'R','N','B','Q','K','B','N','R'}};

// 0:White  1:Black
int turn = 0;
int check = 0;

char x[8] = {'A','B','C','D','E','F','G','H'};
char y[8] = {'1','2','3','4','5','6','7','8'};
char whitePieces[6] = {'P','R','N','B','Q','K'};
char blackPieces[6] = {'p','r','n','b','q','k'};

// Taken pieces
int numTaken = 0;
char taken[30];

char undoBoard[80][8][8];
int undoTaken[80];
int undoCheck[80];
int undoCastling[80][6];
int counter = 0;

// Initial Kings Position
int whiteKingPos[2] = {4, 7};
int blackKingPos[2] = {4, 0};

// Left black rook, black king, right black rook, left white rook, white king, right white rook
int castling[6] = {0};

struct saveData{
    char board[8][8];
    int turn;
    int check;
    int numTaken;
    char taken[30];
    int castling[6];
}data;

void save(){
    FILE *fp;
    fp = fopen("SaveData.txt", "wb");
    if(fp == NULL)
        exit(1);
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            data.board[y][x] = board[y][x];
    }
    data.turn = turn;
    data.check = check;
    data.numTaken = numTaken;
    for(int i = 0; i < numTaken; i++)
        data.taken[i] = taken[i];
    for(int i = 0; i < 6; i++)
        data.castling[i] = castling[i];
    fwrite(&data, sizeof(data), 1, fp);
    fclose(fp);
}

void load(){
    FILE *fp;
    fp = fopen("SaveData.txt", "rb");
    if(fp == NULL)
        exit(1);
    fread(&data, sizeof(data), 1, fp);
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            board[y][x] = data.board[y][x];
    }
    turn = data.turn;
    check = data.check;
    numTaken = data.numTaken;
    for(int i = 0; i < numTaken; i++)
        taken[i] = data.taken[i];
    for(int i = 0; i < 6; i++)
        castling[i] = data.castling[i];
    fclose(fp);
}

int valid(int x1, int y1, int x2, int y2);

void castle(int c){
    char temp[8][8];
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            temp[y][x] = board[y][x];
    }
    switch(c){
        case 0:
            board[7][4] = '-';
            board[7][7] = '.';
            board[7][6] = 'K';
            board[7][5] = 'R';
            turn = (turn == 0)?1:0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 6, 7) || valid(x1, y1, 5, 7)){
                        for(int y = 0; y < 8; y++){
                            for(int x = 0; x < 8; x++)
                                board[y][x] = temp[y][x];
                        }
                        return ;
                    }
                }
            }
            turn = (turn == 0)?1:0;
            break;
        case 1:
            board[7][4] = '-';
            board[7][0] = '-';
            board[7][2] = 'K';
            board[7][3] = 'R';
            turn = (turn == 0)?1:0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 2, 7) || valid(x1, y1, 3, 7)){
                        for(int y = 0; y < 8; y++){
                            for(int x = 0; x < 8; x++)
                                board[y][x] = temp[y][x];
                        }
                        return ;
                    }
                }
            }
            turn = (turn == 0)?1:0;
            break;
        case 2:
            board[0][4] = '.';
            board[0][7] = '-';
            board[0][6] = 'k';
            board[0][5] = 'r';
            turn = (turn == 0)?1:0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 6, 0) || valid(x1, y1, 5, 0)){
                        for(int y = 0; y < 8; y++){
                            for(int x = 0; x < 8; x++)
                                board[y][x] = temp[y][x];
                        }
                        return ;
                    }
                }
            }
            turn = (turn == 0)?1:0;
            break;
        case 3:
            board[0][4] = '.';
            board[0][0] = '.';
            board[0][2] = 'k';
            board[0][3] = 'r';
            turn = (turn == 0)?1:0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 2, 0) || valid(x1, y1, 3, 0)){
                        for(int y = 0; y < 8; y++){
                            for(int x = 0; x < 8; x++)
                                board[y][x] = temp[y][x];
                        }
                        return ;
                    }
                }
            }
            turn = (turn == 0)?1:0;
            break;
    }
    turn = (turn == 0)?1:0;
    counter++;
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            undoBoard[counter][y][x] = board[y][x];
    }
    undoTaken[counter] = numTaken;
    undoCheck[counter] = check;
    for(int i = 0; i < 6; i++)
        undoCastling[counter][i] = castling[i];
}

// Get coordinate
void getCoor(char s[], int coor[]){
    int x1, y1, x2, y2;
    coor[4] = 0;
    for(x1 = 0; x1 < 8; x1++){
        if(s[0] == x[x1])
            break;
    }
    for(y1 = 0; y1 < 8; y1++){
        if(s[1] == y[y1])
            break;
    }
    for(x2 = 0; x2 < 8; x2++){
        if(s[2] == x[x2])
            break;
    }
    for(y2 = 0; y2 < 8; y2++){
        if(s[3] == y[y2])
            break;
    }
    switch(turn){
        case 0:
            for(int i = 1; i < 5; i++){
                if(toupper(s[4]) == whitePieces[i]){
                    coor[4] = 1;
                    break;
                }
            }
            break;
        case 1:
            for(int i = 1; i < 5; i++){
                if(tolower(s[4]) == blackPieces[i]){
                    coor[4] = 1;
                    break;
                }
            }
            break;
    }
    coor[0] = x1;
    coor[1] = y1;
    coor[2] = x2;
    coor[3] = y2;
}

// To prevent killing pieces of the same color
int checkOppPiece(int x1, int y1, int x2, int y2){
    for(int i = 0; i < 6; i++){
        if(board[y1][x1] == whitePieces[i]){
            for(int i = 0; i < 6; i++){
                if(board[y2][x2] == whitePieces[i]){
                    return 0;
                    break;
                }
            }
            break;
        }
    }
    for(int i = 0; i < 6; i++){
        if(board[y1][x1] == blackPieces[i]){
            for(int i = 0; i < 6; i++){
                if(board[y2][x2] == blackPieces[i]){
                    return 0;
                    break;
                }
            }
            break;
        }
    }
    return 1;
}

int checkPawnMove(int x1, int y1, int x2, int y2){
    switch(turn){
        case 0:
            if((y2 == y1 - 1) && (x1 == x2) && ((board[y2][x2] == '-') || (board[y2][x2] == '.')))
                return 1;
            else if((y2 == y1 - 2) && (y1 == 6) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 - 1; i >= y2; i--){
                    if(!((board[i][x1] == '-') || (board[i][x1] == '.'))){
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
                    if(board[y2][x2] == blackPieces[i]){
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
                    if(board[y2][x2] == blackPieces[i]){
                        valid = 1;
                        break;
                    }
                }
                if(valid)
                    return 1;
            }
            break;
        case 1:
            if((y2 == y1 + 1) && (x1 == x2) && ((board[y2][x2] == '-') || (board[y2][x2] == '.')))
                return 1;
            else if((y2 == y1 + 2) && (y1 == 1) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 + 1; i <= y2; i++){
                    if(!((board[i][x1] == '-') || (board[i][x1] == '.'))){
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
                    if(board[y2][x2] == whitePieces[i]){
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
                    if(board[y2][x2] == whitePieces[i]){
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

int checkRookMove(int x1, int y1, int x2, int y2){
    int valid1 = 1, valid2 = 0;
    if((x2 > x1) && (y1 == y2)){
        for(int i = x1 + 1; i < x2; i++){
            if(!((board[y1][i] == '-') || (board[y1][i] == '.'))){
                valid1 = 0;
                break;
            }
        }
        valid2 = checkOppPiece(x1, y1, x2, y2);
        if(valid1 && valid2)
                return 1;
    }
    else if((x2 < x1) && (y1 == y2)){
        for(int i = x1 - 1; i > x2; i--){
            if(!((board[y1][i] == '-') || (board[y1][i] == '.'))){
                    valid1 = 0;
                    break;
            }
        }
        valid2 = checkOppPiece(x1, y1, x2, y2);
        if(valid1 && valid2)
                return 1;
    }
    else if((x2 == x1) && (y1 < y2)){
        for(int i = y1 + 1; i < y2; i++){
            if(!((board[i][x1] == '-') || (board[i][x1] == '.'))){
                    valid1 = 0;
                    break;
            }
        }
        valid2 = checkOppPiece(x1, y1, x2, y2);
        if(valid1 && valid2)
            return 1;
    }
    else if((x2 == x1) && (y1 > y2)){
        for(int i = y1 - 1; i > y2; i--){
            if(!((board[i][x1] == '-') || (board[i][x1] == '.'))){
                valid1 = 0;
                break;
            }
        }
        valid2 = checkOppPiece(x1, y1, x2, y2);
        if(valid1 && valid2)
            return 1;
    }
    return 0;
}

int checkKnightMove(int x1, int y1, int x2, int y2){
    int cond1 = 0, cond2 = 0, cond3 = 0, cond4 = 0, cond5 = 0, cond6 = 0, cond7 = 0, cond8 = 0;
    cond1 = (x2 == x1 + 1) && (y2 == y1 + 2);
    cond2 = (x2 == x1 + 1) && (y2 == y1 - 2);
    cond3 = (x2 == x1 - 1) && (y2 == y1 + 2);
    cond4 = (x2 == x1 - 1) && (y2 == y1 - 2);
    cond5 = (x2 == x1 + 2) && (y2 == y1 + 1);
    cond6 = (x2 == x1 + 2) && (y2 == y1 - 1);
    cond7 = (x2 == x1 - 2) && (y2 == y1 + 1);
    cond8 = (x2 == x1 - 2) && (y2 == y1 - 1);
    if(cond1 || cond2 || cond3 || cond4 || cond5 || cond6 || cond7 || cond8){
        if(checkOppPiece(x1, y1, x2, y2))
            return 1;
    }
    return 0;
}

int checkBishopMove(int x1, int y1, int x2, int y2){
    if(abs(x2 - x1) == abs(y2 - y1)){
        int valid1 = 1, valid2 = 0;
        if((x2 > x1) && ( y2 > y1)){
            for(int i = 1; i < abs(x2 - x1); i++){
                if(!((board[y1 + i][x1 + i] == '-') || (board[y1 + i][x1 + i] == '.'))){
                    valid1 = 0;
                    break;
                }
            }
            valid2 = checkOppPiece(x1, y1, x2, y2);
            if(valid1 && valid2)
                return 1;
        }
        else if((x2 > x1) && (y2 < y1)){
            for(int i = 1; i < abs(x2 - x1); i++){
                if(!((board[y1 - i][x1 + i] == '-')||(board[y1 - i][x1 + i] == '.'))){
                    valid1 = 0;
                    break;
                }
            }
            valid2 = checkOppPiece(x1, y1, x2, y2);
            if(valid1 && valid2)
                return 1;
        }
        else if((x2 < x1) && (y2 > y1)){
            for(int i = 1; i < abs(x2 - x1); i++){
                if(!((board[y1 + i][x1 - i] == '-')||(board[y1 + i][x1 - i] == '.'))){
                    valid1 = 0;
                    break;
                }
            }
            valid2 = checkOppPiece(x1, y1, x2, y2);
            if(valid1 && valid2)
                return 1;
        }
        else if((x2 < x1) && (y2 < y1)){
            for(int i = 1; i < abs(x2 - x1); i++){
                if(!((board[y1 - i][x1 - i] == '-') || (board[y1 - i][x1 - i] == '.'))){
                    valid1 = 0;
                    break;
                }
            }
            valid2 = checkOppPiece(x1, y1, x2, y2);
            if(valid1 && valid2)
                return 1;
        }
    }
    return 0;
}

int checkQueenMove(int x1, int y1, int x2, int y2){
    int valid = (checkBishopMove(x1, y1, x2, y2) || checkRookMove(x1, y1, x2, y2));
    return valid;
}

int checkKingMove(int x1, int y1, int x2, int y2){
    if((abs(x1 - x2) == 1 && y1 == y2) || (abs(y1 - y2) == 1 &&  x1 == x2) || (abs(x1 - x2) == 1 && abs(y1 - y2) == 1)){
        if(checkQueenMove(x1, y1, x2, y2))
            return 1;
    }
    switch(turn){
        case 0:
            if(x1 == 4 && y1 == 7 && x2 == 6 && y2 == 7 && castling[4] == 0 && castling[5] == 0 && board[7][5] == '.' && board[7][6] == '-')
                castle(0);
            else if(x1 == 4 && y1 == 7 && x2 == 2 && y2 == 7 && castling[4] == 0 && castling[3] == 0 && board[7][3] == '.' && board[7][2] == '-' && board[7][1] == '.')
                castle(1);
            break;
        case 1:
            if(x1 == 4 && y1 == 0 && x2 == 6 && y2 == 0 && castling[1] == 0 && castling[2] == 0 && board[0][5] == '-' && board[0][6] == '.')
                castle(2);
            else if(x1 == 4 && y1 == 0 && x2 == 2 && y2 == 0 && castling[1] == 0 && castling[0] == 0 && board[0][3] == '-' && board[0][2] == '.' && board[0][1] == '-')
                castle(3);
            break;
    }
    return 0;
}

int valid(int x1, int y1, int x2, int y2){
    switch(turn){
        case 0:
            switch(board[y1][x1]){
                case 'P':
                    return checkPawnMove(x1, y1, x2, y2);
                    break;
                case 'R':
                    return checkRookMove(x1, y1, x2, y2);
                    break;
                case 'N':
                    return checkKnightMove(x1, y1, x2, y2);
                    break;
                case 'B':
                    return checkBishopMove(x1, y1, x2, y2);
                    break;
                case 'Q':
                    return checkQueenMove(x1, y1, x2, y2);
                    break;
                case 'K':
                    return checkKingMove(x1, y1, x2, y2);
                    break;
            }
            break;
        case 1:
            switch(board[y1][x1]){
                case 'p':
                    return checkPawnMove(x1, y1, x2, y2);
                    break;
                case 'r':
                    return checkRookMove(x1, y1, x2, y2);
                    break;
                case 'n':
                    return checkKnightMove(x1, y1, x2, y2);
                    break;
                case 'b':
                    return checkBishopMove(x1, y1, x2, y2);
                    break;
                case 'q':
                    return checkQueenMove(x1, y1, x2, y2);
                    break;
                case 'k':
                    return checkKingMove(x1, y1, x2, y2);
                    break;
            }
            break;
    }
    return 0;
}

int isCheck(){
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8 ; x++){
            if(board[y][x] == 'K'){
                whiteKingPos[0] = x;
                whiteKingPos[1] = y;
            }
            if(board[y][x] == 'k'){
                blackKingPos[0] = x;
                blackKingPos[1] = y;
            }
        }
    }
    switch(turn){
        case 0:
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, blackKingPos[0], blackKingPos[1]))
                        return 1;
                }
            }
            break;
       case 1:
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, whiteKingPos[0], whiteKingPos[1]))
                        return 1;
                }
            }
            break;
    }
    return 0;
}

int promotion(int x2, int y2, char pieceSwap){
    switch(turn){
        case 0:
            if(y2 == 0 && board[y2][x2] == 'P'){
                for(int i = 1; i < 5; i++){
                    if(toupper(pieceSwap) == whitePieces[i]){
                        board [y2][x2] = whitePieces[i];
                        return 1;
                    }
                }
            }
            break;
        case 1:
            if(y2 == 7 && board[y2][x2] == 'p'){
                for(int i = 1; i < 5; i++){
                    if(tolower(pieceSwap) == blackPieces[i]){
                        board [y2][x2] = blackPieces[i];
                        return 1;
                    }
                }
            }
            break;
    }
    return 0;
}

void move(int x1, int y1, int x2, int y2, int coor4, char s4){
    char temp[8][8];
    // Promotion piece is given? 1:0
    if(coor4 == 0){
        if((board[y1][x1] == 'P' && turn == 0 && y2 == 0) || (board[y1][x1] == 'p' && turn == 1 && y2 == 7))
            return;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                temp[y][x] = board[y][x];
        }
        if(board[y2][x2] != '-' && board[y2][x2] != '.'){
            taken[numTaken] = board[y2][x2];
            numTaken++;
        }
        board[y2][x2] = board[y1][x1];
        board[y1][x1] = ((y1 + x1) % 2 == 0)?'.':'-';
        turn = (turn == 0)?1:0;
        check = isCheck();
        turn = (turn == 0)?1:0;
        if(check == 1){
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++)
                    board[y1][x1] = temp[y1][x1];
            }
            if(board[y2][x2] != '-' && board[y2][x2] != '.')
                numTaken--;
            return;
        }
        check = isCheck();
        turn = (turn == 0)?1:0;
        counter++;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                undoBoard[counter][y][x] = board[y][x];
        }
        undoTaken[counter] = numTaken;
        undoCheck[counter] = check;
        for(int i = 0; i < 6; i++)
            undoCastling[counter][i] = castling[i];
        if(x1 == 0 && y1 ==0)
            castling[0] = 1;
        else if(x1 == 4 && y1 ==0)
            castling[1] = 1;
        else if(x1 == 7 && y1 ==0)
            castling[2] = 1;
        else if(x1 == 0 && y1 ==7)
            castling[3] = 1;
        else if(x1 == 4 && y1 ==7)
            castling[4] = 1;
        else if(x1 == 7 && y1 ==7)
            castling[5] = 1;
    }
    else{
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                temp[y][x] = board[y][x];
        }
        if(board[y2][x2] != '-' && board[y2][x2] != '.'){
            taken[numTaken] = board[y2][x2];
            numTaken++;
        }
        board[y2][x2] = board[y1][x1];
        board[y1][x1] = ((y1 + x1) % 2 == 0)?'.':'-';
        turn = (turn == 0)?1:0;
        check = isCheck();
        turn = (turn == 0)?1:0;
        if(check == 1 || promotion(x2, y2, s4) == 0){
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++)
                    board[y1][x1] = temp[y1][x1];
            }
            if(board[y2][x2] != '-' && board[y2][x2] != '.')
                numTaken--;
            return;
        }
        check = isCheck();
        turn = (turn == 0)?1:0;
        counter++;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                undoBoard[counter][y][x] = board[y][x];
        }
        undoTaken[counter] = numTaken;
        undoCheck[counter] = check;
        for(int i = 0; i < 6; i++)
            undoCastling[counter][i] = castling[i];
    }
}

void undo(){
    if(counter > 0){
        counter--;
        turn = (turn == 0)?1:0;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                board[y][x] = undoBoard[counter][y][x];
        }
        numTaken = undoTaken[counter];
        check = undoCheck[counter];
        for(int i = 0; i < 6; i++)
            castling[i] = undoCastling[counter][i];
    }
}

int checkMate(){
    int i, j, k, m;
    char temp1, temp2;
    switch(turn){
        case 0:
            for(i = 0; i < 8; i++){
                for(j = 0; j < 8; j++){
                   if(isupper(board [j][i])){
                        for(k = 0; k < 8; k++){
                            for(m = 0; m < 8; m++){
                                if(!(isupper(board [m][k]))){
                                    if(valid(i, j, k, m)){
                                        temp2 = board[m][k];
                                        temp1 = board [j][i];
                                        board [j][i] = '-';
                                        board [m][k] = temp1;
                                        turn = (turn == 0)?1:0;
                                        if(!(isCheck())){
                                            board [j][i] = board[m][k];
                                            board [m][k] = temp2;
                                            turn = (turn == 0)?1:0;
                                            return 0;
                                        }
                                        board [j][i] = board [m][k];
                                        board [m][k] = temp2;
                                        turn = (turn == 0)?1:0;
                                    }
                                }
                            }
                        }
                   }
                }
            }
            break;
        case 1:
            for(i = 0; i < 8; i++){
                for(j = 0; j < 8; j++){
                   if(islower(board [j][i])){
                        for(k = 0; k < 8; k++){
                            for(m = 0; m < 8; m++){
                                if(!(islower(board [m][k]))){
                                   if(valid(i, j, k, m)){
                                        temp2 = board [m][k];
                                        temp1 = board [j][i];
                                        board [j][i] = '-';
                                        board [m][k] = temp1;
                                        turn = (turn == 0)?1:0;
                                        if(!(isCheck())){
                                            board [j][i] = board [m][k];
                                            board [m][k] = temp2;
                                            turn = (turn == 0)?1:0;
                                            return 0;
                                        }
                                        board [j][i] = board[m][k];
                                        board [m][k] = temp2;
                                        turn = (turn == 0)?1:0;
                                   }
                                }
                            }
                        }
                   }
                }
            }
        break;
    }
    return 1;
}

int staleMate(){
    int i, j, k, m;
    if(check == 0){
        if(checkMate()){
            printf("Stalemate\n");
            return 1;
        }
    }
    if(numTaken == 29){
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(board [j][i] == 'N' || board [j][i] == 'n' || board [j][i] == 'B' || board [j][i] == 'b'){
                    printf("Draw\n");
                    return 1;
                }
            }
        }
    }
    else if(numTaken == 28){
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(board [j][i] == 'N' || board [j][i] == 'B'){
                    for(k = 0; k < 8; k++){
                        for(m = 0; m < 8; m++){
                            if(board [m][k] == 'n' || board [m][k] == 'b'){
                                printf("Draw\n");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(numTaken == 30){
        printf("Draw\n");
        return 1;
    }
    return 0;
}

int getInput(){
    if(counter == 80){
        printf("Draw");
        return 1;
    }
    int i;
    char s[6];
    char u[] = "undo";
    char sa[] = "save";
    char lo[] = "load";
    if(staleMate())
        return 1;
    if(check){
        if(!(checkMate())){
            printf("Check king\n");
        }
        else{
            switch(turn){
                case 0:
                    printf("Black Wins\n");
                    break;
                case 1:
                    printf("White wins\n");
                    break;
            }
            return 1;
        }
    }
    switch(turn){
        case 0:
            printf("White's turn: ");
            break;
        case 1:
            printf("Black's turn: ");
            break;
    }
    fgets(s, 6, stdin);
    for(i=0; i<4; i++){
        if(tolower(s[i])!=u[i])
            break;
    }
    if(i == 4){
        undo();
        return 0;
    }
    for(i = 0; i < 4; i++){
        if(tolower(s[i]) != sa[i])
            break;
    }
    if(i == 4){
        save();
        return 0;
    }
    for(i = 0; i < 4; i++){
        if(tolower(s[i]) != lo[i])
            break;
    }
    if(i == 4){
        load();
        return 0;
    }
    for(i = 0; i < 4; i = i + 2)
        s[i] = toupper(s[i]);
    int coor[5];
    getCoor(s, coor);
    for(i = 0; i < 4; i++){
        if(coor[i] == 8)
            return 0;
    }
    if(valid(coor[0], coor[1], coor[2], coor[3]))
        move(coor[0], coor[1], coor[2], coor[3], coor[4], s[4]);
    return 0;
}

void printBoard (){
    int i, j;
    printf("       ");
    for(i = 0; i < 8; i++)
        printf("%c      ", x[i]);
    printf("\n\n");
    for(i = 0; i < 8; i++){
        printf("    ");
        for(j = 0; j < 8; j++){
            if((i + j) % 2 == 0)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
            else
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            printf("       ");
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        printf("\n");
        printf("%c   ", y[i]);
        for(j = 0; j < 8; j++){
            if((i + j) % 2 == 0)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
            else
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            printf("   %c   ", board[i][j]);
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        printf("   %c", y[i]);
        switch(i){
            case 0:
                printf("\n");
                break;
            case 1:
                printf("\t\t\tTaken white pieces:\n");
                break;
            case 2:
                printf("\t\t\t");
                for(int k = 0; k < numTaken; k++){
                    for(int l = 0; l < 6; l++){
                        if(taken[k] == whitePieces[l])
                            printf("%c  ", taken[k]);
                    }
                }
                printf("\n");
                break;
            case 3:
                printf("\n");
                break;
            case 4:
                printf("\n");
                break;
            case 5:
                printf("\t\t\tTaken black pieces:\n");
                break;
            case 6:
                printf("\t\t\t");
                for(int k = 0; k < numTaken; k++){
                    for(int l = 0; l < 6; l++){
                        if(taken[k] == blackPieces[l])
                            printf("%c  ", taken[k]);
                    }
                }
                printf("\n");
                break;
            case 7:
                printf("\n");
                break;
        }
                printf("    ");
        for(j = 0; j < 8; j++){
            if((i + j) % 2 == 0)
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
            else
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
            printf("       ");
        }
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
        printf("\n");
    }
    printf("\n");
    printf("       ");
    for(i = 0; i < 8; i++)
        printf("%c      ", x[i]);
    printf("\n\n\n");
}


int main(){
    // First undo move
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            undoBoard[0][y][x] = board[y][x];
    }
    undoTaken[0] = 0;
    undoCheck[0] = 0;
    for(int i = 0; i < 6; i++)
        undoCastling[0][i] = castling[i];
    while(1){
        printBoard();
        if(getInput())
            break;
        system("cls");
    }
    // To display the last message before termination(e.g. Stalemate, White wins.. etc)
    while(getchar() != '\n');
    return 0;
}
