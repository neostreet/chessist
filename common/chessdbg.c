#include <stdio.h>
#include <string.h>

#include "chess.h"
#include "chess.fun"
#include "chess.glb"
#include "chess.mac"

extern char piece_ids[]; /* "RNBQK" */

static char buf[20];
char fmt_str[] = "%s\n";

int kingside_castle(struct game *gamept)
{
  return (gamept->moves[gamept->curr_move-1].special_move_info &
    SPECIAL_MOVE_KINGSIDE_CASTLE);
}

int queenside_castle(struct game *gamept)
{
  return (gamept->moves[gamept->curr_move-1].special_move_info &
    SPECIAL_MOVE_QUEENSIDE_CASTLE);
}

char decode_piece(int piece,int bShowBlack)
{
  int bBlack;

  if (piece < 0) {
    bBlack = TRUE;
    piece *= -1;
  }
  else
    bBlack = FALSE;

  if (!piece)
    return '.';

  if (piece == 1) {
    if (!bShowBlack || bBlack)
      return 'P';
    else
      return 'p';
  }

  piece -= 2;

  if (!bShowBlack || bBlack)
    return piece_ids[piece];
  else
    return piece_ids[piece] + 'a' - 'A';
}

char get_decoded_piece(struct game *gamept)
{
  int piece;

  piece = get_piece1(gamept,
    gamept->moves[gamept->curr_move-1].to);
  return decode_piece(piece,FALSE);
}

void print_game(struct game *gamept)
{
  printf(fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20);
    printf(fmt_str,buf);

    update_board(gamept,FALSE);
  }
}

void fprintf_move(FILE *fptr,struct game *gamept)
{
  if (gamept->curr_move == gamept->num_moves)
    return;

  fprintf(fptr,"%d %d %d\n",
    gamept->moves[gamept->curr_move].from,
    gamept->moves[gamept->curr_move].to,
    gamept->moves[gamept->curr_move].special_move_info);
}

void sprintf_move(struct game *gamept,char *buf,int buf_len)
{
  int bWhite;
  int put_count;
  char decoded_piece;
  int bDone;
  int from;
  int from_file;
  int to;
  int to_file;
  int to_rank;

  if (!gamept->curr_move) {
    strcpy(buf,"Starting position");
    put_count = strlen(buf);

    for ( ; put_count < buf_len - 1; put_count++)
      buf[put_count] = ' ';

    buf[put_count] = 0;

    return;
  }

  bWhite = !((gamept->black_to_play + gamept->curr_move-1) % 2);

  sprintf(buf,"%2d. ",((gamept->curr_move-1) / 2) + 1);
  put_count = 4;

  if (!bWhite) {\
    sprintf(&buf[put_count],"...  ");
    put_count += 5;
  }

  if (kingside_castle(gamept)) {
    sprintf(&buf[put_count],"O-O");
    put_count += 3;
  }
  else if (queenside_castle(gamept)) {
    sprintf(&buf[put_count],"O-O-O");
    put_count += 5;
  }
  else {
    decoded_piece = get_decoded_piece(gamept);

    if (decoded_piece == 'P') {
      if (gamept->moves[gamept->curr_move-1].special_move_info &
        SPECIAL_MOVE_CAPTURE) {
        from = gamept->moves[gamept->curr_move-1].from;
        from_file = FILE_OF(from);
        to = gamept->moves[gamept->curr_move-1].to;
        to_file = FILE_OF(to);
        buf[put_count++] = 'a' + from_file;
        buf[put_count++] = 'x';
        buf[put_count++] = 'a' + to_file;
        bDone = TRUE;
      }
      else
        bDone = FALSE;
    }
    else {
      buf[put_count++] = decoded_piece;
      bDone = FALSE;

      if (gamept->moves[gamept->curr_move-1].special_move_info &
        SPECIAL_MOVE_CAPTURE)
        buf[put_count++] = 'x';
    }

    if (!bDone) {
      to = gamept->moves[gamept->curr_move-1].to;
      to_file = FILE_OF(to);
      to_rank = RANK_OF(to);
      buf[put_count++] = 'a' + to_file;
      buf[put_count++] = '1' + to_rank;
    }
  }

  for ( ; put_count < buf_len - 1; put_count++)
    buf[put_count] = ' ';

  buf[put_count] = 0;
}
