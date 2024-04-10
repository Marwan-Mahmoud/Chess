#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

#define WHITE BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY
#define BLACK FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED

// Initial Board
char board [8][8] = {
    {'r','n','b','q','k','b','n','r'},
    {'p','p','p','p','p','p','p','p'},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
    {' ',' ',' ',' ',' ',' ',' ',' '},
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
int firstUndo = 0;

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
    int counter;
}data;

void printBoard ();

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
    data.counter = counter;
    fwrite(&data, sizeof(data), 1, fp);
    fclose(fp);
}

void load(){
    FILE *fp;
    fp = fopen("SaveData.txt", "rb");
    if(fp == NULL)
        exit(1);
    fread(&data, sizeof(data), 1, fp);
    counter = firstUndo = data.counter;
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            board[y][x] = undoBoard[counter][y][x] = data.board[y][x];
    }
    turn = data.turn;
    check = undoCheck[counter] = data.check;
    numTaken = undoTaken[counter] = data.numTaken;
    for(int i = 0; i < numTaken; i++)
        taken[i] = data.taken[i];
    for(int i = 0; i < 6; i++)
        castling[i] = undoCastling[counter][i] = data.castling[i];
    fclose(fp);
    printBoard();
}

void undo(){
    if(counter > firstUndo){
        counter--;
        turn = turn == 0;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++)
                board[y][x] = undoBoard[counter][y][x];
        }
        numTaken = undoTaken[counter];
        check = undoCheck[counter];
        for(int i = 0; i < 6; i++)
            castling[i] = undoCastling[counter][i];
        printBoard();
    }
}

int valid(int x1, int y1, int x2, int y2);

int updateBoard(int x1, int y1, int x2, int y2);

void printPiece (char p, int t);

void drawButton(int key, int color){
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    char word[4];
    switch(key){
        case 0:
            coord.X = 63;
            for(int i = 0; i < 4; i++)
                word[i] = "Undo"[i];
            break;
        case 1:
            coord.X = 77;
            for(int i = 0; i < 4; i++)
                word[i] = "Save"[i];
            break;
        case 2:
            coord.X = 91;
            for(int i = 0; i < 4; i++)
                word[i] = "Load"[i];
            break;
        default:
            coord.X = 75 + (key - 3) * 4;
    }
    SetConsoleTextAttribute(hOutput, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | (BACKGROUND_INTENSITY * color));
    if (key < 3){
        coord.Y = 18;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L"\x250F\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2513 ");
        coord.Y++;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L"\x2503   ");
        for(int i = 0; i < 4; i++)
            wprintf(L"%c", word[i]);
        wprintf(L"  \x2503 ");
        coord.Y++;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L"\x2517\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x251B ");
    }
    else {
        char piece = turn == 0 ? whitePieces[key - 2] : blackPieces[key - 2];
        coord.Y = 23;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L"    ");
        coord.Y++;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L" ");
        printPiece(piece, 0);
        wprintf(L" ");
        coord.Y++;
        SetConsoleCursorPosition(hOutput, coord);
        wprintf(L"    ");
    }
    SetConsoleTextAttribute(hOutput, BLACK);
}

