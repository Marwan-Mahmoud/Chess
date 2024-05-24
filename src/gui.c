#include "gui.h"

HANDLE hInput;
HANDLE hOutput;

struct button buttons[3] = {{"Undo", {63, 18}}, {"Save", {77, 18}}, {"Load", {91, 18}}};
struct button pieces[4] = {{"Rr", {75, 23}}, {"Nn", {79, 23}}, {"Bb", {83, 23}}, {"Qq", {87, 23}}};
void (*guiFunctions[])() = {undo, save, load};

void initGUI(struct state *state) {
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    setup();

    initBoard();
    printBoard(state->board);
    printCurrentTurn(state->counter, state->turn);
    printTakenPieces(state->numTaken, state->taken);
    for (int i = 0; i < 3; i++)
        drawButton(buttons[i], 0);
}

void setup() {
    HMONITOR hMonitor = MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFOEX monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(hMonitor, (LPMONITORINFO)&monitorInfo);
    int width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = width / 113;
    cfi.dwFontSize.Y = cfi.dwFontSize.X * 2;
    cfi.FontFamily = TMPF_TRUETYPE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"MS Gothic");
    SetCurrentConsoleFontEx(hOutput, FALSE, &cfi);

    SetConsoleTitle("Chess");
    HWND hConsole = GetConsoleWindow();
    ShowWindow(hConsole, SW_SHOWMAXIMIZED);
    SetWindowLong(hConsole, GWL_STYLE, GetWindowLong(hConsole, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX);
    _setmode(_fileno(stdout), _O_U16TEXT);
    SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT);

    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(hOutput, &cci);
    cci.bVisible = FALSE;
    SetConsoleCursorInfo(hOutput, &cci);
}

void initBoard() {
    char x[8] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    char y[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};

    COORD coord = {6, 0};
    SetConsoleCursorPosition(hOutput, coord);

    for (int i = 0; i < 8; i++)
        wprintf(L"%c     ", x[i]);
    wprintf(L"\n\n");

    for (int i = 0; i < 8; i++) {
        wprintf(L"    ");
        for (int j = 0; j < 8; j++) {
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE_BACKGROUND) : SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
            wprintf(L"      ");
        }

        SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
        wprintf(L"\n%c   ", y[i]);
        for (int j = 0; j < 8; j++) {
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE_BACKGROUND) : SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
            wprintf(L"      ");
        }
        SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
        wprintf(L"   %c\n", y[i]);

        wprintf(L"    ");
        for (int j = 0; j < 8; j++) {
            (i + j) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE_BACKGROUND) : SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
            wprintf(L"      ");
        }
        SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
        wprintf(L"\n");
    }

    wprintf(L"\n      ");
    for (int i = 0; i < 8; i++)
        wprintf(L"%c     ", x[i]);
}

void refresh(struct state *state) {
    printBoard(state->board);
    printCurrentTurn(state->counter, state->turn);
    printTakenPieces(state->numTaken, state->taken);
    state->check ? printMessage("Check!") : printMessage("              ");
}

// Get coordinate
void getCoor(char board[8][8], int turn, COORD *boardPosition) {
    INPUT_RECORD InputRecord;
    DWORD Events;
    ReadConsoleInput(hInput, &InputRecord, 1, &Events);
    COORD mousePosition = InputRecord.Event.MouseEvent.dwMousePosition;
    static int hoveredButton = -1;

    // Check if mouse is on a button
    for (int i = 0; i < 3; i++) {
        if (mousePosition.X >= buttons[i].pos.X && mousePosition.X <= buttons[i].pos.X + 11 && mousePosition.Y >= buttons[i].pos.Y && mousePosition.Y <= buttons[i].pos.Y + 3) {
            if (hoveredButton == -1) {
                hoveredButton = i;
                drawButton(buttons[i], 1);
                break;
            }
            if (InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && InputRecord.Event.MouseEvent.dwEventFlags != MOUSE_MOVED)
                guiFunctions[i]();
        }
    }
    if (hoveredButton != -1 && !(mousePosition.X >= buttons[hoveredButton].pos.X && mousePosition.X <= buttons[hoveredButton].pos.X + 11 && mousePosition.Y >= buttons[hoveredButton].pos.Y && mousePosition.Y <= buttons[hoveredButton].pos.Y + 3)) {
        drawButton(buttons[hoveredButton], 0);
        hoveredButton = -1;
    }

    // Check if mouse is clicked on board
    if (InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && InputRecord.Event.MouseEvent.dwEventFlags != MOUSE_MOVED) {
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                if (mousePosition.X >= (4 + x * 6) && mousePosition.X <= (9 + x * 6) && mousePosition.Y >= (2 + y * 3) && mousePosition.Y <= (4 + y * 3)) {
                    boardPosition->X = x;
                    boardPosition->Y = y;
                    drawActiveSquare(x, y);
                }
            }
        }
    }
}

char getPromotionPiece(int turn) {
    INPUT_RECORD InputRecord;
    DWORD Events;
    int hoveredButton = -1;

    // Draw promotion pieces buttons
    for (int i = 0; i < 4; i++)
        drawPieceButton(pieces[i].label[turn], pieces[i].pos, 0);

    while (1) {
        ReadConsoleInput(hInput, &InputRecord, 1, &Events);
        COORD coord = InputRecord.Event.MouseEvent.dwMousePosition;
        for (int i = 0; i < 4; i++) {
            if (coord.X >= pieces[i].pos.X && coord.X <= pieces[i].pos.X + 3 && coord.Y >= pieces[i].pos.Y && coord.Y <= pieces[i].pos.Y + 2) {
                if (hoveredButton == -1) {
                    hoveredButton = i;
                    drawPieceButton(pieces[i].label[turn], pieces[i].pos, 1);
                    break;
                }
                if (InputRecord.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                    char p = pieces[i].label[turn];
                    hoveredButton = -1;

                    // Clear promotion buttons
                    coord = pieces[0].pos;
                    for (int i = 0; i < 3; i++) {
                        SetConsoleCursorPosition(hOutput, coord);
                        wprintf(L"                ");
                        coord.Y++;
                    }
                    return p;
                }
            }
        }
        if (hoveredButton != -1 && !(coord.X >= pieces[hoveredButton].pos.X && coord.X <= pieces[hoveredButton].pos.X + 3 && coord.Y >= pieces[hoveredButton].pos.Y && coord.Y <= pieces[hoveredButton].pos.Y + 2)) {
            drawPieceButton(pieces[hoveredButton].label[turn], pieces[hoveredButton].pos, 0);
            hoveredButton = -1;
        }
    }
}

