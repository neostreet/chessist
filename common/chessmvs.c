#include <stdio.h>
#include <string.h>

#include "chess.h"
#include "chess.glb"
#include "chess.mac"
#include "chess.fun"

using namespace std;

static struct game scratch;

int do_castle(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
{
  int rank;
  int special_move_info;

  if (direction == 1)  /* if white's move: */
    rank = 0;
  else
    rank = 7;

  /* make sure the king is on his original square: */
  if (get_piece2(gamept->board,rank,4) != 6 * direction) {
    do_castle_failures++;
    return 1;
  }

  if (wordlen == 3) {  /* kingside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept->board,rank,7) != 2 * direction) {
      do_castle_failures++;
      return 2;
    }

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept->board,rank,5) || get_piece2(gamept->board,rank,6)) {
      do_castle_failures++;
      return 3;
    }

    if (direction == 1) {  /* if white's move: */
      move_ptr->from = 4;
      move_ptr->to = 6;
    }
    else {
      move_ptr->from = 60;
      move_ptr->to = 62;
    }

    special_move_info = SPECIAL_MOVE_KINGSIDE_CASTLE;
  }
  else if (wordlen == 5) {  /* queenside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept->board,rank,0) != 2 * direction) {
      do_castle_failures++;
      return 4;
    }

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept->board,rank,1) || get_piece2(gamept->board,rank,2) || get_piece2(gamept->board,rank,3)) {
      do_castle_failures++;
      return 5;
    }

    if (direction == 1) {  /* if white's move: */
      move_ptr->from = 4;
      move_ptr->to = 2;
    }
    else {
      move_ptr->from = 60;
      move_ptr->to = 58;
    }

    special_move_info = SPECIAL_MOVE_QUEENSIDE_CASTLE;
  }
  else {
    do_castle_failures++;
    return 6;
  }

  move_ptr->special_move_info = special_move_info;
  do_castle_successes++;

  return 0;  /* success */
}

