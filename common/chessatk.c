#include <stdio.h>
#include "chess.h"
#include "chess.glb"
#include "chess.fun"
#include "chess.mac"

int square_attacks_square(struct game *gamept,int square1,int square2)
{
  int retval;
  int piece;
  int color;

  if (square1 == square2)
    return FALSE;

  piece = get_piece1(gamept,square1);

  if (!piece)
    return FALSE;

  retval = FALSE;

  if (piece < 0) {
    piece *= -1;
    color = BLACK;
  }
  else
    color = WHITE;

  switch (piece) {
    case PAWN_ID:
      retval = pawn_attacks_square(gamept,square1,color,square2);

      break;
    case ROOK_ID:
      retval = rook_attacks_square(gamept,square1,square2);

      break;
    case KNIGHT_ID:
      retval = knight_attacks_square(gamept,square1,square2);

      break;
    case BISHOP_ID:
      retval = bishop_attacks_square(gamept,square1,square2);

      break;
    case QUEEN_ID:
      retval = queen_attacks_square(gamept,square1,square2);

      break;
    case KING_ID:
      retval = king_attacks_square(gamept,square1,square2);

      break;
  }

  return retval;
}

int pawn_attacks_square(struct game *gamept,int square1,int color,int square2)
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
        return TRUE;
    }
  }
  else {
    if (rank2 == rank1 - 1) {
      if ((file2 == file1 - 1) || (file2 == file1 + 1))
        return TRUE;
    }
  }

  return FALSE;
}

int rook_attacks_square(struct game *gamept,int square1,int square2)
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

        if (get_piece1(gamept,work_pos))
          break;
      }
    }
    else {
      for (work_file = file1 + 1; work_file < file2; work_file++) {
        work_pos = POS_OF(rank1,work_file);

        if (get_piece1(gamept,work_pos))
          break;
      }
    }

    if (work_file == file2)
      return TRUE;
  }
  else if (file1 == file2) {
    if (rank1 > rank2) {
      for (work_rank = rank1 - 1; work_rank > rank2; work_rank--) {
        work_pos = POS_OF(work_rank,file1);

        if (get_piece1(gamept,work_pos))
          break;
      }
    }
    else {
      for (work_rank = rank1 + 1; work_rank < rank2; work_rank++) {
        work_pos = POS_OF(work_rank,file1);

        if (get_piece1(gamept,work_pos))
          break;
      }
    }

    if (work_rank == rank2)
      return TRUE;
  }

  return FALSE;
}

int knight_attacks_square(struct game *gamept,int square1,int square2)
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
    return TRUE;
  else if ((rank_diff == 1) && (file_diff == 2))
    return TRUE;

  return FALSE;
}

int bishop_attacks_square(struct game *gamept,int square1,int square2)
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
    return FALSE;

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

    if (get_piece1(gamept,work_pos))
      break;
  }

  if (n == rank_diff - 1)
    return TRUE;

  return FALSE;
}

int queen_attacks_square(struct game *gamept,int square1,int square2)
{
  if (rook_attacks_square(gamept,square1,square2))
    return TRUE;

  if (bishop_attacks_square(gamept,square1,square2))
    return TRUE;

  return FALSE;
}

int king_attacks_square(struct game *gamept,int square1,int square2)
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
    return TRUE;

  return FALSE;
}
