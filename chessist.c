#include <windows.h>
#include <commctrl.h> // includes the common control header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
#include "chess.h"
#define MAKE_GLOBALS_HERE
#include "chess.glb"
#include "chess.fun"
#include "chess.mac"
#include "chessist.h"
#include "resource.h"

static char appname[] = "Chessistant";
#define ID_TOOLBAR         1

#define TOOLBAR_HEIGHT 29

static RECT spi_rect;

static int width_in_pixels;
static int height_in_pixels;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

static char read_game_failure[] =
  "read_game() of %s failed: %d, curr_move = %d";

static char read_binary_game_failure[] =
  "read_binary_game() of %s failed: %d, curr_move = %d";

static int bChangesMade;

static char *chess_piece_bitmap_names[] = {
  "BIGBMP2",
  "BIGBMP"
};

static HANDLE chess_piece_bitmap_handles[2];
static HDC hdc_compatible[2];

static OPENFILENAME OpenFileName;
static TCHAR szFile[MAX_PATH];
static OPENFILENAME ReadBoardName;
static TCHAR szReadBoardName[MAX_PATH];
static OPENFILENAME WriteFileName;
static TCHAR szWriteFileName[MAX_PATH];

static char chess_filter[] = "\
Chess files\0\
*.ch\0\
All files (*.*)\0\
*.*\0\
\0\
\0\
";
static char ch_ext[] = "ch";
static char board_filter[] = "\
Chessboard files\0\
*.bd\0\
All files (*.*)\0\
*.*\0\
\0\
\0\
";
static char bd_ext[] = "bd";

static int board_x_offset;
static int board_y_offset;

static int top_margin;
static int font_height;
static int left_margin;
static int bottom_margin;
static COLORREF bk_color;

static LOGFONT lf;
static HFONT hfont;

static int initial_x_pos;
static int initial_y_pos;

static int chess_window_width;
static int chess_window_height;

#define WINDOW_EXTRA_WIDTH  16
#define WINDOW_EXTRA_HEIGHT 61

static int window_extra_width;
static int window_extra_height;

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
   #define IS_WIN32 TRUE
#else
   #define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && \
(LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

#define COLOR_BLACK     0x00000000
#define COLOR_WHITE     0x00ffffff
#define DARK_GRAY 0x00a8a8a8
#define RED       0x000000ff
#define ORANGE    0x000088ff
#define YELLOW    0x0000ffff
#define GREEN     0x0000ff00
#define BLUE      0x00ff0000

#define CHARACTER_WIDTH   8
#define CHARACTER_HEIGHT 13

// Global Variables:

static HINSTANCE hInst;      // current instance
static HWND hWndToolBar;
static char szAppName[100];  // Name of the app
static char szTitle[100];    // The title bar text

static int bHaveListFile;
static int num_files_in_list;
static int curr_chess_file;

static CHESS_FILE_LIST chess_file_list;

static int bHaveGame;
static struct game curr_game;

static TBBUTTON tbButtons[] = {
    { 0, IDM_NEXT_GAME,                TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
    { 1, IDM_PREV_GAME,                TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
    { 2, IDM_SAVE,                     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
};

static int special_move_info;
static boolean bPlayingVsMakeAMove;
static int move_number;

// Forward declarations of functions included in this code module:

BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
char *trim_name(char *name);

int is_list_file(LPSTR file_name);
int read_list_file(LPSTR file_name,CHESS_FILE_LIST *cfl_ptr,int *num_files);

LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Promotion(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MoveNumber(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
void do_lbuttondown(HWND hWnd,int file,int rank);

//
//  FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
//
//  PURPOSE: Entry point for the application.
//
//  COMMENTS:
//
// This function initializes the application and processes the
// message loop.
//
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
  int n;
  MSG msg;
  char *cpt;
  time_t now;
  int cmd_len;

  bBig = TRUE;

  width_in_pixels = WIDTH_IN_PIXELS;
  height_in_pixels = HEIGHT_IN_PIXELS;

  highlight_rank = -1;
  highlight_file = -1;

  cpt = getenv("DEBUG_CHESSIST");

  if (cpt != NULL) {
    debug_level = atoi(cpt);
    debug_fptr = fopen("chessist.dbg","w");
  }
  else {
    debug_level = 0;
    debug_fptr = NULL;
  }

  //if (getenv("BINARY_FORMAT"))
    bBinaryFormat = true;
  //else
  //  bBinaryFormat = false;

  if (getenv("AUTO_SAVE"))
    bAutoSave = true;
  else
    bAutoSave = false;

  if (getenv("ASSUME_MOVE_IS_LEGAL"))
    bAssumeMoveIsLegal = true;
  else
    bAssumeMoveIsLegal = false;

  if ((cpt = getenv("TOP_MARGIN")) != NULL)
    top_margin = atoi(cpt);
  else
    top_margin = 16;

  font_height = top_margin;

  if ((cpt = getenv("LEFT_MARGIN")) != NULL)
    left_margin = atoi(cpt);
  else
    left_margin = 12;

  board_x_offset = left_margin;

  if ((cpt = getenv("BOTTOM_MARGIN")) != NULL)
    bottom_margin = atoi(cpt);
  else
    bottom_margin = 16;

  if ((cpt = getenv("BK_COLOR")) != NULL)
    sscanf(cpt,"%x",&bk_color);
  else
    bk_color = DARK_GRAY;

  if ((cpt = getenv("WINDOW_EXTRA_WIDTH")) != NULL)
    sscanf(cpt,"%d",&window_extra_width);
  else
    window_extra_width = WINDOW_EXTRA_WIDTH;

  if ((cpt = getenv("WINDOW_EXTRA_HEIGHT")) != NULL)
    sscanf(cpt,"%d",&window_extra_height);
  else
    window_extra_height = WINDOW_EXTRA_HEIGHT;

  board_y_offset = TOOLBAR_HEIGHT; // + font_height * 3;

  // Initialize global strings
  lstrcpy (szAppName, appname);

  // read seed if it's there
  if (!strncmp(lpCmdLine,"seed",4)) {
    sscanf(&lpCmdLine[4],"%d",&seed);

    // save name of chess game, if one is specified after the seed

    cmd_len = strlen(lpCmdLine);

    for (n = 0; n < cmd_len; n++) {
      if (lpCmdLine[n] == ' ') {
        if (n < cmd_len - 1)
          lstrcpy(szFile,&lpCmdLine[n+1]);
        else
          szFile[0] = 0;

        break;
      }
    }

    if (n == cmd_len)
      szFile[0] = 0;
  }
  else {
    time(&now);
    seed = (int)now;

    // save name of chess game
    lstrcpy(szFile,lpCmdLine);
  }

  if (debug_fptr)
    fprintf(debug_fptr,"WinMain: seed = %d\n",seed);

  srand(seed);

  if (szFile[0])
    wsprintf(szTitle,"%s - %s",szAppName,
      trim_name(szFile));
  else
    lstrcpy(szTitle,szAppName);

  if (!hPrevInstance) {
     // Perform instance initialization:
     if (!InitApplication(hInstance)) {
        return (FALSE);
     }
  }

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
     return (FALSE);
  }

  // Main message loop:
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  if (debug_fptr)
    fclose(debug_fptr);

  return (msg.wParam);
}

//
//  FUNCTION: InitApplication(HANDLE)
//
//  PURPOSE: Initializes window data and registers window class
//
//  COMMENTS:
//
//       In this function, we initialize a window class by filling out a data
//       structure of type WNDCLASS and calling RegisterClass.
//
BOOL InitApplication(HINSTANCE hInstance)
{
  WNDCLASSEX wcex;

  lf.lfHeight         = CHARACTER_HEIGHT;
  lf.lfWidth          = CHARACTER_WIDTH;
  lf.lfEscapement     =   0;
  lf.lfOrientation    =   0;
  lf.lfWeight         = 400;
  lf.lfItalic         =   0;
  lf.lfUnderline      =   0;
  lf.lfStrikeOut      =   0;
  lf.lfCharSet        =   0;
  lf.lfOutPrecision   =   1;
  lf.lfClipPrecision  =   2;
  lf.lfQuality        =   1;
  lf.lfPitchAndFamily =  49;
  lstrcpy(lf.lfFaceName,"Courier");
  hfont = CreateFontIndirect(&lf);

  // Fill in window class structure with parameters that describe
  // the main window.
  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style         = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc   = (WNDPROC)WndProc;
  wcex.cbClsExtra    = 0;
  wcex.cbWndExtra    = 0;
  wcex.hInstance     = hInstance;
  wcex.hIcon         = LoadIcon(hInstance,(LPCTSTR)IDI_CHESSIST);
  wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = CreateSolidBrush(bk_color);
  wcex.lpszMenuName  = szAppName;
  wcex.lpszClassName = szAppName;
  wcex.hIconSm       = LoadIcon(hInstance,(LPCTSTR)IDI_SMALL);

  // Register the window class and return success/failure code.
  return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable
//        and create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  HWND hWnd;
  char *cpt;
  int debug_x_offset;
  int debug_y_offset;

  cpt = getenv("DEBUG_X_OFFSET");

  if (cpt != NULL)
    debug_x_offset = atoi(cpt);
  else
    debug_x_offset = 0;

  cpt = getenv("DEBUG_Y_OFFSET");

  if (cpt != NULL)
    debug_y_offset = atoi(cpt);
  else
    debug_y_offset = 0;

  hInst = hInstance; // Store instance handle in our global variable

  chess_window_width = board_x_offset + BOARD_WIDTH + window_extra_width;
  chess_window_height = board_y_offset + BOARD_HEIGHT + window_extra_height +
    bottom_margin;

  SystemParametersInfo(SPI_GETWORKAREA,0,&spi_rect,0);

  initial_x_pos = debug_x_offset + ((spi_rect.right - spi_rect.left) - chess_window_width) / 2;
  initial_y_pos = debug_y_offset + ((spi_rect.bottom - spi_rect.top) - chess_window_height) / 2;

  hWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPEDWINDOW,
     initial_x_pos, initial_y_pos,
     chess_window_width,chess_window_height,
     NULL, NULL, hInstance, NULL);

  if (!hWnd)
     return FALSE;

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return (TRUE);
}

int get_piece_offset(int piece,int rank,int file)
{
  int bBlack;
  int offset;
  int retval;

  if (debug_level == 2) {
    if (debug_fptr != NULL) {
      fprintf(debug_fptr,"dbg1 rank = %d, file = %d, piece = %2d, ",
        rank,file,piece);
    }
  }

  if (!piece) {
    /* this is a blank square; these are represented in the bitmap as well
       as pieces
    */
    if (!((rank + file) % 2))
      retval = 24;
    else
      retval = 25;
  }
  else {
    if (piece < 0) {
      bBlack = TRUE;
      piece *= -1;
    }
    else
      bBlack = FALSE;

    if ((piece >= 1) && (piece <= 6)) {
      piece--;
      offset = piece * 4;

      if (bBlack)
        offset += 2;

      if ((rank + file) % 2)
        offset++;

      retval = offset;
    }
    else
      retval = -1;
  }

  if (debug_level == 2) {
    if (debug_fptr != NULL)
      fprintf(debug_fptr,"retval = %2d\n",retval);
  }

  return retval;
}

void invalidate_rect(HWND hWnd,int rank,int file)
{
  RECT rect;

  rect.left = board_x_offset + file * width_in_pixels;
  rect.top = board_y_offset + rank * height_in_pixels;
  rect.right = rect.left + width_in_pixels;
  rect.bottom = rect.top + height_in_pixels;

  if (debug_fptr) {
    fprintf(debug_fptr,"invalidate_rect(): left = %d, top = %d, right = %d, bottom = %d\n",
      rect.left,rect.top,rect.right,rect.bottom);
  }

  InvalidateRect(hWnd,&rect,FALSE);
}

void invalidate_square(HWND hWnd,int square)
{
  int rank;
  int file;

  rank = RANK_OF(square);
  file = FILE_OF(square);

  if (!curr_game.orientation)
    rank = (NUM_RANKS - 1) - rank;
  else
    file = (NUM_FILES - 1) - file;

  if (debug_fptr) {
    fprintf(debug_fptr,"invalidate_square(): rank = %d, file = %d\n",
      rank,file);
  }

  invalidate_rect(hWnd,rank,file);
}

void invalidate_board(HWND hWnd)
{
  RECT rect;

  rect.left = board_x_offset;
  rect.top = board_y_offset;
  rect.right = rect.left + NUM_FILES * width_in_pixels;
  rect.bottom = rect.top + NUM_RANKS * height_in_pixels;

  InvalidateRect(hWnd,&rect,FALSE);
}

void invalidate_board_and_coords(HWND hWnd)
{
  RECT rect;

  rect.left = 0;
  rect.top = board_y_offset;
  rect.right = rect.left + board_x_offset + NUM_FILES * width_in_pixels;
  rect.bottom = rect.top + NUM_RANKS * height_in_pixels + bottom_margin;

  InvalidateRect(hWnd,&rect,TRUE);
}

static void redisplay_counts(HWND hWnd,HDC hdc)
{
  HDC local_hdc;
  RECT rect;
  char buf[80];

  return; // for now

  if (hdc != NULL)
    local_hdc = hdc;
  else {
    local_hdc = GetDC(hWnd);

    if (bk_color != COLOR_WHITE)
      SetBkColor(local_hdc,bk_color);

    SelectObject(local_hdc,hfont);
  }

  rect.left = 0;
  rect.top = TOOLBAR_HEIGHT;

  sprintf_move(&curr_game,buf,20,true);
  TextOut(local_hdc,rect.left,rect.top,buf,lstrlen(buf));
}

void do_paint(HWND hWnd)
{
  int m;
  int n;
  int piece;
  int piece_offset;
  int bigbmp_column;
  int bigbmp_row;
  HDC hdc;
  PAINTSTRUCT ps;
  RECT rect;
  int bSetBkColor;
  int bSelectedFont;
  char buf[80];

  hdc = BeginPaint(hWnd,&ps);

  if (debug_level == 2) {
    if (debug_fptr) {
      fprintf(debug_fptr,"do_paint():\n");
      fprintf(debug_fptr,"  highlight_rank = %d\n",highlight_rank);
      fprintf(debug_fptr,"  highlight_file = %d\n",highlight_file);
    }
  }

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      rect.left = board_x_offset + n * width_in_pixels;
      rect.top = board_y_offset + m * height_in_pixels;
      rect.right = rect.left + width_in_pixels;
      rect.bottom = rect.top + height_in_pixels;

      if (!RectVisible(hdc,&rect))
        continue;

      if (debug_level == 2) {
        if (debug_fptr) {
          fprintf(debug_fptr,"  rank = %d, file = %d\n",m,n);
          fprintf(debug_fptr,"  rect.left = %d, rect.top = %d\n",
            rect.left,rect.top);
          fprintf(debug_fptr,"  rect.right = %d, rect.bottom = %d\n",
            rect.right,rect.bottom);
        }
      }

      if (!curr_game.orientation)
        piece = get_piece2(curr_game.board,(NUM_RANKS - 1) - m,n);
      else
        piece = get_piece2(curr_game.board,m,(NUM_FILES - 1) - n);

      if (debug_level == 2) {
        if (debug_fptr)
          fprintf(debug_fptr,"  piece = %d\n",piece);
      }

      piece_offset = get_piece_offset(piece,m,n);

      if (piece_offset >= 0) {
        bigbmp_column = piece_offset;

        if ((m == highlight_rank) && (n == highlight_file))
          bigbmp_row = 2;
        else {
          if (curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_MATE)
            bigbmp_row = 1;
          else if (curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_STALEMATE)
            bigbmp_row = 3;
          else if (curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_CHECK)
            bigbmp_row = 5;
          else
            bigbmp_row = 0;
        }

        if (debug_fptr)
          fprintf(debug_fptr,"  bigbmp_column = %d, bigbmp_row = %d\n",bigbmp_column,bigbmp_row);

        BitBlt(hdc,rect.left,rect.top,
          width_in_pixels,height_in_pixels,
          hdc_compatible[bBig],
          bigbmp_column * width_in_pixels,
          bigbmp_row * height_in_pixels,
          SRCCOPY);
      }
    }
  }

  bSetBkColor = FALSE;
  bSelectedFont = FALSE;

  rect.left = 0;
  rect.top = TOOLBAR_HEIGHT;
  rect.right = chess_window_width;
  rect.bottom = TOOLBAR_HEIGHT + 16;

  if (RectVisible(hdc,&rect)) {
    if (debug_level == 2) {
      if (debug_fptr)
        fprintf(debug_fptr,"  printing the title\n");
    }

    if (!bSetBkColor && (bk_color != COLOR_WHITE)) {
      SetBkColor(hdc,bk_color);
      bSetBkColor = TRUE;
    }

    if (!bSelectedFont) {
      SelectObject(hdc,hfont);
      bSelectedFont = TRUE;
    }

    // TextOut(hdc,rect.left,rect.top,curr_game.title,lstrlen(curr_game.title));
  }

  rect.top = TOOLBAR_HEIGHT + 16;
  rect.right = chess_window_width;
  rect.bottom = TOOLBAR_HEIGHT + 64;

  if (RectVisible(hdc,&rect)) {
    if (!bSetBkColor && (bk_color != COLOR_WHITE)) {
      SetBkColor(hdc,bk_color);
      bSetBkColor = TRUE;
    }

    if (!bSelectedFont) {
      SelectObject(hdc,hfont);
      bSelectedFont = TRUE;
    }

    redisplay_counts(hWnd,hdc);
  }

  // display the ranks, if necessary
  rect.left = 2;
  rect.right = rect.left + CHARACTER_WIDTH;

  for (m = 0; m < NUM_RANKS; m++) {
    rect.top = board_y_offset + m * height_in_pixels +
      (bBig ? 19 : 6);
    rect.bottom = rect.top + CHARACTER_HEIGHT;

    if (RectVisible(hdc,&rect)) {
      if (!bSetBkColor && (bk_color != COLOR_WHITE)) {
        SetBkColor(hdc,bk_color);
        bSetBkColor = TRUE;
      }

      if (!bSelectedFont) {
        SelectObject(hdc,hfont);
        bSelectedFont = TRUE;
      }

      if (!curr_game.orientation)
        buf[0] = '1' + (NUM_FILES - 1) - m;
      else
        buf[0] = '1' + m;

      TextOut(hdc,rect.left,rect.top,buf,1);
    }
  }

  // display the files, if necessary
  rect.top = board_y_offset + NUM_RANKS * height_in_pixels + 2;
  rect.bottom = rect.top + CHARACTER_HEIGHT;

  for (m = 0; m < NUM_FILES; m++) {
    rect.left = board_x_offset + m * width_in_pixels +
      (bBig ? 21 : 8);
    rect.right = rect.left + CHARACTER_WIDTH;

    if (RectVisible(hdc,&rect)) {
      if (!bSetBkColor && (bk_color != COLOR_WHITE)) {
        SetBkColor(hdc,bk_color);
        bSetBkColor = TRUE;
      }

      if (!bSelectedFont) {
        SelectObject(hdc,hfont);
        bSelectedFont = TRUE;
      }

      if (!curr_game.orientation)
        buf[0] = 'a' + m;
      else
        buf[0] = 'a' + (NUM_FILES - 1) - m;

      TextOut(hdc,rect.left,rect.top,buf,1);
    }
  }

  EndPaint(hWnd,&ps);
}

static void do_move(HWND hWnd)
{
  int n;
  int invalid_squares[4];
  int num_invalid_squares;

  if (curr_game.curr_move == curr_game.num_moves)
    return;

  num_invalid_squares = 0;
  update_board(&curr_game,invalid_squares,&num_invalid_squares,false);
  update_piece_info(&curr_game);

  for (n = 0; n < num_invalid_squares; n++)
    invalidate_square(hWnd,invalid_squares[n]);

  if (debug_level == 2) {
    if (debug_fptr) {
      fprintf(debug_fptr,"do_move\n");
      fprint_bd2(curr_game.board,debug_fptr);
    }
  }

  curr_game.curr_move++;
  redisplay_counts(hWnd,NULL);
}

static int click_in_board(int x,int y)
{
  if ((x >= board_x_offset) && (x < board_x_offset + BOARD_WIDTH) &&
      (y >= board_y_offset) && (y < board_y_offset + BOARD_HEIGHT))
    return TRUE;

  return FALSE;
}

static int board_square(int x,int y,int *board_rank_pt,int *board_file_pt)
{
  if (!click_in_board(x,y))
    return FALSE;

  x -= board_x_offset;
  y -= board_y_offset;

  *board_rank_pt = y / height_in_pixels;
  *board_file_pt = x / width_in_pixels;

  return TRUE;
}

static void handle_char_input(HWND hWnd,WPARAM wParam)
{
  if ((wParam == 'e') || (wParam == 'E'))
    DestroyWindow(hWnd);
}

static void toggle_orientation(HWND hWnd)
{
  curr_game.orientation ^= 1;

  if (highlight_rank != -1) {
    highlight_rank = (NUM_RANKS - 1) - highlight_rank;
    highlight_file = (NUM_FILES - 1) - highlight_file;
  }

  invalidate_board_and_coords(hWnd);
}

static void toggle_board_size(HWND hWnd)
{
  RECT rect;

  if (debug_fptr) {
    fprintf(debug_fptr,"toggle_board_size()\n");
    fprintf(debug_fptr,"  bBig = %d\n",bBig);
    fprintf(debug_fptr,"  highlight_rank = %d\n",highlight_rank);
    fprintf(debug_fptr,"  highlight_file = %d\n",highlight_file);
  }

  if (!bBig) {
    width_in_pixels = WIDTH_IN_PIXELS;
    height_in_pixels = HEIGHT_IN_PIXELS;
    bBig = TRUE;
  }
  else {
    width_in_pixels = SHRUNK_WIDTH_IN_PIXELS;
    height_in_pixels = SHRUNK_HEIGHT_IN_PIXELS;
    bBig = FALSE;
  }

  chess_window_width = board_x_offset + BOARD_WIDTH + window_extra_width;
  chess_window_height = board_y_offset + BOARD_HEIGHT + window_extra_height +
    bottom_margin;

  rect.left = ((spi_rect.right - spi_rect.left) - chess_window_width) / 2;
  rect.top = ((spi_rect.bottom - spi_rect.top) - chess_window_height) / 2;

  if (debug_fptr) {
    fprintf(debug_fptr,"  width_in_pixels = %d\n",width_in_pixels);
    fprintf(debug_fptr,"  height_in_pixels = %d\n",height_in_pixels);
    fprintf(debug_fptr,"  bBig = %d\n",bBig);
  }

  MoveWindow(hWnd,rect.left,rect.top,
    chess_window_width,chess_window_height,TRUE);
}

void do_new(HWND hWnd,struct game *gamept)
{
  char *cpt;

  gamept->title[0] = 0;

  if ((cpt = getenv("DEBUG_ORIENTATION")) != NULL)
    gamept->orientation = atoi(cpt);
  else
    gamept->orientation = 0;

  gamept->num_moves = 0;
  gamept->curr_move = 0;

  set_initial_board(gamept);
  invalidate_board(hWnd);

  wsprintf(szTitle,"%s - new game",szAppName);
  SetWindowText(hWnd,szTitle);
}

void prev_move(HWND hWnd)
{
  if (!curr_game.curr_move)
    return;

  position_game(&curr_game,curr_game.curr_move - 1);
  invalidate_board(hWnd);
  redisplay_counts(hWnd,NULL);
}

void next_move(HWND hWnd)
{
  do_move(hWnd);

  if ((curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_MATE) ||
      (curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_STALEMATE) ||
      (curr_game.moves[curr_game.curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) ||
      ((curr_game.curr_move > 2) && (curr_game.moves[curr_game.curr_move-2].special_move_info & SPECIAL_MOVE_CHECK))) {
    invalidate_board(hWnd);
    redisplay_counts(hWnd,NULL);
  }
}

void start_of_game(HWND hWnd)
{
  position_game(&curr_game,0);
  invalidate_board(hWnd);
  redisplay_counts(hWnd,NULL);
}

void end_of_game(HWND hWnd)
{
  position_game(&curr_game,curr_game.num_moves);
  invalidate_board(hWnd);
  redisplay_counts(hWnd,NULL);
}

void do_read(HWND hWnd,LPSTR name,struct game *gamept,bool bBinaryFormat)
{
  int retval;
  char buf[256];

  if (debug_fptr)
    fprintf(debug_fptr,"%s\n","do_read(): top of function");

  if (!bBinaryFormat)
    retval = read_game(name,gamept,err_msg);
  else
    retval = read_binary_game(name,gamept);

  if (!retval) {
    bHaveGame = TRUE;

    wsprintf(szTitle,"%s - %s",szAppName,
      trim_name(name));
    SetWindowText(hWnd,szTitle);
    highlight_rank = -1;
    highlight_file = -1;
    InvalidateRect(hWnd,NULL,TRUE);
  }
  else {
    if (!bBinaryFormat) {
      wsprintf(buf,read_game_failure,
        name,retval,gamept->curr_move);
    }
    else {
      wsprintf(buf,read_binary_game_failure,
        name,retval,gamept->curr_move);
    }

    MessageBox(hWnd,buf,NULL,MB_OK);
  }
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  MESSAGES:
//
// WM_COMMAND - process the application menu
// WM_PAINT - Paint the main window
// WM_DESTROY - post a quit message and return
//    WM_DISPLAYCHANGE - message sent to Plug & Play systems when the display
//                       changes
//    WM_RBUTTONDOWN - Right mouse click -- put up context menu here if
//                     appropriate
//    WM_NCRBUTTONUP - User has clicked the right button on the application's
//                     system menu
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int n;
  int wmId, wmEvent;
  int file;
  int rank;
  int retval;
  LPSTR name;
  int bHaveName;
  HDC hdc;
  RECT rect;
  char buf[80];

  switch (message) {
    case WM_CREATE:
      // load the chess piece bitmaps
      for (n = 0; n < 2; n++) {
        chess_piece_bitmap_handles[n] = LoadBitmap(
          hInst,
          chess_piece_bitmap_names[n]);

        if (chess_piece_bitmap_handles[n] != NULL) {
          hdc_compatible[n] = CreateCompatibleDC(GetDC(hWnd));
          SelectObject(hdc_compatible[n],chess_piece_bitmap_handles[n]);
        }
      }

      // initialize the structure used for opening a file
      OpenFileName.lStructSize       = sizeof(OPENFILENAME);
      OpenFileName.hwndOwner         = hWnd;
      OpenFileName.hInstance         = hInst;
      OpenFileName.lpstrFilter       = chess_filter;
      OpenFileName.lpstrCustomFilter = NULL;
      OpenFileName.nMaxCustFilter    = 0;
      OpenFileName.nFilterIndex      = 1;
      OpenFileName.lpstrFile         = szFile;
      OpenFileName.nMaxFile          = sizeof(szFile);
      OpenFileName.lpstrFileTitle    = NULL;
      OpenFileName.nMaxFileTitle     = 0;
      OpenFileName.lpstrInitialDir   = NULL;
      OpenFileName.lpstrTitle        = "Open a chess file";
      OpenFileName.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
        OFN_EXTENSIONDIFFERENT;
      OpenFileName.nFileOffset       = 0;
      OpenFileName.nFileExtension    = 0;
      OpenFileName.lpstrDefExt       = ch_ext;
      OpenFileName.lCustData         = 0;
      OpenFileName.lpfnHook          = NULL;
      OpenFileName.lpTemplateName    = NULL;

      ReadBoardName.lStructSize       = sizeof(OPENFILENAME);
      ReadBoardName.hwndOwner         = hWnd;
      ReadBoardName.hInstance         = hInst;
      ReadBoardName.lpstrFilter       = board_filter;
      ReadBoardName.lpstrCustomFilter = NULL;
      ReadBoardName.nMaxCustFilter    = 0;
      ReadBoardName.nFilterIndex      = 1;
      ReadBoardName.lpstrFile         = szReadBoardName;
      ReadBoardName.nMaxFile          = sizeof(szReadBoardName);
      ReadBoardName.lpstrFileTitle    = NULL;
      ReadBoardName.nMaxFileTitle     = 0;
      ReadBoardName.lpstrInitialDir   = NULL;
      ReadBoardName.lpstrTitle        = "Open a chessboard file";
      ReadBoardName.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
        OFN_EXTENSIONDIFFERENT;
      ReadBoardName.nFileOffset       = 0;
      ReadBoardName.nFileExtension    = 0;
      ReadBoardName.lpstrDefExt       = bd_ext;
      ReadBoardName.lCustData         = 0;
      ReadBoardName.lpfnHook          = NULL;
      ReadBoardName.lpTemplateName    = NULL;

      WriteFileName.lStructSize = sizeof(OPENFILENAME);
      WriteFileName.hwndOwner = hWnd;
      WriteFileName.hInstance = hInst;
      WriteFileName.lpstrFilter = chess_filter;
      WriteFileName.lpstrCustomFilter = NULL;
      WriteFileName.nMaxCustFilter = 0;
      WriteFileName.nFilterIndex = 1;
      WriteFileName.lpstrFile = szWriteFileName;
      WriteFileName.nMaxFile = sizeof szWriteFileName;
      WriteFileName.lpstrFileTitle = NULL;
      WriteFileName.nMaxFileTitle = 0;
      WriteFileName.lpstrInitialDir = NULL;
      WriteFileName.lpstrTitle = NULL;
      WriteFileName.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
        OFN_EXTENSIONDIFFERENT;
      WriteFileName.nFileOffset = 0;
      WriteFileName.nFileExtension = 0;
      WriteFileName.lpstrDefExt = ch_ext;
      WriteFileName.lCustData = 0;
      WriteFileName.lpfnHook = NULL;
      WriteFileName.lpTemplateName = NULL;

      // create toolbar control
      hWndToolBar = CreateToolbarEx(
          hWnd,                   // parent
          WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS |
              CCS_ADJUSTABLE,     // style
          ID_TOOLBAR,             // toolbar id
          11,                      // number of bitmaps
          hInst,                  // mod instance
          IDB_TOOLBAR,            // resource id for the bitmap
          (LPCTBBUTTON)&tbButtons,// address of buttons
          3,                     // number of buttons
          16,16,                  // width & height of the buttons
          16,16,                  // width & height of the bitmaps
          sizeof(TBBUTTON));      // structure size

      // read the game passed on the command line, if there is one
      if (szFile[0]) {
        bHaveName = FALSE;
        retval = is_list_file(szFile);

        if (retval) {
          if (!read_list_file(szFile,
            &chess_file_list,&num_files_in_list)) {
            name = chess_file_list[curr_chess_file];
            bHaveName = TRUE;
          }
        }
        else {
          name = szFile;
          bHaveName = TRUE;
        }

        if (bHaveName)
          do_read(hWnd,name,&curr_game,bBinaryFormat);
        else
          do_new(hWnd,&curr_game);
      }
      else {
        if (debug_fptr)
          fprintf(debug_fptr,"%s\n","calling do_new() instead of do_read(), since szFile[0] is NULL");

        do_new(hWnd,&curr_game);
      }

      highlight_rank = -1;
      highlight_file = -1;
      InvalidateRect(hWnd,NULL,TRUE);

      break;

    case WM_SIZE:
      // Tell the toolbar to resize itself to fill the top of the window.
      SendMessage(hWndToolBar, TB_AUTOSIZE, 0L, 0L);
      break;

    case WM_CHAR:
      handle_char_input(hWnd,wParam);

      break;

    case WM_KEYDOWN:
      switch (wParam) {
        case VK_F2:
          toggle_orientation(hWnd);

          break;

        case VK_F3:
          toggle_board_size(hWnd);

          break;

        case VK_F6:
        case VK_F7:
          if (bHaveListFile) {
            if (bAutoSave) {
              // toggle the orientation, and save
              curr_game.orientation ^= 1;
              write_binary_game(chess_file_list[curr_chess_file],&curr_game);
            }

            if (wParam == VK_F6) {
              curr_chess_file++;

              if (curr_chess_file == num_files_in_list)
                curr_chess_file = 0;
            }
            else {
              curr_chess_file--;

              if (curr_chess_file < 0)
                curr_chess_file = num_files_in_list - 1;
            }

            do_read(hWnd,chess_file_list[curr_chess_file],&curr_game,bBinaryFormat);
          }

          break;

        case VK_HOME:
          start_of_game(hWnd);

          break;

        case VK_END:
          end_of_game(hWnd);

          break;

        case VK_LEFT:
          prev_move(hWnd);

          break;

        case VK_RIGHT:
          next_move(hWnd);

          break;

        case VK_F8:
          // must not be in the middle of replaying the moves from the game
          if (curr_game.curr_move == curr_game.num_moves) {
            if (make_a_move(&curr_game))
              do_move(hWnd);
          }

          break;
      }

      break;

    case WM_COMMAND:
      wmId    = LOWORD(wParam); // Remember, these are...
      wmEvent = HIWORD(wParam); // ...different for Win32!

      //Parse the menu selections:
      switch (wmId) {
        case IDM_NEW:
          do_new(hWnd,&curr_game);

          break;

        case IDM_OPEN:
	  // Call the common dialog function.
          bHaveGame = FALSE;

          if (GetOpenFileName(&OpenFileName)) {
            name = OpenFileName.lpstrFile;
            do_read(hWnd,name,&curr_game,false);
          }

          break;

        case IDM_OPEN_BINARY_GAME:
	  // Call the common dialog function.
          bHaveGame = FALSE;

          if (GetOpenFileName(&OpenFileName)) {
            name = OpenFileName.lpstrFile;
            do_read(hWnd,name,&curr_game,true);
          }

          break;

        case IDM_READ_BOARD:
	  // Call the common dialog function.

          if (GetOpenFileName(&ReadBoardName)) {
            name = ReadBoardName.lpstrFile;
            retval = populate_board_from_board_file(curr_game.board,name);
            invalidate_board(hWnd);
          }

          break;

        case IDM_ABOUT:
           DialogBox(hInst,"AboutBox",hWnd,(DLGPROC)About);

           break;

        case IDM_EXIT:
          DestroyWindow (hWnd);
          break;

        case IDM_SET_LAST_MOVE:
          curr_game.num_moves = curr_game.curr_move;
          curr_game.moves[curr_game.curr_move].special_move_info = 0;

          break;

        case IDM_PRINT_BOARD:
          if (debug_level == 2) {
            if (debug_fptr)
              fprint_bd2(curr_game.board,debug_fptr);
          }

          break;

        case IDM_PRINT_GAME:
          if (debug_level == 2) {
            if (debug_fptr)
              fprint_game2(&curr_game,debug_fptr);
          }

          break;

        case IDM_PRINT_MOVES:
          if (debug_level == 2) {
            if (debug_fptr)
              fprint_moves2(&curr_game,debug_fptr);
          }

          break;

        case IDM_PRINT_PIECE_INFO:
          if (debug_fptr)
            fprint_piece_info(&curr_game,debug_fptr);

          break;

        case IDM_MAKE_A_MOVE:
          // must not be in the middle of replaying the moves from the game
          if (curr_game.curr_move == curr_game.num_moves) {
            if (make_a_move(&curr_game))
              do_move(hWnd);
          }

          break;

        case IDM_PLAY_VS_MAKE_A_MOVE:
          if (!bPlayingVsMakeAMove)
            bPlayingVsMakeAMove = true;
          else
            bPlayingVsMakeAMove = false;

          break;

        case IDM_GO_TO_MOVE_NUMBER:
           if (DialogBox(hInst,"MoveNumberBox",hWnd,(DLGPROC)MoveNumber)) {
             if (move_number < 0)
               move_number = 0;
             else if (move_number > curr_game.num_moves)
               move_number = curr_game.num_moves;

             position_game(&curr_game,move_number);
             invalidate_board(hWnd);
           }

           break;


        case IDM_TOGGLE_ORIENTATION:
          toggle_orientation(hWnd);

          break;

        case IDM_TOGGLE_BOARD_SIZE:
          toggle_board_size(hWnd);

          break;

        case IDM_PREV_MOVE:
          prev_move(hWnd);

          break;

        case IDM_NEXT_MOVE:
          next_move(hWnd);

          break;

        case IDM_START_OF_GAME:
          start_of_game(hWnd);

          break;

        case IDM_END_OF_GAME:
          end_of_game(hWnd);

          break;

        case IDM_SAVE:
          write_binary_game(szFile,&curr_game);

          break;

        case IDM_SAVEAS:
	  // Call the common dialog function.
          bHaveName = FALSE;
          bHaveGame = FALSE;

          if (GetOpenFileName(&WriteFileName)) {
            bHaveName = TRUE;
            lstrcpy(szFile,szWriteFileName);
            write_binary_game(szFile,&curr_game);
          }

          break;

        case IDM_PREV_GAME:
        case IDM_NEXT_GAME:
          if (bHaveListFile) {
            if (wmId == IDM_NEXT_GAME) {
              curr_chess_file++;

              if (curr_chess_file == num_files_in_list)
                curr_chess_file = 0;
            }
            else {
              curr_chess_file--;

              if (curr_chess_file < 0)
                curr_chess_file = num_files_in_list - 1;
            }

            do_read(hWnd,chess_file_list[curr_chess_file],&curr_game,bBinaryFormat);
          }

          break;

        // Here are all the other possible menu options,
        // all of these are currently disabled:
        case IDM_PRINT:
        case IDM_PRINTSETUP:

        default:
          return (DefWindowProc(hWnd, message, wParam, lParam));
      }

      break;

    case WM_LBUTTONDOWN:
      if (curr_game.curr_move == curr_game.num_moves) {
        file = (LOWORD(lParam) - board_x_offset) / width_in_pixels;
        rank = (HIWORD(lParam) - board_y_offset) / height_in_pixels;
        do_lbuttondown(hWnd,file,rank);
      }

      break;

    case WM_PAINT:
      do_paint(hWnd);

      break;

    case WM_DESTROY:
      for (n = 0; n < 2; n++) {
        if (chess_piece_bitmap_handles[n] != NULL)
          DeleteDC(hdc_compatible[n]);
      }

      PostQuitMessage(0);

      break;

    default:
      return (DefWindowProc(hWnd, message, wParam, lParam));
  }

  return (0);
}

char *trim_name(char *name)
{
  int n;

  for (n = strlen(name) - 1; (n >= 0); n--)
    if (name[n] == '\\')
      break;

  n++;

  return &name[n];
}

int is_list_file(LPSTR file_name)
{
  int n;
  int len;

  len = lstrlen(file_name);

  for (n = len - 1; (n >= 0); n--) {
    if (file_name[n] == '.')
      break;
  }

  if (n < 0)
    return FALSE;

  n++;

  if (!strcmp(&file_name[n],"lst"))
    return TRUE;

  return FALSE;
}

int read_list_file(LPSTR file_name,CHESS_FILE_LIST *cfl_ptr,int *num_files)
{
  int m;
  int n;
  FILE *fptr;
  int chara;
  char *cpt;

  if (chess_file_list != NULL) {
    free(chess_file_list);
    chess_file_list = NULL;
  }

  if ((fptr = fopen(file_name,"r")) == NULL)
    return 1;

  num_files_in_list = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == 0x0a)
      num_files_in_list++;
  }

  fseek(fptr,0,SEEK_SET);

  chess_file_list = (CHESS_FILE_LIST)malloc(MAX_FILE_NAME_LEN *
    num_files_in_list);

  if (chess_file_list == NULL)
    return 2;

  n = 0;
  m = 0;
  cpt = (char *)chess_file_list[n];

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == 0x0a) {
      cpt[m] = 0;
      m = 0;
      n++;
      cpt = (char *)chess_file_list[n];
    }
    else
      cpt[m++] = chara;
  }

  fclose(fptr);

  bHaveListFile = TRUE;
  curr_chess_file = 0;

  return 0;
}