int do_pawn_move(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
{
  int n;
  int file;
  int to_rank;
  int rank;
  int capture_file;
  int piece;
  int which_piece;
  bool bBlack;

  if (debug_fptr)
    fprintf(debug_fptr,"do_pawn_move: curr_move = %d, word = %s\n",gamept->curr_move,word);

  /*printf("%s\n",word);/*for now*/
  file = word[0] - 'a';

  if ((file < 0) || (file > 7))
    return 1;

  /* pawn advance or capture: */
  rank = word[1] - '1';

  if ((rank >= 0) && (rank <= 7)) {
    to_rank = rank;

    /* pawn advance */
    move_ptr->special_move_info = 0;
    move_ptr->to = POS_OF(rank,file);

    for (n = 0; n < 2; n++) {
      rank -= direction;

      if ((rank >= 1) && (rank <= 6)) {
        piece = get_piece2(gamept->board,rank,file);

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

    move_ptr->from = POS_OF(rank,file);

    /* handle pawn promotion */
    if (direction == 1) {
      /* white's move */
      if (to_rank == 7) {
        if (wordlen != 3)
          return 6;

        switch (word[2]) {
          case 'Q':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_QUEEN;

            break;
          case 'R':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_ROOK;

            break;
          case 'N':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_KNIGHT;

            break;
          case 'B':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_BISHOP;

            break;
        }
      }
    }
    else {
      /* black's move */
      if (to_rank == 0) {
        if (wordlen != 3)
          return 8;

        switch (word[2]) {
          case 'Q':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_QUEEN;

            break;
          case 'R':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_ROOK;

            break;
          case 'N':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_KNIGHT;

            break;
          case 'B':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_BISHOP;

            break;
        }
      }
    }

    goto check_for_illegal_move;
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

  if ((wordlen >= 2) && (wordlen <= 4)) {
    if (direction == 1)
      rank = 1;
    else
      rank = 6;

    for (n = 0; n < 6; n++, rank += direction) {
      /*printf("%d %d\n",get_piece2(gamept->board,rank,file),
        get_piece2(gamept->board,rank+direction,capture_file));/*for now*/
      if ((get_piece2(gamept->board,rank,file) == direction) &&
        (get_piece2(gamept->board,rank+direction,capture_file) * direction < 0)) {

        if ((wordlen == 3) || (wordlen == 4))
          if (word[2] - '1' != rank+direction)
            continue;

        move_ptr->from = POS_OF(rank,file);
        move_ptr->to = POS_OF(rank+direction,capture_file);

        if (wordlen == 4) {
          switch (word[3]) {
            case 'Q':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_QUEEN;

              break;
            case 'R':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_ROOK;

              break;
            case 'N':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_KNIGHT;

              break;
            case 'B':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_BISHOP;

              break;
          }
        }

        goto check_for_illegal_move;
      }
    }

    // handle en passant captures
    if (wordlen == 3) {
      if (direction == -1) {
        // black
        if (word[2] == '3') {
          if ((get_piece2(gamept->board,3,file) == direction) &&
              (get_piece2(gamept->board,3,capture_file) == direction * -1)) {
            move_ptr->from = POS_OF(3,file);
            move_ptr->to = POS_OF(2,capture_file);
            move_ptr->special_move_info |=
              SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
            goto check_for_illegal_move;
          }
        }
      }
      else {
        // white
        if (word[2] == '6') {
          if ((get_piece2(gamept->board,4,file) == direction) &&
              (get_piece2(gamept->board,4,capture_file) == direction * -1)) {
            move_ptr->from = POS_OF(4,file);
            move_ptr->to = POS_OF(5,capture_file);
            move_ptr->special_move_info |=
              SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
            goto check_for_illegal_move;
          }
        }
      }
    }

    return 13;
  }

  return 14;

check_for_illegal_move:

  // don't allow moves which would put the mover in check; use a scratch game
  // to achieve this

  copy_game(&scratch,gamept);
  scratch.moves[scratch.curr_move].from = move_ptr->from;
  scratch.moves[scratch.curr_move].to = move_ptr->to;
  update_board(&scratch,NULL,NULL);
  bBlack = scratch.curr_move & 0x1;

  if (player_is_in_check(bBlack,scratch.board,scratch.curr_move))
    return 15;

  return 0;
}

int do_pawn_move2(struct game *gamept)
{
  bool bWhiteMove;
  bool bBlack;
  int start_rank;
  int start_file;
  int end_rank;
  int end_file;
  int rank_diff;
  int file_diff;
  int retval;

  bWhiteMove = (move_start_square_piece > 0);

  if (bWhiteMove) {
    // white pawn move

    if (move_start_square > move_end_square)
      return 1; // failure
  }
  else {
    // black pawn move

    if (move_start_square < move_end_square)
      return 2; // failure
  }

  start_rank = RANK_OF(move_start_square);
  start_file = FILE_OF(move_start_square);
  end_rank = RANK_OF(move_end_square);
  end_file = FILE_OF(move_end_square);

  if (start_rank >= end_rank)
    rank_diff = start_rank - end_rank;
  else
    rank_diff = end_rank - start_rank;

  if (start_file >= end_file)
    file_diff = start_file - end_file;
  else
    file_diff = end_file - start_file;

  if (file_diff == 0) {
    if (move_end_square_piece)
      return 3; // failure
  }

  if (rank_diff == 0)
    return 4; // failure

  if (file_diff > 1)
    return 5; // failure

  if (rank_diff > 2)
    return 6; // failure

  if (rank_diff > 1) {
    if (file_diff)
      return 7; // failure

    if (bWhiteMove) {
      if (start_rank != 1)
        return 8; // failure
    }
    else {
      if (start_rank != 6)
        return 9; // failure
    }
  }

  if (file_diff == 1) {
    if (rank_diff != 1)
      return 10; // failure

    if (!move_end_square_piece) {
      // check for en passant capture
      if (bWhiteMove && (start_rank == 4) &&
        (get_piece2(gamept->board,4,end_file) == PAWN_ID * -1) &&
        (gamept->moves[gamept->curr_move-1].special_move_info == SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

        gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_EN_PASSANT_CAPTURE;
      }
      else if (!bWhiteMove && (start_rank == 3) &&
        (get_piece2(gamept->board,3,end_file) == PAWN_ID) &&
        (gamept->moves[gamept->curr_move-1].special_move_info == SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

        gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_EN_PASSANT_CAPTURE;
      }
      else
        return 11; // failure
    }
  }

  // don't allow moves which would put the mover in check; use a scratch game
  // to achieve this

  copy_game(&scratch,gamept);
  scratch.moves[scratch.curr_move].from = move_start_square;
  scratch.moves[scratch.curr_move].to = move_end_square;
  update_board(&scratch,NULL,NULL);
  bBlack = scratch.curr_move & 0x1;

  if (player_is_in_check(bBlack,scratch.board,scratch.curr_move))
    return 12;

  gamept->moves[gamept->curr_move].from = move_start_square;
  gamept->moves[gamept->curr_move].to = move_end_square;
  retval = 0;

  if (rank_diff > 1)
    gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE;

  return retval; // success
}

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

int (*piece_functions2[])(struct game *) = {
  rook_move2,
  knight_move2,
  bishop_move2,
  queen_move2,
  king_move2
};

int do_piece_move(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
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
  bool bBlack;
  int dbg;

  if (debug_fptr)
    fprintf(debug_fptr,"do_piece_move: curr_move = %d, word = %s\n",gamept->curr_move,word);

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
  to_piece = get_piece2(gamept->board,to_rank,to_file);

  /* don't allow a capture of same color piece*/
  if (to_piece * direction > 0)
    return 4;

  move_ptr->special_move_info = 0;

  /* search the board for the search piece: */
  for (curr_rank = rank_start; curr_rank < rank_end; curr_rank++) {
    for (curr_file = file_start; curr_file < file_end; curr_file++) {
      if (get_piece2(gamept->board,curr_rank,curr_file) == search_piece) {
        /* see if a possible piece move: */
        retval = (*piece_functions[which_piece])(
          gamept,curr_file,curr_rank,to_file,to_rank);

        if (!retval) {
          move_ptr->from = POS_OF(curr_rank,curr_file);
          move_ptr->to = POS_OF(to_rank,to_file);

          // don't allow moves which would put the mover in check; use a scratch game
          // to achieve this

          copy_game(&scratch,gamept);
          scratch.moves[scratch.curr_move].from = move_ptr->from;
          scratch.moves[scratch.curr_move].to = move_ptr->to;
          update_board(&scratch,NULL,NULL);
          bBlack = scratch.curr_move & 0x1;

          if (!player_is_in_check(bBlack,scratch.board,scratch.curr_move))
            return 0;  /* success */
        }
      }
    }
  }

  return 5;
}

int do_piece_move2(struct game *gamept)
{
  int which_piece;
  int retval;
  bool bBlack;

  which_piece = move_start_square_piece;

  if (which_piece < 0)
    which_piece *= -1;

  which_piece -= ROOK_ID;

  retval = (*piece_functions2[which_piece])(gamept);

  if (retval)
    return 1;

  // don't allow moves which would put the mover in check; use a scratch game
  // to achieve this

  copy_game(&scratch,gamept);
  scratch.moves[scratch.curr_move].from = move_start_square;
  scratch.moves[scratch.curr_move].to = move_end_square;
  update_board(&scratch,NULL,NULL);
  bBlack = scratch.curr_move & 0x1;

  if (player_is_in_check(bBlack,scratch.board,scratch.curr_move))
    return 1;

  gamept->moves[gamept->curr_move].from = move_start_square;
  gamept->moves[gamept->curr_move].to = move_end_square;
  return 0;  /* success */
}

int get_to_position(char *word,int wordlen,int *to_filept,int *to_rankpt)
{
  *to_filept = word[wordlen - 2] - 'a';

  if ((*to_filept < 0) || (*to_filept > 7))
    return false;

  *to_rankpt = word[wordlen - 1] - '1';

  if ((*to_rankpt < 0) || (*to_rankpt > 7))
    return false;

  return true;
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
        if (get_piece2(gamept->board,n,file1))
          return 1;  /* failure */
    }
    else
      for (n = rank1 + 1; n < rank2; n++)
        if (get_piece2(gamept->board,n,file1))
          return 2;  /* failure */

    return 0;  /* success */
  }

  if (rank1 == rank2) {
    if (file1 > file2) {
      for (n = file2 + 1; n < file1; n++)
        if (get_piece2(gamept->board,rank1,n))
          return 1;  /* failure */
    }
    else
      for (n = file1 + 1; n < file2; n++)
        if (get_piece2(gamept->board,rank1,n))
          return 2;  /* failure */

    return 0;  /* success */
  }

  return 3;  /* failure */
}

int rook_move2(
  struct game *gamept
)
{
  int retval;

  retval = rook_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
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

int knight_move2(
  struct game *gamept
)
{
  int retval;

  retval = knight_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
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

    if (get_piece2(gamept->board,rank1,file1))
      return 2;  /* failure */
  }

  return 0;  /* success */
}

int bishop_move2(
  struct game *gamept
)
{
  int retval;

  retval = bishop_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
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

int queen_move2(
  struct game *gamept
)
{
  int retval;

  retval = queen_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
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

int king_move2(
  struct game *gamept
)
{
  int retval;

  retval = king_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}

void get_legal_moves(struct game *gamept,struct move *legal_moves,int *legal_moves_count)
{
  int n;
  bool bWhiteToMove;
  struct piece_info *info_pt;
  unsigned char board[CHARS_IN_BOARD];
  char piece_id;

  bWhiteToMove = !(gamept->num_moves % 2);

  if (bWhiteToMove)
    info_pt = gamept->white_pieces;
  else
    info_pt = gamept->black_pieces;

  print_piece_info2(info_pt); // for now
  putchar(0x0a);
  populate_board_from_piece_info(gamept,board);
  print_bd0(board,gamept->orientation);

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (info_pt[n].current_board_position == -1)
      continue;

    piece_id = info_pt[n].piece_id;

    if (piece_id < 0)
      piece_id *= -1;

    switch (piece_id) {
      case PAWN_ID:
        legal_pawn_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case ROOK_ID:
        legal_rook_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case KNIGHT_ID:
        legal_knight_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case BISHOP_ID:
        legal_bishop_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case QUEEN_ID:
        legal_queen_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case KING_ID:
        legal_king_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
    }
  }
}

void legal_pawn_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int square;
  int rank;
  int file;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
}

void legal_rook_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int square;
  int rank;
  int file;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
}

struct knight_move_offset {
  char rank_offset;
  char file_offset;
};

struct knight_move_offset offsets[] = {
  1, 2,
  2, 1,
  1, -2,
  2, -1,
  -1, -2,
  -2, -1,
  -1, 2,
  -2, 1
};
#define NUM_OFFSETS (sizeof offsets / sizeof(struct knight_move_offset))

void legal_knight_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  bool bBlack;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);

  for (n = 0; n < NUM_OFFSETS; n++) {
    work_rank = rank + offsets[n].rank_offset;
    work_file = file + offsets[n].file_offset;

    if ((work_rank < 0) || (work_rank >= NUM_RANKS))
      continue;

    if ((work_file < 0) || (work_file >= NUM_FILES))
      continue;

    square2 = get_piece2(gamept->board,work_rank,work_file);

    // can't capture a piece of the same color
    if ((square * square2) > 0)
      continue;

    // don't allow moves which would put the mover in check; use a scratch game
    // to achieve this

    copy_game(&scratch,gamept);
    scratch.moves[scratch.curr_move].from = current_board_position;
    scratch.moves[scratch.curr_move].to = POS_OF(work_rank,work_file);
    update_board(&scratch,NULL,NULL);
    bBlack = scratch.curr_move & 0x1;

    if (player_is_in_check(bBlack,scratch.board,scratch.curr_move))
      continue;

    if (*legal_moves_count < MAX_LEGAL_MOVES) {
      legal_moves[*legal_moves_count].from = current_board_position;
      legal_moves[*legal_moves_count].to = POS_OF(work_rank,work_file);
      legal_moves[*legal_moves_count].special_move_info = 0;
      (*legal_moves_count)++;
    }
  }
}

void legal_bishop_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int square;
  int rank;
  int file;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
}

void legal_queen_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int square;
  int rank;
  int file;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
}

void legal_king_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int square;
  int rank;
  int file;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
}

void make_a_move(struct game *gamept)
{
  legal_moves_count =  0;

  get_legal_moves(gamept,&legal_moves[0],&legal_moves_count);
}
