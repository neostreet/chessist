#include <stdio.h>
#include <string.h>

#include "chess.h"
#include "chess.glb"
#include "chess.mac"
#include "chess.fun"

int do_castle(struct game *gamept,int direction,char *word,int wordlen)
{
  int rank;

  if (direction == 1)  /* if white's move: */
    rank = 0;
  else
    rank = 7;

  /* make sure the king is on his original square: */
  if (get_piece2(gamept,rank,4) != 6 * direction)
    return 1;

  if (wordlen == 3) {  /* kingside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept,rank,7) != 2 * direction)
      return 2;

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept,rank,5) || get_piece2(gamept,rank,6))
      return 3;

    gamept->moves[gamept->curr_move].special_move_info =
      SPECIAL_MOVE_KINGSIDE_CASTLE;
  }
  else if (wordlen == 5) {  /* queenside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept,rank,0) != 2 * direction)
      return 4;

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept,rank,1) || get_piece2(gamept,rank,2) || get_piece2(gamept,rank,3))
      return 5;

    gamept->moves[gamept->curr_move].special_move_info =
      SPECIAL_MOVE_QUEENSIDE_CASTLE;
  }
  else
    return 6;

  return 0;  /* success */
}

int do_pawn_move(struct game *gamept,int direction,char *word,int wordlen)
{
  int n;
  int file;
  int to_rank;
  int rank;
  int capture_file;
  int piece;
  int which_piece;

  /*printf("%s\n",word);/*for now*/
  file = word[0] - 'a';

  if ((file < 0) || (file > 7))
    return 1;

  /* pawn advance or capture: */
  rank = word[1] - '1';

  if ((rank >= 0) && (rank <= 7)) {
    to_rank = rank;

    /* pawn advance */
    gamept->moves[gamept->curr_move].special_move_info = 0;
    gamept->moves[gamept->curr_move].to = POS_OF(rank,file);

    for (n = 0; n < 2; n++) {
      rank -= direction;

      if ((rank >= 1) && (rank <= 6)) {
        piece = get_piece2(gamept,rank,file);

        if (piece == direction)
          break;

        if (piece)
          return 2;
      }
    }

    if (n == 2)
      return 3;

    if (n == 1) {
      /* only allow two square move from initial position */
      if (direction == 1) {
        if (rank != 1)
          return 4;
      }
      else if (rank != 6)
        return 5;
    }

    gamept->moves[gamept->curr_move].from = POS_OF(rank,file);

    /* handle pawn promotion */
    if (direction == 1) {
      /* white's move */
      if (to_rank == 7) {
        if (wordlen != 3)
          return 6;

        which_piece = get_piece_id_ix(word[2]);

        if (which_piece == NUM_PIECE_TYPES)
          return 7;

        gamept->moves[gamept->curr_move].special_move_info =
          which_piece + 2;
      }
    }
    else {
      /* black's move */
      if (to_rank == 0) {
        if (wordlen != 3)
          return 8;

        which_piece = get_piece_id_ix(word[2]);

        if (which_piece == NUM_PIECE_TYPES)
          return 9;

        gamept->moves[gamept->curr_move].special_move_info =
          (which_piece + 2) * -1;
      }
    }

    return 0;
  }

  /* pawn capture */
  capture_file = word[1] - 'a';

  if ((capture_file < 0) || (capture_file > 7))
    return 10;

  if (capture_file > file) {
    if (capture_file - file != 1)
      return 11;
  }
  else if (file - capture_file != 1)
    return 12;

  if ((wordlen == 2) || (wordlen == 3)) {
    if (direction == 1)
      rank = 1;
    else
      rank = 6;

    for (n = 0; n < 6; n++, rank += direction) {
      /*printf("%d %d\n",get_piece2(gamept,rank,file),
        get_piece2(gamept,rank+direction,capture_file));/*for now*/
      if ((get_piece2(gamept,rank,file) == direction) &&
        (get_piece2(gamept,rank+direction,capture_file) * direction < 0)) {

        if (wordlen == 3)
          if (word[2] - '1' != rank+direction)
            continue;

        gamept->moves[gamept->curr_move].special_move_info =
          SPECIAL_MOVE_CAPTURE;
        gamept->moves[gamept->curr_move].from = POS_OF(rank,file);
        gamept->moves[gamept->curr_move].to = POS_OF(rank+direction,capture_file);
        return 0;
      }
    }

    return 13;
  }

  /* en passant case: */
  if (wordlen == 6) {
    if (!strcmp(&word[wordlen - 4],"e.p.")) {
      if (direction == 1)
        rank = 4;
      else
        rank = 3;

      if ((get_piece2(gamept,rank,file) == direction) &&
        (get_piece2(gamept,rank,capture_file) * direction < 0)) {
        gamept->moves[gamept->curr_move].special_move_info =
          SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT;
        gamept->moves[gamept->curr_move].from = POS_OF(rank,file);
        gamept->moves[gamept->curr_move].to = POS_OF(rank+direction,capture_file);
        return 0;
      }
      else
        return 14;
    }
  }
  else
    return 15;

  return 0;
}

char piece_ids[] = "RNBQK";

int get_piece_id_ix(char piece)
{
  int which_piece;

  /* calculate the id to search for: */
  for (which_piece = 0; which_piece < NUM_PIECE_TYPES; which_piece++) {
    if (piece_ids[which_piece] == piece)
      break;
  }

  return which_piece;
}

int (*piece_functions[])(struct game *,int,int,int,int) = {
  rook_move,
  knight_move,
  bishop_move,
  queen_move,
  king_move
};

int do_piece_move(struct game *gamept,int direction,char *word,int wordlen)
{
  int which_piece;
  int search_piece;
  int curr_file;
  int file_start;
  int file_end;
  int curr_rank;
  int rank_start;
  int rank_end;
  int where;
  int to_file;
  int to_rank;
  int to_piece;
  int retval;

  if (wordlen == 4) {
    where = word[1];

    if ((where >= 'a') && (where <= 'h')) {
      file_start = where - 'a';
      file_end = file_start + 1;
      rank_start = 0;
      rank_end = 8;
    }
    else if ((where >= '1') && (where <= '8')) {
      file_start = 0;
      file_end = 8;
      rank_start = where - '1';
      rank_end = rank_start + 1;
    }
    else
      return 1;
  }
  else {
    file_start = 0;
    file_end = 8;
    rank_start = 0;
    rank_end = 8;
  }

  if (!get_to_position(word,wordlen,&to_file,&to_rank))
    return 2;

  which_piece = get_piece_id_ix(word[0]);

  /* error if not found (should never happen): */
  if (which_piece == NUM_PIECE_TYPES)
    return 3;

  search_piece = (which_piece + 2) * direction; /* calculate search id */
  to_piece = get_piece2(gamept,to_rank,to_file);

  /* don't allow a capture of same color piece*/
  if (to_piece * direction > 0)
    return 4;

  if (to_piece != 0) {
    /* a capture; move the captured piece off the board: */
    gamept->moves[gamept->curr_move].special_move_info =
      SPECIAL_MOVE_CAPTURE;
  }
  else
    gamept->moves[gamept->curr_move].special_move_info = 0;

  /* search the board for the search piece: */
  for (curr_rank = rank_start; curr_rank < rank_end; curr_rank++) {
    for (curr_file = file_start; curr_file < file_end; curr_file++) {
      if (get_piece2(gamept,curr_rank,curr_file) == search_piece) {
        if ((curr_file == to_file) && (curr_rank == to_rank))
          continue;
        else {
          /* see if a possible piece move: */
          retval = (*piece_functions[which_piece])(
            gamept,curr_file,curr_rank,to_file,to_rank);

          if (!retval) {
            gamept->moves[gamept->curr_move].from = POS_OF(curr_rank,curr_file);
            gamept->moves[gamept->curr_move].to = POS_OF(to_rank,to_file);
            return 0;  /* success */
          }
        }
      }
    }
  }

  return 5;
}

int get_to_position(char *word,int wordlen,int *to_filept,int *to_rankpt)
{
  *to_filept = word[wordlen - 2] - 'a';

  if ((*to_filept < 0) || (*to_filept > 7))
    return FALSE;

  *to_rankpt = word[wordlen - 1] - '1';

  if ((*to_rankpt < 0) || (*to_rankpt > 7))
    return FALSE;

  return TRUE;
}

int rook_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int n;

  if (file1 == file2) {
    if (rank1 > rank2) {
      for (n = rank2 + 1; n < rank1; n++)
        if (get_piece2(gamept,n,file1))
          return 1;  /* failure */
    }
    else
      for (n = rank1 + 1; n < rank2; n++)
        if (get_piece2(gamept,n,file1))
          return 2;  /* failure */

    return 0;  /* success */
  }

  if (rank1 == rank2) {
    if (file1 > file2) {
      for (n = file2 + 1; n < file1; n++)
        if (get_piece2(gamept,rank1,n))
          return 1;  /* failure */
    }
    else
      for (n = file1 + 1; n < file2; n++)
        if (get_piece2(gamept,rank1,n))
          return 2;  /* failure */

    return 0;  /* success */
  }

  return 3;  /* failure */
}