void castle(int c){
    switch(c){
        case 0:
            board[7][4] = board[7][7] = ' ';
            board[7][6] = 'K';
            board[7][5] = 'R';
            turn = turn == 0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 6, 7) || valid(x1, y1, 5, 7)){
                        board[7][4] = 'K';
                        board[7][7] = 'R';
                        board[7][6] = board[7][5] = ' ';
                        turn = turn == 0;
                        return;
                    }
                }
            }
            counter++;
            updateBoard(4, 7, 6, 7);
            updateBoard(7, 7, 5, 7);
            break;
        case 1:
            board[7][4] = board[7][0] = ' ';
            board[7][2] = 'K';
            board[7][3] = 'R';
            turn = turn == 0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 2, 7) || valid(x1, y1, 3, 7)){
                        board[7][4] = 'K';
                        board[7][0] = 'R';
                        board[7][2] = board[7][3] = ' ';
                        turn = turn == 0;
                        return;
                    }
                }
            }
            counter++;
            updateBoard(4, 7, 2, 7);
            updateBoard(0, 7, 3, 7);
            break;
        case 2:
            board[0][4] = board[0][7] = ' ';
            board[0][6] = 'k';
            board[0][5] = 'r';
            turn = turn == 0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 6, 0) || valid(x1, y1, 5, 0)){
                        board[0][4] = 'k';
                        board[0][7] = 'r';
                        board[0][6] = board[0][5] = ' ';
                        turn = turn == 0;
                        return;
                    }
                }
            }
            counter++;
            updateBoard(4, 0, 6, 0);
            updateBoard(7, 0, 5, 0);
            break;
        case 3:
            board[0][4] = board[0][0] = ' ';
            board[0][2] = 'k';
            board[0][3] = 'r';
            turn = turn == 0;
            for(int y1 = 0; y1 < 8; y1++){
                for(int x1 = 0; x1 < 8; x1++){
                    if(valid(x1, y1, 2, 0) || valid(x1, y1, 3, 0)){
                        board[0][4] = 'k';
                        board[0][0] = 'r';
                        board[0][2] = board[0][3] = ' ';
                        turn = turn == 0;
                        return;
                    }
                }
            }
            counter++;
            updateBoard(4, 0, 2, 0);
            updateBoard(0, 0, 3, 0);
            break;
    }
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
void getCoor(int coor[], char *p){
    INPUT_RECORD InputRecord;
    DWORD Events;
    COORD coord;
    static COORD prev;
    static int iprev, jprev;
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    ReadConsoleInput(hInput, &InputRecord, 1, &Events);
    coord.X = InputRecord.Event.MouseEvent.dwMousePosition.X;
    coord.Y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
    static int printButton = 11;
    for(int i = 0; i < 3; i++){
        if(coord.X >= (63 + i*14) && coord.X <= (74 + i*14) && coord.Y >= 18 && coord.Y <= 21){
            if (printButton == 11){
                printButton = i;
                drawButton(i, 0);
                break;
            }
            if(InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && InputRecord.Event.MouseEvent.dwEventFlags != MOUSE_MOVED){
                switch (i){
                    case 0:
                        undo();
                        break;
                    case 1:
                        save();
                        break;
                    case 2:
                        load();
                        break;
                }
            }
        }
    }
    if(printButton != 11 && !(coord.X >= (63 + printButton*14) && coord.X <= (74 + printButton*14) && coord.Y >= 18 && coord.Y <= 21)){
        drawButton(printButton, 1);
        printButton = 11;
    }
    if(InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && InputRecord.Event.MouseEvent.dwEventFlags != MOUSE_MOVED){
        coord.X = InputRecord.Event.MouseEvent.dwMousePosition.X;
        coord.Y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
        for(int y = 0; y < 8; y++){
            for(int x = 0; x < 8; x++){
                if(coord.X >= (4 + x*6) && coord.X <= (9 + x*6) && coord.Y >= (2 + y*3) && coord.Y <= (4 + y*3)){
                    if(prev.X){
                        SetConsoleCursorPosition(hOutput, prev);
                        (iprev + jprev) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
                        wprintf(L"      ");
                        prev.Y += 2;
                        SetConsoleCursorPosition(hOutput, prev);
                        wprintf(L"      ");
                        prev.Y--;
                        SetConsoleCursorPosition(hOutput, prev);
                        wprintf(L" ");
                        prev.X += 5;
                        SetConsoleCursorPosition(hOutput, prev);
                        wprintf(L" ");
                    }
                    coord.X = prev.X = 4 + x*6; coord.Y = prev.Y = 2 + y*3;
                    iprev = y; jprev = x;
                    SetConsoleCursorPosition(hOutput, coord);
                    SetConsoleTextAttribute(hOutput, ((BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY) * !((y + x) % 2)) | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    wprintf(L"\x2588\x2580\x2580\x2580\x2580\x2588");
                    coord.Y += 2;
                    SetConsoleCursorPosition(hOutput, coord);
                    wprintf(L"\x2588\x2584\x2584\x2584\x2584\x2588");
                    coord.Y--;
                    SetConsoleCursorPosition(hOutput, coord);
                    wprintf(L"\x2588");
                    coord.X += 5;
                    SetConsoleCursorPosition(hOutput, coord);
                    wprintf(L"\x2588");
                    if(coor[0] == 8){
                        coor[0] = x;
                        coor[1] = y;
                    }
                    else{
                        coor[2] = x;
                        coor[3] = y;
                    }
                }
            }
        }
    }
    if(((board[coor[1]][coor[0]] == 'P' && turn == 0 && coor[3] == 0) || (board[coor[1]][coor[0]] == 'p' && turn == 1 && coor[3] == 7)) && valid(coor[0], coor[1], coor[2], coor[3])){
        drawButton(3, 1);
        drawButton(4, 1);
        drawButton(5, 1);
        drawButton(6, 1);
        coor[4] = 1;
        while (1){
            ReadConsoleInput(hInput, &InputRecord, 1, &Events);
            coord.X = InputRecord.Event.MouseEvent.dwMousePosition.X;
            coord.Y = InputRecord.Event.MouseEvent.dwMousePosition.Y;
            for(int i = 0; i < 4; i++){
                if(coord.X >= (75 + i*4) && coord.X <= (78 + i*4) && coord.Y >= 23 && coord.Y <= 25){
                    if (printButton == 11){
                        printButton = i + 3;
                        drawButton(printButton, 0);
                        break;
                    }
                    if(InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED){
                        switch (i){
                            case 0:
                                *p = turn == 0 ? 'R':'r';
                                break;
                            case 1:
                                *p = turn == 0 ? 'N':'n';
                                break;
                            case 2:
                                *p = turn == 0 ? 'B':'b';
                                break;
                            case 3:
                                *p = turn == 0 ? 'Q':'q';
                                break;
                        }
                        printButton = 11;
                        coord.X = 75; coord.Y = 23;
                        SetConsoleCursorPosition(hOutput, coord);
                        wprintf(L"                ");
                        coord.Y++;
                        SetConsoleCursorPosition(hOutput, coord);
                        wprintf(L"                ");
                        coord.Y++;
                        SetConsoleCursorPosition(hOutput, coord);
                        wprintf(L"                ");
                        return;
                    }
                }
            }
            if(printButton != 11 && !(coord.X >= (75 + (printButton - 3)*4) && coord.X <= (78 + (printButton - 3)*4) && coord.Y >= 23 && coord.Y <= 25)){
                drawButton(printButton, 1);
                printButton = 11;
            }
        }
    }
    else
        coor[4] = 0;
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
            if((y2 == y1 - 1) && (x1 == x2) && (board[y2][x2] == ' '))
                return 1;
            else if((y2 == y1 - 2) && (y1 == 6) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 - 1; i >= y2; i--){
                    if(!(board[i][x1] == ' ')){
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
            if((y2 == y1 + 1) && (x1 == x2) && (board[y2][x2] == ' '))
                return 1;
            else if((y2 == y1 + 2) && (y1 == 1) && (x1 == x2)){
                int valid = 1;
                for(int i = y1 + 1; i <= y2; i++){
                    if(!(board[i][x1] == ' ')){
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
            if(!(board[y1][i] == ' ')){
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
            if(!(board[y1][i] == ' ')){
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
            if(!(board[i][x1] == ' ')){
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
            if(!(board[i][x1] == ' ')){
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
                if(!(board[y1 + i][x1 + i] == ' ')){
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
                if(!(board[y1 - i][x1 + i] == ' ')){
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
                if(!(board[y1 + i][x1 - i] == ' ')){
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
                if(!(board[y1 - i][x1 - i] == ' ')){
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
            if(x1 == 4 && y1 == 7 && x2 == 6 && y2 == 7 && castling[4] == 0 && castling[5] == 0 && board[7][5] == ' ' && board[7][6] == ' ')
                castle(0);
            else if(x1 == 4 && y1 == 7 && x2 == 2 && y2 == 7 && castling[4] == 0 && castling[3] == 0 && board[7][3] == ' ' && board[7][2] == ' ' && board[7][1] == ' ')
                castle(1);
            break;
        case 1:
            if(x1 == 4 && y1 == 0 && x2 == 6 && y2 == 0 && castling[1] == 0 && castling[2] == 0 && board[0][5] == ' ' && board[0][6] == ' ')
                castle(2);
            else if(x1 == 4 && y1 == 0 && x2 == 2 && y2 == 0 && castling[1] == 0 && castling[0] == 0 && board[0][3] == ' ' && board[0][2] == ' ' && board[0][1] == ' ')
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

void move(int x1, int y1, int x2, int y2, int coor4, char pieceSwap){
    char temp;
    if(board[y2][x2] != ' '){
        taken[numTaken] = board[y2][x2];
        numTaken++;
    }
    temp = board[y2][x2];
    board[y2][x2] = board[y1][x1];
    board[y1][x1] = ' ';
    turn = turn == 0;
    check = isCheck();
    turn = turn == 0;
    // Promotion piece is given? 1:0
    if(coor4 == 0){
        if(check == 1){
            board[y1][x1] = board[y2][x2];
            board[y2][x2] = temp;
            if(board[y2][x2] != ' ')
                numTaken--;
            return;
        }
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
        if(check == 1 || promotion(x2, y2, pieceSwap) == 0){
            board[y1][x1] = board[y2][x2];
            board[y2][x2] = temp;
            if(board[y2][x2] != ' ')
                numTaken--;
            return;
        }
    }
    check = isCheck();
    turn = turn == 0;
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
                                        board [j][i] = ' ';
                                        board [m][k] = temp1;
                                        turn = turn == 0;
                                        if(!(isCheck())){
                                            board [j][i] = board[m][k];
                                            board [m][k] = temp2;
                                            turn = turn == 0;
                                            return 0;
                                        }
                                        board [j][i] = board [m][k];
                                        board [m][k] = temp2;
                                        turn = turn == 0;
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
                                        board [j][i] = ' ';
                                        board [m][k] = temp1;
                                        turn = turn == 0;
                                        if(!(isCheck())){
                                            board [j][i] = board [m][k];
                                            board [m][k] = temp2;
                                            turn = turn == 0;
                                            return 0;
                                        }
                                        board [j][i] = board[m][k];
                                        board [m][k] = temp2;
                                        turn = turn == 0;
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
            wprintf(L"Stalemate");
            return 1;
        }
    }
    if(numTaken == 29){
        for(i = 0; i < 8; i++){
            for(j = 0; j < 8; j++){
                if(board [j][i] == 'N' || board [j][i] == 'n' || board [j][i] == 'B' || board [j][i] == 'b'){
                    wprintf(L"Draw");
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
                                wprintf(L"Draw");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(numTaken == 30){
        wprintf(L"Draw");
        return 1;
    }
    return 0;
}

int getInput(){
    static int coor[5] = {8, 8, 8, 8, 0};
    char p;
    getCoor(coor, &p);
    for(int i = 0; i < 5; i++){
        if(coor[i] == 8)
            return 0;
    }
    int returnValue;
    if(valid(coor[0], coor[1], coor[2], coor[3])){
        move(coor[0], coor[1], coor[2], coor[3], coor[4], p);
        returnValue = updateBoard(coor[0], coor[1], coor[2], coor[3]);
        coor[0] = coor[1] = coor[2] = coor[3] = 8;
        return returnValue;
    }
    else{
        coor[0] = coor[2];
        coor[1] = coor[3];
    }
    return 0;
}

void printPiece (char p, int t){
    // t ? 0:White tile  1:Black tile
    switch(p){
        case 'P':
            t == 0 ? wprintf(L"\x2659"):wprintf(L"\x265F");
            break;
        case 'R':
            t == 0 ? wprintf(L"\x2656"):wprintf(L"\x265C");
            break;
        case 'N':
            t == 0 ? wprintf(L"\x2658"):wprintf(L"\x265E");
            break;
        case 'B':
            t == 0 ? wprintf(L"\x2657"):wprintf(L"\x265D");
            break;
        case 'Q':
            t == 0 ? wprintf(L"\x2655"):wprintf(L"\x265B");
            break;
        case 'K':
            t == 0 ? wprintf(L"\x2654"):wprintf(L"\x265A");
            break;
        case 'p':
            t == 0 ? wprintf(L"\x265F"):wprintf(L"\x2659");
            break;
        case 'r':
            t == 0 ? wprintf(L"\x265C"):wprintf(L"\x2656");
            break;
        case 'n':
            t == 0 ? wprintf(L"\x265E"):wprintf(L"\x2658");
            break;
        case 'b':
            t == 0 ? wprintf(L"\x265D"):wprintf(L"\x2657");
            break;
        case 'q':
            t == 0 ? wprintf(L"\x265B"):wprintf(L"\x2655");
            break;
        case 'k':
            t == 0 ? wprintf(L"\x265A"):wprintf(L"\x2654");
            break;
        default:
            wprintf(L"  ");
        }
}

void printBoard (){
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    int i, j;
    coord.X = 0; coord.Y = 0;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"%d      ", counter);
    for(i = 0; i < 8; i++)
        wprintf(L"%c     ", x[i]);
    wprintf(L"\n\n");
    for(i = 0; i < 8; i++){
        wprintf(L"    ");
        for(j = 0; j < 8; j++){
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
            wprintf(L"      ");
        }
        SetConsoleTextAttribute(hOutput, BLACK);
        wprintf(L"\n%c   ", y[i]);
        for(j = 0; j < 8; j++){
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
            wprintf(L"  ");
            printPiece(board[i][j], (i + j) % 2);
            wprintf(L"  ");
        }
        SetConsoleTextAttribute(hOutput, BLACK);
        wprintf(L"   %c", y[i]);
        wprintf(L"\n    ");
        for(j = 0; j < 8; j++){
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
            wprintf(L"      ");
        }
        SetConsoleTextAttribute(hOutput, BLACK);
        wprintf(L"\n");
    }
    wprintf(L"\n      ");
    for(i = 0; i < 8; i++)
        wprintf(L"%c     ", x[i]);

    coord.X = 75; coord.Y = 3;
    SetConsoleCursorPosition(hOutput, coord);
    turn == 0 ? wprintf(L"White's turn"):wprintf(L"Black's turn");
    coord.X = 72; coord.Y = 8;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"Taken white pieces:");
    coord.X = 72; coord.Y = 13;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"Taken black pieces:");
    coord.X = 62; coord.Y = 10;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"                                             ");
    SetConsoleCursorPosition(hOutput, coord);
    for(int i = 0; i < numTaken; i++){
        for(int j = 0; j < 6; j++){
            if(taken[i] == whitePieces[j]){
                printPiece(taken[i], 1);
                wprintf(L" ");
            }
        }
    }
    coord.X = 62; coord.Y = 15;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"                                             ");
    SetConsoleCursorPosition(hOutput, coord);
    for(int i = 0; i < numTaken; i++){
        for(int j = 0; j < 6; j++){
            if(taken[i] == blackPieces[j]){
                printPiece(taken[i], 1);
                wprintf(L" ");
            }
        }
    }
}

int updateBoard(int x1, int y1, int x2, int y2){
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = 6 + x1*6; coord.Y = 3 + y1*3;
    SetConsoleCursorPosition(hOutput, coord);
    (x1 + y1) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
    printPiece(board[y1][x1], (y1 + x1) % 2);
    coord.X = 6 + x2*6; coord.Y = 3 + y2*3;
    SetConsoleCursorPosition(hOutput, coord);
    (x2 + y2) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE) : SetConsoleTextAttribute(hOutput, BLACK);
    printPiece(board[y2][x2], (y2 + x2) % 2);
    SetConsoleTextAttribute(hOutput, BLACK);
    coord.X = 0; coord.Y = 0;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"%d", counter);
    coord.X = 62; coord.Y = 10;
    SetConsoleCursorPosition(hOutput, coord);
    for(int i = 0; i < numTaken; i++){
        for(int j = 0; j < 6; j++){
            if(taken[i] == whitePieces[j]){
                printPiece(taken[i], 1);
                wprintf(L" ");
            }
        }
    }
    coord.X = 62; coord.Y = 15;
    SetConsoleCursorPosition(hOutput, coord);
    for(int i = 0; i < numTaken; i++){
        for(int j = 0; j < 6; j++){
            if(taken[i] == blackPieces[j]){
                printPiece(taken[i], 1);
                wprintf(L" ");
            }
        }
    }
    coord.X = 75; coord.Y = 3;
    SetConsoleCursorPosition(hOutput, coord);
    turn == 0 ? wprintf(L"White"):wprintf(L"Black");
    coord.X = 75; coord.Y = 5;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"          ");
    SetConsoleCursorPosition(hOutput, coord);
    if(counter == 80){
        wprintf(L"Draw");
        return 1;
    }
    if(staleMate())
        return 1;
    if(check){
        if(!(checkMate())){
            wprintf(L"Check king");
        }
        else{
            turn == 0 ? wprintf(L"Black Wins"):wprintf(L"White wins");
            return 1;
        }
    }
    return 0;
}


int main(){
    SetConsoleTitle("Chess");
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = TMPF_TRUETYPE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"MS Gothic");
    SetCurrentConsoleFontEx(hOutput, FALSE, &cfi);
    _setmode(_fileno(stdout), _O_U16TEXT);
    ShowWindow(GetConsoleWindow(), SW_SHOWMAXIMIZED);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOutput, &csbi);
    while(!(csbi.dwMaximumWindowSize.Y > 27 && csbi.dwMaximumWindowSize.Y < 30 && csbi.dwMaximumWindowSize.X > 110 && csbi.dwMaximumWindowSize.X < 115)){
        cfi.dwFontSize.Y++;
        SetCurrentConsoleFontEx(hOutput, FALSE, &cfi);
        GetConsoleScreenBufferInfo(hOutput, &csbi);
    }
    SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
    SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);
    CONSOLE_CURSOR_INFO cci;
    cci.dwSize = 50;
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hOutput, &cci);

    // First undo move
    for(int y = 0; y < 8; y++){
        for(int x = 0; x < 8; x++)
            undoBoard[0][y][x] = board[y][x];
    }
    undoTaken[0] = 0;
    undoCheck[0] = 0;
    for(int i = 0; i < 6; i++)
        undoCastling[0][i] = castling[i];

    printBoard();
    drawButton(0, 1);
    drawButton(1, 1);
    drawButton(2, 1);
    while(1){
        if(getInput())
            break;
    }
    // To display the last message before termination(e.g. Stalemate, White wins.. etc)
    getchar();
    return 0;
}
