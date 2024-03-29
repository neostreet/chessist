#include <stdio.h>
#include "chess.h"
#include "chess.glb"
#include "chess.fun"
#include "chess.mac"
#include "bitfuns.h"

static unsigned char initial_board[] = {
  (unsigned char)0x23, (unsigned char)0x45, (unsigned char)0x64, (unsigned char)0x32,
  (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff,
  (unsigned char)0xed, (unsigned char)0xcb, (unsigned char)0xac, (unsigned char)0xde
};

extern char piece_ids[]; /* "RNBQK" */
extern char fmt_str[];

static int format_square(int square)
{
  int bBlack;
  int return_char;

  if (!square)
    return (int)'.';

  if (square < 0) {
    bBlack = TRUE;
    square *= -1;
  }
  else
    bBlack = FALSE;

  if (square == 1)
    return_char = 'P';
  else
    return_char = piece_ids[square - 2];

  if (!bBlack)
    return_char += ('a' - 'A');

  return return_char;
}

void print_bd(struct game *gamept)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      printf("%c ",format_square(square));
    }

    putchar(0x0a);
  }
}

void fprint_game(struct game *gamept,char *filename)
{
  FILE *fptr;
  char buf[20];

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  fprintf(fptr,fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20,true);
    fprintf(fptr,fmt_str,buf);

    update_board(gamept,NULL,NULL);
  }

  fclose(fptr);
}

void fprint_game2(struct game *gamept,FILE *fptr)
{
  char buf[20];

  fprintf(fptr,fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20,true);
    fprintf(fptr,fmt_str,buf);

    update_board(gamept,NULL,NULL);
  }
}

void fprint_bd(struct game *gamept,char *filename)
{
  int m;
  int n;
  FILE *fptr;
  int square;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }

  fclose(fptr);
}

void fprint_bd2(struct game *gamept,FILE *fptr)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }
}

void fprint_moves(struct game *gamept,char *filename)
{
  int n;
  FILE *fptr;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  for (n = 0; n < gamept->num_moves; n++) {
    fprintf(fptr,"%d %d\n",gamept->moves[n].from,gamept->moves[n].to);
  }

  fclose(fptr);
}

void fprint_moves2(struct game *gamept,FILE *fptr)
{
  int n;

  for (n = 0; n < gamept->num_moves; n++) {
    fprintf(fptr,"%d %d\n",gamept->moves[n].from,gamept->moves[n].to);
  }
}

void set_initial_board(struct game *gamept)
{
  int n;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    gamept->board[n] = initial_board[n];
}

void position_game(struct game *gamept,int move)
{
  set_initial_board(gamept);

  for (gamept->curr_move = 0; gamept->curr_move < move; gamept->curr_move++) {
    update_board(gamept,NULL,NULL);
  }
}

static int update_board_calls;
static int dbg_update_board_call;
static int dbg;

void update_board(struct game *gamept,int *invalid_squares,int *num_invalid_squares)
{
  bool bKingsideCastle;
  bool bQueensideCastle;
  bool bBlack;
  int from_piece;
  int to_piece;

  update_board_calls++;

  if (dbg_update_board_call == update_board_calls)
    dbg = 0;

  bBlack = (gamept->curr_move % 2);

  from_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].from);
  to_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].to);

  if (from_piece * to_piece < 0)
    gamept->moves[gamept->curr_move].special_move_info |= SPECIAL_MOVE_CAPTURE;

  switch (gamept->moves[gamept->curr_move].special_move_info) {
    case SPECIAL_MOVE_PROMOTION_QUEEN:
      from_piece = (bBlack ? QUEEN_ID * -1 : QUEEN_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_ROOK:
      from_piece = (bBlack ? ROOK_ID * -1 : ROOK_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_BISHOP:
      from_piece = (bBlack ? BISHOP_ID * -1 : BISHOP_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_KNIGHT:
      from_piece = (bBlack ? KNIGHT_ID * -1 : KNIGHT_ID);

      break;
  }

  if (invalid_squares) {
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].from;
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].to;
  }

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].to,from_piece);

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].from,0);  /* vacate previous square */

  bKingsideCastle = (gamept->moves[gamept->curr_move].special_move_info == SPECIAL_MOVE_KINGSIDE_CASTLE);
  bQueensideCastle = (gamept->moves[gamept->curr_move].special_move_info == SPECIAL_MOVE_QUEENSIDE_CASTLE);

  if (bKingsideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,5,
        get_piece1(gamept->board,7));
      set_piece1(gamept->board,7,0);

      if (invalid_squares) {
        invalid_squares[(*num_invalid_squares)++] = 5;
        invalid_squares[(*num_invalid_squares)++] = 7;
      }
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,61,
        get_piece1(gamept->board,63));
      set_piece1(gamept->board,63,0);

      if (invalid_squares) {
        invalid_squares[(*num_invalid_squares)++] = 61;
        invalid_squares[(*num_invalid_squares)++] = 63;
      }
    }
  }
  else if (bQueensideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,3,
        get_piece1(gamept->board,0));
      set_piece1(gamept->board,0,0);

      if (invalid_squares) {
        invalid_squares[(*num_invalid_squares)++] = 3;
        invalid_squares[(*num_invalid_squares)++] = 0;
      }
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,59,
        get_piece1(gamept->board,56));
      set_piece1(gamept->board,56,0);

      if (invalid_squares) {
        invalid_squares[(*num_invalid_squares)++] = 59;
        invalid_squares[(*num_invalid_squares)++] = 56;
      }
    }
  }
}

int get_piece1(unsigned char *board,int board_offset)
{
  unsigned int bit_offset;
  unsigned short piece;
  int piece_int;

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;

  piece = get_bits(BITS_PER_BOARD_SQUARE,board,bit_offset);
  piece_int = piece;

  if (piece & 0x8)
    piece_int |= 0xfffffff0;

  return piece_int;
}

int get_piece2(unsigned char *board,int rank,int file)
{
  int board_offset;

  board_offset = rank * NUM_FILES + file;
  return get_piece1(board,board_offset);
}

static int set_piece_calls;
static int dbg_set_piece_call;

void set_piece1(unsigned char *board,int board_offset,int piece)
{
  unsigned int bit_offset;

  set_piece_calls++;

  if (dbg_set_piece_call == set_piece_calls)
    dbg = 0;

  if (debug_level == 2) {
    if (debug_fptr != NULL)
      fprintf(debug_fptr,"set_piece: board_offset = %d, piece %d\n",board_offset,piece);
  }

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;
  set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,piece);
}

void set_piece2(unsigned char *board,int rank,int file,int piece)
{
  int board_offset;

  board_offset = rank * NUM_FILES + file;
  set_piece1(board,board_offset,piece);
}