int knight_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int dist1;
  int dist2;

  dist1 = (file1 - file2);

  if (dist1 < 0)
    dist1 *= -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0)
    dist2 *= -1;

  if ((dist1 == 1) && (dist2 == 2))
    return 0;  /* success */

  if ((dist1 == 2) && (dist2 == 1))
    return 0;  /* success */

  return 1;    /* failure */
}

int bishop_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int dist1;
  int dist2;
  int file_dir;
  int rank_dir;

  dist1 = (file1 - file2);

  if (dist1 < 0) {
    dist1 *= -1;
    file_dir = 1;
  }
  else
    file_dir = -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0) {
    dist2 *= -1;
    rank_dir = 1;
  }
  else
    rank_dir = -1;

  if (dist1 != dist2)
    return 1;  /* failure */

  /* make sure there are no intervening pieces */
  for ( ; ; ) {
    file1 += file_dir;

    if (file1 == file2)
      break;

    rank1 += rank_dir;

    if (get_piece2(gamept,rank1,file1))
      return 2;  /* failure */
  }

  return 0;  /* success */
}

int queen_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  if (!rook_move(gamept,file1,rank1,file2,rank2))
    return 0;  /* success */

  if (!bishop_move(gamept,file1,rank1,file2,rank2))
    return 0;  /* success */

  return 1;    /* failure */
}

int king_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int dist1;
  int dist2;

  dist1 = (file1 - file2);

  if (dist1 < 0)
    dist1 *= -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0)
    dist2 *= -1;

  if ((dist1 < 2) && (dist2 < 2))
    return 0;  /* success */

  return 1;  /* failure */
}
