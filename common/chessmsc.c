#include <stdio.h>

#include "chess.h"
#include "chess.glb"
#include "chess.fun"
#include "chess.mac"

int format_square(int square)
{
  bool bBlack;
  int return_char;

  if (!square)
    return (int)'.';

  if (square < 0) {
    bBlack = true;
    square *= -1;
  }
  else
    bBlack = false;

  if (square == 1)
    return_char = 'P';
  else
    return_char = piece_ids[square - 2];

  if (!bBlack)
    return_char += ('a' - 'A');

  return return_char;
}

void print_bd0(unsigned char *board,int orientation)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      if (!orientation)
        square = get_piece2(board,(NUM_RANKS - 1) - m,n);
      else
        square = get_piece2(board,m,(NUM_FILES - 1) - n);

      printf("%c",format_square(square));

      if (n < (NUM_FILES - 1))
        putchar(' ');
    }

    putchar(0x0a);
  }
}

void print_bd(struct game *gamept)
{
  int m;
  int n;
  int square;
  int dbg;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      if (!gamept->orientation)
        square = get_piece2(gamept->board,7 - m,n);
      else
        square = get_piece2(gamept->board,m,7 - n);

      printf("%c",format_square(square));

      if (n < 7)
        putchar(' ');
    }

    putchar(0x0a);
  }
}

void fprint_game_bin(struct game *gamept,char *filename)
{
  FILE *fptr;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  fprintf(fptr,fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    fprintf_move(fptr,gamept);

    update_board(gamept,NULL,NULL,false);
  }

  fclose(fptr);
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
    fprintf(fptr,"%s",buf);

    if (gamept->curr_move < gamept->num_moves)
      update_board(gamept,NULL,NULL,false);
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

    update_board(gamept,NULL,NULL,false);
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

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      square = get_piece2(gamept->board,7 - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }

  fclose(fptr);
}

void fprint_bd2(unsigned char *board,FILE *fptr)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(board,(NUM_RANKS - 1) - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }
}

void fprint_bd3(unsigned char *board,int orientation,FILE *fptr)
{
  int m;
  int n;
  int square;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      if (!orientation)
        square = get_piece2(board,7 - m,n);
      else
        square = get_piece2(board,m,7 - n);

      fprintf(fptr,"%c",format_square(square));

      if (n < 7)
        fputc(' ',fptr);
    }

    fputc(0x0a,fptr);
  }
}

void print_moves(struct move *moves,int num_moves,bool bHex,bool bMoveNumbers)
{
  int m;
  int n;
  int and_val;
  int hit;
  int dbg_move;
  int dbg;

  dbg_move = -1;

  for (n = 0; n < num_moves; n++) {
    if (n == dbg_move)
      dbg = 1;

    if (bHex) {
      if (bMoveNumbers) {
        printf("%3d from: %c%c to: %c%c %04x\n",
          n,
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to),
          moves[n].special_move_info);
      }
      else {
        printf("from: %c%c to: %c%c %04x\n",
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to),
          moves[n].special_move_info);
       }
    }
    else {
      if (bMoveNumbers) {
        printf("%3d from: %c%c to: %c%c",
          n,
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to));
      }
      else {
        printf("from: %c%c to: %c%c",
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to));
      }

      and_val = 0x1;
      hit = 0;

      for (m = 0; m < num_special_moves; m++) {
        if (moves[n].special_move_info & and_val) {
          hit = 1;
          putchar(' ' );
          printf("%s",special_moves[m]);
        }

        and_val <<= 1;
      }

      if (!hit)
        printf(" SPECIAL_MOVE_NONE");

      putchar(0x0a);
    }
  }

  dbg = 1; // see if you get here
}