//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
//
//  MESSAGES:
//
// WM_INITDIALOG - initialize dialog box
// WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
        case WM_INITDIALOG:
         ShowWindow (hDlg, SW_HIDE);

         CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

         ShowWindow (hDlg, SW_SHOW);

         return (TRUE);

      case WM_COMMAND:
         if (LOWORD(wParam) == IDOK) {
            EndDialog(hDlg, TRUE);
            return (TRUE);
         }

         break;
   }

    return FALSE;
}

//
//  FUNCTION: Promotion(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "Promotion" dialog box
//
//  MESSAGES:
//
// WM_INITDIALOG - initialize dialog box
// WM_COMMAND    - Input received
//
//
LRESULT CALLBACK Promotion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
      case WM_INITDIALOG:
         ShowWindow (hDlg, SW_HIDE);

         CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

         ShowWindow (hDlg, SW_SHOW);

         return (TRUE);

      case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDC_QUEEN:
                special_move_info = SPECIAL_MOVE_PROMOTION_QUEEN;
                break;
            case IDC_ROOK:
                special_move_info = SPECIAL_MOVE_PROMOTION_ROOK;
                break;
            case IDC_BISHOP:
                special_move_info = SPECIAL_MOVE_PROMOTION_BISHOP;
                break;
            case IDC_KNIGHT:
                special_move_info = SPECIAL_MOVE_PROMOTION_KNIGHT;
                break;
            case IDOK:
                EndDialog(hDlg, TRUE);
                return (TRUE);
            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return (FALSE);
         }

         break;
   }

    return FALSE;
}

