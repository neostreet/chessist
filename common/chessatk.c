#include <stdio.h>
#include "chess.h"
#include "chess.glb"
#include "chess.fun"
#include "chess.mac"

int square_attacks_square(unsigned char *board,int square1,int square2)
{
  int retval;
  int piece;
  int color;

  if (square1 == square2)
    return false;

  piece = get_piece1(board,square1);

  if (!piece)
    return false;

  retval = false;

  if (piece < 0) {
    piece *= -1;
    color = BLACK;
  }
  else
    color = WHITE;

  switch (piece) {
    case PAWN_ID:
      retval = pawn_attacks_square(board,square1,color,square2);

      break;
    case ROOK_ID:
      retval = rook_attacks_square(board,square1,square2);

      break;
    case KNIGHT_ID:
      retval = knight_attacks_square(board,square1,square2);

      break;
    case BISHOP_ID:
      retval = bishop_attacks_square(board,square1,square2);

      break;
    case QUEEN_ID:
      retval = queen_attacks_square(board,square1,square2);

      break;
    case KING_ID:
      retval = king_attacks_square(board,square1,square2);

      break;
  }

  return retval;
}

int pawn_attacks_square(unsigned char *board,int square1,int color,int square2)
{
  int rank1;
  int file1;
  int rank2;
  int file2;

  rank1 = RANK_OF(square1);
  file1 = FILE_OF(square1);
  rank2 = RANK_OF(square2);
  file2 = FILE_OF(square2);

  if (color == WHITE) {
    if (rank2 == rank1 + 1) {
      if ((file2 == file1 - 1) || (file2 == file1 + 1))
        return true;
    }
  }
  else {
    if (rank2 == rank1 - 1) {
      if ((file2 == file1 - 1) || (file2 == file1 + 1))
        return true;
    }
  }

  return false;
}

int rook_attacks_square(unsigned char *board,int square1,int square2)
{
  int rank1;
  int file1;
  int rank2;
  int file2;
  int work_rank;
  int work_file;
  int work_pos;

  rank1 = RANK_OF(square1);
  file1 = FILE_OF(square1);
  rank2 = RANK_OF(square2);
  file2 = FILE_OF(square2);

  if (rank1 == rank2) {
    if (file1 > file2) {
      for (work_file = file1 - 1; work_file > file2; work_file--) {
        work_pos = POS_OF(rank1,work_file);

        if (get_piece1(board,work_pos))
          break;
      }
    }
    else {
      for (work_file = file1 + 1; work_file < file2; work_file++) {
        work_pos = POS_OF(rank1,work_file);

        if (get_piece1(board,work_pos))
          break;
      }
    }

    if (work_file == file2)
      return true;
  }
  else if (file1 == file2) {
    if (rank1 > rank2) {
      for (work_rank = rank1 - 1; work_rank > rank2; work_rank--) {
        work_pos = POS_OF(work_rank,file1);

        if (get_piece1(board,work_pos))
          break;
      }
    }
    else {
      for (work_rank = rank1 + 1; work_rank < rank2; work_rank++) {
        work_pos = POS_OF(work_rank,file1);

        if (get_piece1(board,work_pos))
          break;
      }
    }

    if (work_rank == rank2)
      return true;
  }

  return false;
}

int knight_attacks_square(unsigned char *board,int square1,int square2)
{
  int rank1;
  int file1;
  int rank2;
  int file2;
  int rank_diff;
  int file_diff;

  rank1 = RANK_OF(square1);
  file1 = FILE_OF(square1);
  rank2 = RANK_OF(square2);
  file2 = FILE_OF(square2);

  rank_diff = ABS_VAL(rank1,rank2);
  file_diff = ABS_VAL(file1,file2);

  if ((rank_diff == 2) && (file_diff == 1))
    return true;
  else if ((rank_diff == 1) && (file_diff == 2))
    return true;

  return false;
}

int bishop_attacks_square(unsigned char *board,int square1,int square2)
{
  int n;
  int rank1;
  int file1;
  int rank2;
  int file2;
  int rank_diff;
  int file_diff;
  int work_rank;
  int work_file;
  int rank_incr;
  int file_incr;
  int work_pos;

  rank1 = RANK_OF(square1);
  file1 = FILE_OF(square1);
  rank2 = RANK_OF(square2);
  file2 = FILE_OF(square2);

  rank_diff = ABS_VAL(rank1,rank2);
  file_diff = ABS_VAL(file1,file2);

  if (rank_diff != file_diff)
    return false;

  if (rank1 > rank2)
    rank_incr = -1;
  else
    rank_incr = 1;

  if (file1 > file2)
    file_incr = -1;
  else
    file_incr = 1;

  work_rank = rank1;
  work_file = file1;

  for (n = 0; n < rank_diff - 1; n++) {
    work_rank += rank_incr;
    work_file += file_incr;

    work_pos = POS_OF(work_rank,work_file);

    if (get_piece1(board,work_pos))
      break;
  }

  if (n == rank_diff - 1)
    return true;

  return false;
}

int queen_attacks_square(unsigned char *board,int square1,int square2)
{
  if (rook_attacks_square(board,square1,square2))
    return true;

  if (bishop_attacks_square(board,square1,square2))
    return true;

  return false;
}

int king_attacks_square(unsigned char *board,int square1,int square2)
{
  int rank1;
  int file1;
  int rank2;
  int file2;
  int rank_diff;
  int file_diff;

  rank1 = RANK_OF(square1);
  file1 = FILE_OF(square1);
  rank2 = RANK_OF(square2);
  file2 = FILE_OF(square2);

  rank_diff = ABS_VAL(rank1,rank2);
  file_diff = ABS_VAL(file1,file2);

  if ((rank_diff <= 1) && (file_diff <= 1))
    return true;

  return false;
}

static int debug_move = -1;

bool any_opponent_piece_attacks_square(int square,bool bBlack,unsigned char *board,int curr_move)
{
  int n;
  int piece;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (!piece)
      continue;

    if (bBlack) {
      if (piece < 0)
        continue;
    }
    else {
      if (piece > 0)
        continue;
    }

    if (square_attacks_square(board,n,square)) {
      if (debug_fptr)
        fprint_bd2(board,debug_fptr);

      return true;
    }
  }

  return false;
}

bool player_is_in_check(bool bBlack,unsigned char *board,int curr_move)
{
  int n;
  int movers_king;
  int movers_king_square;
  int piece;
  int dbg;

  if (curr_move == debug_move)
    dbg = 1;

  if (debug_fptr)
    fprintf(debug_fptr,"player_is_in_check: bBlack = %d, curr_move = %d\n",bBlack,curr_move);

  // first, find the mover's king

  if (bBlack)
    movers_king = KING_ID * -1;
  else
    movers_king = KING_ID;

  movers_king_square = -1;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == movers_king) {
      movers_king_square = n;
      break;
    }
  }

  if (n == NUM_BOARD_SQUARES)
    return false; // should never happen

  // now determine if any of the opponent's pieces attack the mover's king
  if (any_opponent_piece_attacks_square(movers_king_square,bBlack,board,curr_move))
    return true;

  return false;
}

int calc_square(char *algebraic_notation)
{
  char file_char;
  char rank_char;
  int file;
  int rank;

  file_char = algebraic_notation[0];
  rank_char = algebraic_notation[1];

  if ((file_char >= 'a') && (file_char <= 'h'))
    file = file_char - 'a';
  else
    return -1;

  if ((rank_char >= '1') && (rank_char <= '8'))
    rank = rank_char - '1';
  else
    return -1;

  return rank * NUM_FILES + file;
}