void fprint_moves(struct move *moves,int num_moves,char *filename)
{
  int m;
  int n;
  int and_val;
  int hit;
  FILE *fptr;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  for (n = 0; n < num_moves; n++) {
    fprintf(fptr,"%3d from: %c%c to: %c%c",
      n,
      'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
      'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to));

    and_val = 0x1;
    hit = 0;

    for (m = 0; m < num_special_moves; m++) {
      if (moves[n].special_move_info & and_val) {
        hit = 1;
        fputc(' ',fptr);
        fprintf(fptr,"%s",special_moves[m]);
      }

      and_val <<= 1;
    }

    if (!hit)
      fprintf(fptr," SPECIAL_MOVE_NONE");

    fputc(0x0a,fptr);
  }

  fclose(fptr);
}

void fprint_moves2(struct move *moves,int num_moves,FILE *fptr)
{
  int n;

  fprintf(fptr,"fprint_moves2:\n");

  for (n = 0; n < num_moves; n++) {
    fprintf(fptr,"%d %d %x\n",
      moves[n].from,
      moves[n].to,
      moves[n].special_move_info);
  }
}

void fprint_moves3(FILE *fptr,struct move *moves,int num_moves,bool bHex,bool bMoveNumbers)
{
  int m;
  int n;
  int and_val;
  int hit;
  int dbg_move;
  int dbg;

  dbg_move = -1;

  for (n = 0; n < num_moves; n++) {
    if (n == dbg_move)
      dbg = 1;

    if (bHex) {
      if (bMoveNumbers) {
        fprintf(fptr,"%3d from: %c%c to: %c%c %04x\n",
          n,
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to),
          moves[n].special_move_info);
      }
      else {
        fprintf(fptr,"from: %c%c to: %c%c %04x\n",
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to),
          moves[n].special_move_info);
       }
    }
    else {
      if (bMoveNumbers) {
        fprintf(fptr,"%3d from: %c%c to: %c%c",
          n,
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to));
      }
      else {
        fprintf(fptr,"from: %c%c to: %c%c",
          'a' + FILE_OF(moves[n].from),'1' + RANK_OF(moves[n].from),
          'a' + FILE_OF(moves[n].to),'1' + RANK_OF(moves[n].to));
      }

      and_val = 0x1;
      hit = 0;

      for (m = 0; m < num_special_moves; m++) {
        if (moves[n].special_move_info & and_val) {
          hit = 1;
          fputc(' ',fptr);
          fprintf(fptr,"%s",special_moves[m]);
        }

        and_val <<= 1;
      }

      if (!hit)
        fprintf(fptr," SPECIAL_MOVE_NONE");

      fputc(0x0a,fptr);
    }
  }

  dbg = 1; // see if you get here
}

void print_special_moves(struct game *gamept)
{
  int n;
  int and_val;
  int hit;

  and_val = 0x1;
  hit = 0;

  for (n = 0; n < num_special_moves; n++) {
    if (gamept->moves[gamept->curr_move].special_move_info & and_val) {
      if (hit)
        putchar(' ' );

      printf("%s",special_moves[n]);

      hit++;
    }

    and_val <<= 1;
  }

  if (hit)
    putchar(0x0a);
  else
    printf("SPECIAL_MOVE_NONE\n");
}

int match_board(unsigned char *board1,unsigned char *board2,bool bExactMatch)
{
  int m;
  int n;
  int square1;
  int square2;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      square1 = get_piece2(board1,7 - m,n);
      square2 = get_piece2(board2,7 - m,n);

      if (!bExactMatch) {
        if (square2) {
          if (square2 == EMPTY_ID) {
            if (square1)
              break;
          }
          else if (square1 != square2)
            break;
        }
      }
      else {
        if (square2 == EMPTY_ID) {
          if (square1)
            break;
        }
        else if (square1 != square2)
          break;
      }
    }

    if (n < 8)
      break;
  }

  if (m < 8)
    return 0;

  return 1;
}

bool multiple_queens(unsigned char *board)
{
  int n;
  int piece;
  int num_white_queens = 0;
  int num_black_queens = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == QUEEN_ID)
      num_white_queens++;
    else if (piece == QUEEN_ID * -1)
      num_black_queens++;
  }

  if ((num_white_queens > 1) || (num_black_queens > 1))
    return true;

  return false;
}

bool no_queens(unsigned char *board)
{
  int n;
  int piece;
  int num_white_queens = 0;
  int num_black_queens = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == QUEEN_ID)
      num_white_queens++;
    else if (piece == QUEEN_ID * -1)
      num_black_queens++;
  }

  if (!num_white_queens && !num_black_queens)
    return true;

  return false;
}

bool opposite_colored_bishops(unsigned char *board)
{
  int n;
  int piece;
  int rank;
  int file;
  int num_white_bishops = 0;
  bool bWhiteBishopOnWhiteSquare;
  int num_black_bishops = 0;
  bool bBlackBishopOnWhiteSquare;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);
    rank = RANK_OF(n);
    file = FILE_OF(n);

    if (piece == BISHOP_ID) {
      num_white_bishops++;
      bWhiteBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
    else if (piece == BISHOP_ID * -1) {
      num_black_bishops++;
      bBlackBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
  }

  if ((num_white_bishops == 1) && (num_black_bishops == 1) && (bWhiteBishopOnWhiteSquare != bBlackBishopOnWhiteSquare))
    return true;

  return false;
}