void printBoard(char board[8][8]) {
    COORD coord;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            coord.X = 6 + x * 6;
            coord.Y = 3 + y * 3;
            SetConsoleCursorPosition(hOutput, coord);
            (x + y) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE_BACKGROUND) : SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
            printPiece(board[y][x], (x + y) % 2);
        }
    }
    SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
}

void printCurrentTurn(int counter, int turn) {
    COORD coord = {0, 0};
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"   ");
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"%d", counter);

    coord.X = 75; coord.Y = 3;
    SetConsoleCursorPosition(hOutput, coord);
    turn == WHITE ? wprintf(L"White's turn") : wprintf(L"Black's turn");
}

void printTakenPieces(int numTaken, char taken[30]) {
    COORD coord = {72, 8};
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"Taken white pieces:");

    coord.Y = 13;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"Taken black pieces:");

    coord.X = 62;
    coord.Y = 10;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"                                             ");
    SetConsoleCursorPosition(hOutput, coord);
    for (int i = 0; i < numTaken; i++) {
        if (isWhitePiece(taken[i])) {
            printPiece(taken[i], BLACK);
            wprintf(L" ");
        }
    }

    coord.X = 62;
    coord.Y = 15;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"                                             ");
    SetConsoleCursorPosition(hOutput, coord);
    for (int i = 0; i < numTaken; i++) {
        if (isBlackPiece(taken[i])) {
            printPiece(taken[i], BLACK);
            wprintf(L" ");
        }
    }
}

void printMessage(char *message) {
    COORD coord = {75, 5};
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"%s", message);
}

void drawButton(struct button b, int hover) {
    SetConsoleTextAttribute(hOutput, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | (BACKGROUND_INTENSITY * !hover));

    COORD coord = b.pos;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"\x250F\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2513 ");

    coord.Y++;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"\x2503   %s  \x2503 ", b.label);

    coord.Y++;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"\x2517\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x2501\x251B ");

    SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
}

void drawPieceButton(char piece, COORD coord, int hover) {
    SetConsoleTextAttribute(hOutput, BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | (BACKGROUND_INTENSITY * !hover));

    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"    ");

    coord.Y++;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L" ");
    printPiece(piece, WHITE);
    wprintf(L" ");

    coord.Y++;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"    ");

    SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
}

void drawActiveSquare(int x, int y) {
    static int prevX = 0, prevY = 0;

    // Clear previous active square
    COORD coord = {4 + prevX * 6, 2 + prevY * 3};
    SetConsoleCursorPosition(hOutput, coord);
    (prevX + prevY) % 2 == 0 ? SetConsoleTextAttribute(hOutput, WHITE_BACKGROUND) : SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
    wprintf(L"      ");
    coord.Y += 2;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L"      ");
    coord.Y--;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L" ");
    coord.X += 5;
    SetConsoleCursorPosition(hOutput, coord);
    wprintf(L" ");

    // Draw new active square
    coord.X = 4 + x * 6;
    coord.Y = 2 + y * 3;
    SetConsoleCursorPosition(hOutput, coord);
    SetConsoleTextAttribute(hOutput, ((WHITE_BACKGROUND) * !((y + x) % 2)) | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
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

    SetConsoleTextAttribute(hOutput, BLACK_BACKGROUND);
    prevX = x; prevY = y;
}

void printPiece(char piece, int tileColor) {
    switch (piece) {
    case 'P':
        tileColor == WHITE ? wprintf(L"\x2659") : wprintf(L"\x265F");
        break;
    case 'R':
        tileColor == WHITE ? wprintf(L"\x2656") : wprintf(L"\x265C");
        break;
    case 'N':
        tileColor == WHITE ? wprintf(L"\x2658") : wprintf(L"\x265E");
        break;
    case 'B':
        tileColor == WHITE ? wprintf(L"\x2657") : wprintf(L"\x265D");
        break;
    case 'Q':
        tileColor == WHITE ? wprintf(L"\x2655") : wprintf(L"\x265B");
        break;
    case 'K':
        tileColor == WHITE ? wprintf(L"\x2654") : wprintf(L"\x265A");
        break;
    case 'p':
        tileColor == WHITE ? wprintf(L"\x265F") : wprintf(L"\x2659");
        break;
    case 'r':
        tileColor == WHITE ? wprintf(L"\x265C") : wprintf(L"\x2656");
        break;
    case 'n':
        tileColor == WHITE ? wprintf(L"\x265E") : wprintf(L"\x2658");
        break;
    case 'b':
        tileColor == WHITE ? wprintf(L"\x265D") : wprintf(L"\x2657");
        break;
    case 'q':
        tileColor == WHITE ? wprintf(L"\x265B") : wprintf(L"\x2655");
        break;
    case 'k':
        tileColor == WHITE ? wprintf(L"\x265A") : wprintf(L"\x2654");
        break;
    default:
        wprintf(L"  ");
    }
}