#define MAX_MOVE_NUMBER_LEN 4
static char move_number_buf[MAX_MOVE_NUMBER_LEN+1];

//
//  FUNCTION: MoveNumber(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "MoveNumber" dialog box
//
//  MESSAGES:
//
// WM_INITDIALOG - initialize dialog box
// WM_COMMAND    - Input received
//
//
LRESULT CALLBACK MoveNumber(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
      case WM_INITDIALOG:
         ShowWindow (hDlg, SW_HIDE);

         CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));

         ShowWindow (hDlg, SW_SHOW);

         return (TRUE);

      case WM_COMMAND:
        switch (LOWORD(wParam)) {
            case IDOK:
                if (GetDlgItemText(hDlg,IDC_MOVE_NUMBER,move_number_buf,MAX_MOVE_NUMBER_LEN)) {
                    sscanf(move_number_buf,"%d",&move_number);
                }

                EndDialog(hDlg, TRUE);
                return (TRUE);
            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return (FALSE);
         }

         break;
   }


    return FALSE;
}

//
//   FUNCTION: CenterWindow(HWND, HWND)
//
//   PURPOSE: Centers one window over another.
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
//       This functionwill center one window over another ensuring that
//    the placement of the window is within the 'working area', meaning
//    that it is both within the display limits of the screen, and not
//    obscured by the tray or other framing elements of the desktop.
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
   RECT    rChild, rParent, rWorkArea;
   int     wChild, hChild, wParent, hParent;
   int     xNew, yNew;
   BOOL  bResult;

   // Get the Height and Width of the child window
   GetWindowRect (hwndChild, &rChild);
   wChild = rChild.right - rChild.left;
   hChild = rChild.bottom - rChild.top;

   // Get the Height and Width of the parent window
   GetWindowRect (hwndParent, &rParent);
   wParent = rParent.right - rParent.left;
   hParent = rParent.bottom - rParent.top;

   // Get the limits of the 'workarea'
   bResult = SystemParametersInfo(
      SPI_GETWORKAREA,  // system parameter to query or set
      sizeof(RECT),
      &rWorkArea,
      0);
   if (!bResult) {
      rWorkArea.left = rWorkArea.top = 0;
      rWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
      rWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
   }

   // Calculate new X position, then adjust for workarea
   xNew = rParent.left + ((wParent - wChild) /2);
   if (xNew < rWorkArea.left) {
      xNew = rWorkArea.left;
   } else if ((xNew+wChild) > rWorkArea.right) {
      xNew = rWorkArea.right - wChild;
   }

   // Calculate new Y position, then adjust for workarea
   yNew = rParent.top  + ((hParent - hChild) /2);
   if (yNew < rWorkArea.top) {
      yNew = rWorkArea.top;
   } else if ((yNew+hChild) > rWorkArea.bottom) {
      yNew = rWorkArea.bottom - hChild;
   }

   // Set it, and return
   return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void do_lbuttondown(HWND hWnd,int file,int rank)
{
  int n;
  int retval;
  bool bPromotion;
  int invalid_squares[4];
  int num_invalid_squares;
  bool bBlack;
  int dbg;

  if (curr_game.curr_move == dbg_move)
    dbg = 1;

  if (debug_fptr != NULL) {
    fprintf(debug_fptr,"do_lbuttondown: rank = %d, file = %d\n",rank,file);
  }

  if ((file >= 0) && (file < NUM_FILES) &&
      (rank >= 0) && (rank < NUM_RANKS))
    ;
  else
    return;

  if ((highlight_rank == rank) && (highlight_file == file)) {
    highlight_rank = -1;
    highlight_file = -1;

    invalidate_rect(hWnd,rank,file);
    return;
  }

  if (!curr_game.orientation) {
    if (highlight_rank == -1) {
      move_start_square = ((NUM_RANKS - 1) - rank) * NUM_FILES + file;
      move_start_square_piece = get_piece1(curr_game.board,move_start_square);

      if (!move_start_square_piece)
        return;
    }
    else {
      move_end_square = ((NUM_RANKS - 1) - rank) * NUM_FILES + file;
      move_end_square_piece = get_piece1(curr_game.board,move_end_square);
    }
  }
  else {
    if (highlight_rank == -1) {
      move_start_square = rank * NUM_FILES + (NUM_FILES - 1) - file;
      move_start_square_piece = get_piece1(curr_game.board,move_start_square);

      if (!move_start_square_piece)
        return;
    }
    else {
      move_end_square = rank * NUM_FILES + (NUM_FILES - 1) - file;
      move_end_square_piece = get_piece1(curr_game.board,move_end_square);
    }
  }

  if (highlight_rank == -1) {
    if (debug_fptr) {
      fprintf(debug_fptr,"do_lbuttondown:   start_square_piece = %d, curr_move = %d\n",
        move_start_square_piece,curr_game.curr_move);
    }

    if ( ((move_start_square_piece > 0) && !((curr_game.curr_move) % 2)) ||
         ((move_start_square_piece < 0) &&  ((curr_game.curr_move) % 2)) ) {
      if (debug_fptr) {
        fprintf(debug_fptr,"do_lbuttondown:   setting highlight: rank = %d, file = %d\n",rank,file);
      }

      highlight_file = file;
      highlight_rank = rank;

      invalidate_rect(hWnd,rank,file);
    }
    else {
      if (debug_fptr) {
        fprintf(debug_fptr,"do_lbuttondown:   not setting highlight: rank = %d, file = %d\n",rank,file);
      }
    }

    return;
  }

  // exit early if the square to be moved to contains a piece of the same color as the piece to be moved
  if ((move_start_square_piece * move_end_square_piece) > 0)
    return;

  if (debug_fptr) {
    fprintf(debug_fptr,"do_lbuttondown:   attempting move: rank = %d,file = %d\n",rank,file);
  }

  bPromotion = false;

  if ((move_start_square_piece == PAWN_ID) ||
      (move_start_square_piece == PAWN_ID * -1)) {
    retval = do_pawn_move2(&curr_game);

    if (!retval) {
      // check if this was a pawn promotion

      if (!((curr_game.curr_move) % 2)) {
        // White

        if (!curr_game.orientation) {
          if (!rank)
            bPromotion = true;
        }
        else {
          if (rank == NUM_RANKS - 1)
            bPromotion = true;
        }
      }
      else {
        // Black

        if (!curr_game.orientation) {
          if (rank == NUM_RANKS - 1)
            bPromotion = true;
        }
        else {
          if (!rank)
            bPromotion = true;
        }
      }

      if (bPromotion) {
        special_move_info = 0;

        if (!DialogBox(hInst,"PromotionBox",hWnd,(DLGPROC)Promotion))
          retval = 1;
        else {
          curr_game.moves[curr_game.curr_move].special_move_info = special_move_info;
        }
      }
    }
  }
  else
    retval = do_piece_move2(&curr_game);

  if (!retval) {
    num_invalid_squares = 0;
    update_board(&curr_game,invalid_squares,&num_invalid_squares,false);
    update_piece_info(&curr_game);

    for (n = 0; n < num_invalid_squares; n++)
      invalidate_square(hWnd,invalid_squares[n]);

    highlight_rank = -1;
    highlight_file = -1;

    curr_game.curr_move++;
    curr_game.moves[curr_game.curr_move].special_move_info = 0;
    curr_game.num_moves = curr_game.curr_move;

    bBlack = curr_game.curr_move & 0x1;

    legal_moves_count = 0;
    get_legal_moves(&curr_game,&legal_moves[0],&legal_moves_count);

    if (player_is_in_check(bBlack,curr_game.board,curr_game.curr_move)) {
      curr_game.moves[curr_game.curr_move-1].special_move_info |= SPECIAL_MOVE_CHECK;

      // now determine if this is a checkmate

      if (!legal_moves_count) {
        curr_game.moves[curr_game.curr_move-1].special_move_info |= SPECIAL_MOVE_MATE;
        invalidate_board(hWnd);
      }
    }
    else {
      if (!legal_moves_count) {
        curr_game.moves[curr_game.curr_move-1].special_move_info |= SPECIAL_MOVE_STALEMATE;
        invalidate_board(hWnd);
      }
    }

    if (bPlayingVsMakeAMove) {
      if (make_a_move(&curr_game))
        do_move(hWnd);
    }
  }
}