bool same_colored_bishops(unsigned char *board)
{
  int n;
  int piece;
  int rank;
  int file;
  int num_white_bishops = 0;
  bool bWhiteBishopOnWhiteSquare;
  int num_black_bishops = 0;
  bool bBlackBishopOnWhiteSquare;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);
    rank = RANK_OF(n);
    file = FILE_OF(n);

    if (piece == BISHOP_ID) {
      num_white_bishops++;
      bWhiteBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
    else if (piece == BISHOP_ID * -1) {
      num_black_bishops++;
      bBlackBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
  }

  if ((num_white_bishops == 1) && (num_black_bishops == 1) && (bWhiteBishopOnWhiteSquare == bBlackBishopOnWhiteSquare))
    return true;

  return false;
}

bool two_bishops(unsigned char *board)
{
  int n;
  int piece;
  int num_white_bishops = 0;
  int num_black_bishops = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == BISHOP_ID)
      num_white_bishops++;
    else if (piece == BISHOP_ID * -1)
      num_black_bishops++;
  }

  if (((num_white_bishops < 2) && (num_black_bishops == 2)) || ((num_white_bishops == 2) && (num_black_bishops < 2)))
    return true;

  return false;
}

bool opposite_side_castling(struct game *gamept)
{
  int n;
  bool bHaveKingsideCastle;
  bool bHaveQueensideCastle;

  bHaveKingsideCastle = false;
  bHaveQueensideCastle = false;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      bHaveKingsideCastle = true;

      if (bHaveQueensideCastle)
        return true;
    }
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE) {
      bHaveQueensideCastle = true;

      if (bHaveKingsideCastle)
        return true;
    }
  }

  return false;;
}

bool same_side_castling(struct game *gamept)
{
  int n;
  int kingside_castles;
  int queenside_castles;

  kingside_castles = 0;
  queenside_castles = 0;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      kingside_castles++;
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE)
      queenside_castles++;
  }

  return ((kingside_castles == 2) || (queenside_castles == 2));
}

bool less_than_2_castles(struct game *gamept)
{
  int n;
  int castles;

  castles = 0;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      castles++;
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE)
      castles++;
  }

  return (castles < 2);
}

bool white_pigs(unsigned char *board)
{
  int n;
  int piece;
  int num_white_pigs = 0;

  for (n = 48; n < 56; n++) {
    piece = get_piece1(board,n);

    if (piece == ROOK_ID) {
      num_white_pigs++;

      if (num_white_pigs == 2)
        return true;
    }
    else if ((num_white_pigs == 1) && (piece))
      return false;
  }

  return false;
}

bool black_pigs(unsigned char *board)
{
  int n;
  int piece;
  int num_black_pigs = 0;

  for (n = 8; n < 16; n++) {
    piece = get_piece1(board,n);

    if (piece == ROOK_ID * -1) {
      num_black_pigs++;

      if (num_black_pigs == 2)
        return true;
    }
    else if ((num_black_pigs == 1) && (piece))
      return false;
  }

  return false;
}

bool exchange_sac(struct game *gamept)
{
  int captured_piece;

  if (gamept->curr_move == gamept->num_moves - 1)
    return false;

  if (!(gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_CAPTURE))
    return false;

  if (!(gamept->moves[gamept->curr_move+1].special_move_info & SPECIAL_MOVE_CAPTURE))
    return false;

  if (gamept->moves[gamept->curr_move].to != gamept->moves[gamept->curr_move+1].to)
    return false;

  captured_piece = gamept->moves[gamept->curr_move].captured_piece;

  if (!(gamept->curr_move % 2)) {
    if (get_piece1(gamept->board,gamept->moves[gamept->curr_move].to) != ROOK_ID)
      return false;

    if ((captured_piece != KNIGHT_ID * -1) && (captured_piece != BISHOP_ID * -1))
      return false;
  }
  else {
    if (get_piece1(gamept->board,gamept->moves[gamept->curr_move].to) != ROOK_ID * -1)
      return false;

    if ((captured_piece != KNIGHT_ID) && (captured_piece != BISHOP_ID))
      return false;
  }

  return true;
}

int get_enemy_king_file_and_rank(struct game *gamept,int *file_pt,int *rank_pt)
{
  int m;
  int n;
  int enemy_king_id;
  int square;

  if (!gamept->orientation)
    enemy_king_id = KING_ID * -1;
  else
    enemy_king_id = KING_ID;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      square = get_piece2(gamept->board,m,n);

      if (square == enemy_king_id)
        break;
    }

    if (n < 8)
      break;
  }

  if (m < 8) {
    *file_pt = n;
    *rank_pt = m;
    return 1;
  }

  return 0;
}

void copy_board(unsigned char *from_board,unsigned char *to_board)
{
  int n;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    to_board[n] = from_board[n];
}

void position_game(struct game *gamept,int move)
{
  set_initial_board(gamept);

  for (gamept->curr_move = 0; gamept->curr_move < move; gamept->curr_move++) {
    update_board(gamept,NULL,NULL,false);
  }

  calculate_seirawan_counts(gamept);
}
