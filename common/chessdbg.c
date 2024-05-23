#include <stdio.h>
#include <string.h>

#include "chess.h"
#include "chess.fun"
#include "chess.glb"
#include "chess.mac"

static char buf[20];

bool kingside_castle(struct game *gamept)
{
  if (!((gamept->curr_move) % 2)) {
    // white

    if ((gamept->moves[gamept->curr_move].from == 4) &&
      (gamept->moves[gamept->curr_move].to == 6))
      return true;
  }
  else {
    // black

    if ((gamept->moves[gamept->curr_move].from == 60) &&
      (gamept->moves[gamept->curr_move].to == 62))
      return true;
  }

  return false;
}

bool queenside_castle(struct game *gamept)
{
  if (!((gamept->curr_move) % 2)) {
    // white

    if ((gamept->moves[gamept->curr_move].from == 4) &&
      (gamept->moves[gamept->curr_move].to == 2))
      return true;
  }
  else {
    // black

    if ((gamept->moves[gamept->curr_move].from == 60) &&
      (gamept->moves[gamept->curr_move].to == 58))
      return true;
  }

  return false;
}

char decode_piece(int piece,bool bShowBlack)
{
  bool bBlack;

  if (piece < 0) {
    bBlack = true;
    piece *= -1;
  }
  else
    bBlack = false;

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

  piece = get_piece1(gamept->board,
    gamept->moves[gamept->curr_move].from);
  return decode_piece(piece,false);
}

char get_from_file(struct game *gamept)
{
  return 'a' + FILE_OF(gamept->moves[gamept->curr_move].from);
}

void print_game(struct game *gamept)
{
  int n;
  int save_len;

  printf(fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move < gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20,true);

    if (gamept->curr_move % 2) {
      for (n = save_len; n < 12; n++)
        putchar(' ');
    }
    else
      save_len = strlen(buf);

    printf("%s",buf);

    if (gamept->curr_move % 2)
      putchar(0x0a);

    update_board(gamept,NULL,NULL,false);
  }

  if (gamept->num_moves % 2)
    putchar(0x0a);
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

void sprintf_move(struct game *gamept,char *buf,int buf_len,bool bInline)
{
  bool bWhite;
  int put_count;
  char decoded_piece;
  bool bDone;
  int from;
  int from_file;
  int to;
  int to_file;
  int to_rank;

  bWhite = !((gamept->curr_move) % 2);

  if (bWhite || !bInline) {
    sprintf(buf,"%2d. ",((gamept->curr_move) / 2) + 1);
    put_count = strlen(buf);
  }
  else
    put_count = 0;

  if (!bInline && !bWhite) {
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

    if (decoded_piece == 'P')
      bDone = false;
    else {
      buf[put_count++] = decoded_piece;
      bDone = false;
    }

    if (!bDone) {
      if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_CAPTURE) {
        if (decoded_piece == 'P')
          buf[put_count++] = get_from_file(gamept);

        buf[put_count++] = 'x';
      }

      to = gamept->moves[gamept->curr_move].to;
      to_file = FILE_OF(to);
      to_rank = RANK_OF(to);
      buf[put_count++] = 'a' + to_file;
      buf[put_count++] = '1' + to_rank;
    }
  }

  if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_CHECK)
    buf[put_count++] = '+';

  if (!bInline) {
    for ( ; put_count < buf_len - 1; put_count++)
      buf[put_count] = ' ';
  }

  buf[put_count] = 0;
}

void print_from_to(struct game *gamept)
{
  printf(fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0; gamept->curr_move < gamept->num_moves; gamept->curr_move++) {
    printf("%2d %2d\n",gamept->moves[gamept->curr_move].from,gamept->moves[gamept->curr_move].to);
    print_special_moves(gamept);
  }
}
