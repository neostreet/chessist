#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "chess.h"
#include "chess.fun"
#include "chess.glb"
#include "chess.mac"
#include "bitfuns.h"

static unsigned char canonical_initial_board[] = {
  (unsigned char)0x23, (unsigned char)0x45, (unsigned char)0x64, (unsigned char)0x32,
  (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff,
  (unsigned char)0xed, (unsigned char)0xcb, (unsigned char)0xac, (unsigned char)0xde
};

static int force_values[] = {
  FORCE_VALUE_PAWN,
  FORCE_VALUE_ROOK,
  FORCE_VALUE_KNIGHT,
  FORCE_VALUE_BISHOP,
  FORCE_VALUE_QUEEN,
  FORCE_VALUE_KING
};

static char corrupted_msg[] = "game corrupted\n";
static char bad_castle[] = "bad castle";
static char bad_pawn_move[] = "bad pawn move";

static char *bad_piece_move[] = {
  "bad rook move",
  "bad knight move",
  "bad bishop move",
  "bad queen move",
  "bad king move"
};

static int fen2pos(char *line,int line_len,unsigned char *pos_buf,int *black_to_play);

short force_value_of(short piece)
{
  if (piece < 0)
    piece *= -1;

  piece--;

  return force_values[piece];
}

int line_number(char *word,int wordlen)
{
  int n;

  for (n = 0; n < wordlen; n++) {
    if ((word[n] < '0') || (word[n] > '9')) {
      if ((word[n] == '.') && (n == wordlen - 1))
        break;

      return false;
    }
  }

  return true;
}

int get_piece_type_ix(int chara)
{
  int n;

  for (n = 0; n < NUM_PIECE_TYPES; n++)
    if (chara == piece_ids[n])
      return n;

  return 0; /* should never happen */
}

void set_initial_board(struct game *gamept)
{
  int n;

  if (gamept->has_custom_initial_board) {
    for (n = 0; n < CHARS_IN_BOARD; n++)
      gamept->board[n] = custom_initial_board[n];
  }
  else {
    for (n = 0; n < CHARS_IN_BOARD; n++)
      gamept->board[n] = canonical_initial_board[n];
  }

  calculate_force_counts(gamept);

  populate_piece_info_from_board(gamept->board,gamept->white_pieces,gamept->black_pieces);
}

int read_game(char *filename,struct game *gamept)
{
  int filename_len;
  FILE *fptr;
  int chara;
  int end_of_file;
  int m;
  int n;
  int p;
  char word[WORDLEN];
  int wordlen;
  bool bHaveFirstWord;
  int direction;
  int word_no;
  int dbg;
  int retval;
  int got_error;
  bool bBlack;

  filename_len = strlen(filename);

  if (filename_len > 4) {
    if (!strcmp(&filename[filename_len - 4],".bin"))
      return read_binary_game(filename,gamept);
  }

  bzero(gamept,sizeof (struct game));

  gamept->curr_move = -1;

  if ((fptr = fopen(filename,"r")) == NULL)
    return 1;

  fscanf(fptr,"%d",&gamept->orientation);  /* get board orientation */
                                   /* 0 = standard, 1 = black on bottom */

  end_of_file = get_word(fptr,word,WORDLEN,&wordlen,gamept);
  bHaveFirstWord = true;

  set_initial_board(gamept);

  end_of_file = 0;
  gamept->curr_move = 0;

  gamept->title[0] = 0;
  word_no = 0;
  got_error = 0;

  for ( ; ; ) {
    if (word_no || !bHaveFirstWord)
      end_of_file = get_word(fptr,word,WORDLEN,&wordlen,gamept);

    if (end_of_file)
      break;

    word_no++;

    if (gamept->curr_move == dbg_move)
      dbg = 1;

    /* ignore line numbers: */
    if (line_number(word,wordlen))
      continue;

    if (!strncmp(word,"title",5)) {
      m = 5;
      n = 0;

      for ( ; ; ) {
        chara = word[m++];

        if (!chara)
          break;

        if (n < MAX_TITLE_LEN - 1) {
          if (chara == '\\')
            gamept->title[n++] = ' ';
          else
            gamept->title[n++] = chara;
        }
      }

      gamept->title[n] = 0;

      for (m = n-1; m >= 0; m--) {
        if (gamept->title[m] == ' ')
          break;
      }

      if (m >= 0) {
        m++;

        gamept->result = RESULT_EMPTY;

        if (!strcmp(&gamept->title[m],"1-0")) {
          if (!gamept->orientation)
            gamept->result = RESULT_WIN;
          else
            gamept->result = RESULT_LOSS;
        }
        else if (!strcmp(&gamept->title[m],"0-1")) {
          if (gamept->orientation)
            gamept->result = RESULT_WIN;
          else
            gamept->result = RESULT_LOSS;
        }
        else if (!strcmp(&gamept->title[m],"1/2-1/2"))
          gamept->result = RESULT_DRAW;

        for (m -= 2; m >= 0; m--) {
          if (gamept->title[m] != ' ')
            break;
        }

        for (m -= 1; m >= 0; m--) {
          if (gamept->title[m] == ' ')
            break;
        }

        if (m >= 0) {
         for (p = m + 1, n = 0; (gamept->title[p] != ' ') && (n < MAX_ECO_LEN); p++,n++)
           gamept->eco[n] = gamept->title[p];

          gamept->eco[n] = 0;
        }

        for (m -= 2; m >= 0; m--) {
          if (gamept->title[m] != ' ')
            break;
        }

        for (m -= 1; m >= 0; m--) {
          if (gamept->title[m] == ' ')
            break;
        }

        if (m >= 0) {
         for (p = m + 1, n = 0; (gamept->title[p] != ' ') && (n < MAX_DATE_LEN); p++,n++)
           gamept->date[n] = gamept->title[p];

          gamept->date[n] = 0;
        }
      }

      continue;
    }

    if (gamept->curr_move & 0x1)
      direction = -1;           /* black's move */
    else
      direction = 1;            /* white's move */

    gamept->moves[gamept->curr_move].special_move_info = 0;

    switch(word[0]) {
      case 'O':
      case '0':
        retval = do_castle(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/

          got_error = 2;
        }

        break;

      case 'R':
      case 'N':
      case 'B':
      case 'Q':
      case 'K':
        retval = do_piece_move(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/

          got_error = 3;
        }

        break;

      default:
        retval = do_pawn_move(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/

          got_error = 4;
        }

        break;
    }

    if (got_error)
      break;

    update_board(gamept,NULL,NULL,false);

    gamept->curr_move++;
    gamept->moves[gamept->curr_move].special_move_info = 0;
    gamept->num_moves = gamept->curr_move;

    if (gamept->curr_move == dbg_move)
      dbg = 1;

    bBlack = gamept->curr_move & 0x1;

    if (debug_fptr && (debug_level == 3)) {
      fprintf(debug_fptr,"read_game: curr_move = %d\n",gamept->curr_move);
      fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
    }

    if (player_is_in_check(bBlack,gamept->board,gamept->curr_move)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_CHECK;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_CHECK\n",gamept->curr_move);
    }

    if (queen_is_attacked(bBlack,gamept->board,gamept->curr_move)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_QUEEN_IS_ATTACKED;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_QUEEN_IS_ATTACKED\n",gamept->curr_move);
    }

    if (bSetMateInOne && mate_in_one_exists(gamept)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_MATE_IN_ONE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_MATE_IN_ONE\n",gamept->curr_move);
    }
    else if (bSetStalemateInOne && stalemate_in_one_exists(gamept)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_STALEMATE_IN_ONE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_STALEMATE_IN_ONE\n",gamept->curr_move);
    }
  }

  fclose(fptr);

  legal_moves_count = 0;
  get_legal_moves(gamept,&legal_moves[0],&legal_moves_count);

  if (!legal_moves_count) {
    // determine if this is a checkmate or a stalemate
    if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_MATE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_MATE\n",gamept->curr_move);
    }
    else {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_STALEMATE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_STALEMATE\n",gamept->curr_move);
    }
  }

  if (got_error)
    return got_error;

  calculate_seirawan_counts(gamept);

  return 0;
}

int read_binary_game(char *filename,struct game *gamept)
{
  int filename_len;
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  filename_len = strlen(filename);

  if (filename_len > 3) {
    if (!strcmp(&filename[filename_len - 3],".ch"))
      return read_game(filename,gamept);
  }

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 1;

  bytes_to_read = sizeof (struct game) - (sizeof gamept->moves + sizeof gamept->board +
    sizeof gamept->white_pieces + sizeof gamept->black_pieces);

  bytes_read = read(fhndl,(char *)gamept,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 2;
  }

  bytes_to_read = gamept->num_moves * sizeof (struct move);

  if (bytes_to_read) {
    bytes_read = read(fhndl,(char *)gamept->moves,bytes_to_read);

    if (bytes_read != bytes_to_read) {
      close(fhndl);
      return 3;
    }
  }

  if (gamept->has_custom_initial_board) {
    bytes_to_read = CHARS_IN_BOARD;
    bytes_read = read(fhndl,(char *)custom_initial_board,bytes_to_read);

    if (bytes_read != bytes_to_read) {
      close(fhndl);
      return 4;
    }
  }

  close(fhndl);

  position_game(gamept,gamept->curr_move);

  return 0;
}

int read_game_position(char *filename,struct game_position *position_pt)
{
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 1;

  bytes_to_read = sizeof (struct game_position);

  bytes_read = read(fhndl,(char *)position_pt,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 2;
  }

  close(fhndl);

  return 0;
}

int write_binary_game(char *filename,struct game *gamept)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = sizeof (struct game) - (sizeof gamept->moves + sizeof gamept->board +
    sizeof gamept->white_pieces + sizeof gamept->black_pieces);

  bytes_written = write(fhndl,(char *)gamept,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  bytes_to_write = gamept->num_moves * sizeof (struct move);

  if (bytes_to_write) {
    bytes_written = write(fhndl,(char *)gamept->moves,bytes_to_write);

    if (bytes_written != bytes_to_write) {
      close(fhndl);
      return 3;
    }
  }

  if (gamept->has_custom_initial_board) {
    bytes_to_write = CHARS_IN_BOARD;
    bytes_written = write(fhndl,(char *)custom_initial_board,bytes_to_write);

    if (bytes_written != bytes_to_write) {
      close(fhndl);
      return 4;
    }
  }

  close(fhndl);

  return 0;
}

int write_game_position(char *filename,struct game_position *position_pt)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = sizeof (struct game_position);

  bytes_written = write(fhndl,(char *)position_pt,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  close(fhndl);

  return 0;
}

int write_board_comparison(char *filename,struct board_comparison *comparison_pt)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = sizeof (struct board_comparison);

  bytes_written = write(fhndl,(char *)comparison_pt,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  close(fhndl);

  return 0;
}

int ignore_character(int chara)
{
  if ((chara == 0x0d) ||
    (chara == '(') ||
    (chara == ')') ||
    (chara == 'x') ||
    (chara == '=') ||
    (chara == '+') ||
    (chara == '#'))
    return true;

  return false;
}

#define MAX_COMMENT_LEN 128
static char comment[MAX_COMMENT_LEN+1];
static int get_word_calls;

#define MAX_TMP_BUF_LEN 20
static char tmp_buf[MAX_TMP_BUF_LEN+1];

int get_datum(char *comment,char *datum_name,char *datum,int max_len)
{
  int n;
  char *cpt;

  if ((cpt = strstr(comment,datum_name))) {
    cpt += strlen(datum_name);

    for (n = 0; n < max_len; n++) {
      if (!cpt[n])
        break;

      datum[n] = cpt[n];
    }

    datum[n] = 0;

    return 1;
  }

  return 0;
}

int get_word(FILE *fptr,char *word,int maxlen,int *wordlenpt,struct game *gamept)
{
  int n;
  int chara;
  int started;
  bool bComment;
  int comment_ix;
  int end_of_file;
  int wordlen;
  char *cpt;

  wordlen = 0;
  started = 0;
  bComment = false;
  comment_ix = 0;
  end_of_file = 0;
  get_word_calls++;

  for ( ; ; ) {
    chara = fgetc(fptr);

    /* end of file ? */
    if (feof(fptr)) {
      end_of_file = 1;
      fclose(fptr);
      break;
    }

    // ignore carriage returns and other characters, except in the title or in a comment
    if (((wordlen >= 5) && !strncmp(word,"title",5)) || bComment)
      ;
    else if (ignore_character(chara))
      continue;

    /* end of line ? */
    if (chara == 0x0a) {
      if (started)
        break;
      else {
        comment[comment_ix] = 0;

        if (debug_fptr && (debug_level == 17))
          fprintf(debug_fptr,"get_word(): %d, comment: %s\n",get_word_calls,comment);

        if (get_datum(comment,"[Site \"",gamept->site,MAX_SITE_LEN)) {
          ;
        }
        else if (get_datum(comment,"[Termination \"",tmp_buf,MAX_TMP_BUF_LEN)) {
          if (strstr(tmp_buf,"Time forfeit"))
            gamept->time_forfeit = 1;
        }
        else if (get_datum(comment,"[WhiteElo \"",tmp_buf,MAX_TMP_BUF_LEN)) {
          if (!gamept->orientation)
            sscanf(tmp_buf,"%d",&gamept->my_elo_before);
          else
            sscanf(tmp_buf,"%d",&gamept->opponent_elo_before);
        }
        else if (get_datum(comment,"[BlackElo \"",tmp_buf,MAX_TMP_BUF_LEN)) {
          if (!gamept->orientation)
            sscanf(tmp_buf,"%d",&gamept->opponent_elo_before);
          else
            sscanf(tmp_buf,"%d",&gamept->my_elo_before);
        }
        else if (get_datum(comment,"[WhiteRatingDiff \"",tmp_buf,MAX_TMP_BUF_LEN)) {
          if (!gamept->orientation)
            sscanf(tmp_buf,"%d",&gamept->my_elo_delta);
          else
            sscanf(tmp_buf,"%d",&gamept->opponent_elo_delta);
        }
        else if (get_datum(comment,"[BlackRatingDiff \"",tmp_buf,MAX_TMP_BUF_LEN)) {
          if (!gamept->orientation)
            sscanf(tmp_buf,"%d",&gamept->opponent_elo_delta);
          else
            sscanf(tmp_buf,"%d",&gamept->my_elo_delta);
        }

        bComment = false;
        comment_ix = 0;

        continue;
      }
    }

    /* in comment ? */
    if (bComment) {
      if (comment_ix < MAX_COMMENT_LEN - 1)
        comment[comment_ix++] = chara;

      continue;
    }

    /* comment marker ? */
    if (!wordlen && (chara == '/')) {
      bComment = true;
      continue;
    }

    /* white space ? */
    if ((chara == 0x09) || (chara == ' '))
      if (started)
        break;
      else
        continue;

    if (!(started))
      started = 1;

    if (wordlen < maxlen - 1)
      word[wordlen++] = chara;
  }

  word[wordlen] = 0;
  *wordlenpt = wordlen;

  if (debug_fptr && (debug_level == 17)) {
    if (!end_of_file)
      fprintf(debug_fptr,"get_word(): %d, word: %s\n",get_word_calls,word);
  }

  return end_of_file;
}

static int update_board_calls;
static int dbg_update_board_call;
static int dbg;

void update_board(struct game *gamept,int *invalid_squares,int *num_invalid_squares,bool bScratch)
{
  int n;
  bool bBlack;
  int from_piece;
  int to_piece;
  bool bKingsideCastle = false;
  bool bQueensideCastle = false;
  bool bEnPassantCapture = false;
  int square_to_clear;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  update_board_calls++;

  if (dbg_update_board_call == update_board_calls)
    dbg = 1;

  bBlack = (gamept->curr_move % 2);

  from_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].from);
  to_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].to);

  if (!bScratch && (from_piece * to_piece < 0))
    gamept->moves[gamept->curr_move].special_move_info |= SPECIAL_MOVE_CAPTURE;

  if (debug_fptr && (debug_level == 15)) {
    fprintf(debug_fptr,"update_board (%d): curr_move = %d, special_move_info = %x\n",
      update_board_calls,gamept->curr_move,gamept->moves[gamept->curr_move].special_move_info);
  }

  if (!bInGetLegalMoves) {
    if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      bKingsideCastle = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE)
      bQueensideCastle = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)
      bEnPassantCapture = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_QUEEN)
      from_piece = (bBlack ? QUEEN_ID * -1 : QUEEN_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
      from_piece = (bBlack ? ROOK_ID * -1 : ROOK_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
      from_piece = (bBlack ? BISHOP_ID * -1 : BISHOP_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
      from_piece = (bBlack ? KNIGHT_ID * -1 : KNIGHT_ID);
  }

  if (invalid_squares) {
    *num_invalid_squares = 0;
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].from;
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].to;
  }

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].to,from_piece);

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].from,0);  /* vacate previous square */

  if (bKingsideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,5,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 5;

      set_piece1(gamept->board,7,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 7;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,61,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 61;

      set_piece1(gamept->board,63,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 63;
    }
  }
  else if (bQueensideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,3,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 3;

      set_piece1(gamept->board,0,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 0;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,59,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 59;

      set_piece1(gamept->board,56,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 56;
    }
  }
  else if (bEnPassantCapture) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      square_to_clear = gamept->moves[gamept->curr_move].to - NUM_FILES;
    }
    else {
      // it's Blacks's move
      square_to_clear = gamept->moves[gamept->curr_move].to + NUM_FILES;
    }

    set_piece1(gamept->board,square_to_clear,0);

    if (invalid_squares)
      invalid_squares[(*num_invalid_squares)++] = square_to_clear;
  }

  if (debug_fptr && (debug_level == 9)) {
    if (invalid_squares) {
      for (n = 0; n < *num_invalid_squares; n++) {
        fprintf(debug_fptr,"update_board (%d): invalid_squares[%d] = %d\n",
          update_board_calls,n,invalid_squares[n]);
      }
    }

    if (!bScratch)
      fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
  }

  update_piece_info(gamept);
}

void update_piece_info(struct game *gamept)
{
  int n;
  char from;
  char to;
  int special_move_info;
  int debug;
  unsigned char board[CHARS_IN_BOARD];

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  from = gamept->moves[gamept->curr_move].from;
  to = gamept->moves[gamept->curr_move].to;
  special_move_info = gamept->moves[gamept->curr_move].special_move_info;

  if (!(gamept->curr_move % 2)) {
    // it's White's move
    if (special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      gamept->white_pieces[4].current_board_position = 6;
      gamept->white_pieces[4].move_count++;
      gamept->white_pieces[7].current_board_position = 5;
      gamept->white_pieces[7].move_count++;
    }
    else if (special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE) {
      gamept->white_pieces[4].current_board_position = 2;
      gamept->white_pieces[4].move_count++;
      gamept->white_pieces[0].current_board_position = 3;
      gamept->white_pieces[0].move_count++;
    }
    else {
      for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
        if (gamept->white_pieces[n].current_board_position == from)
          break;
      }

      if (n == NUM_PIECES_PER_PLAYER)
        return; // should never happen

      gamept->white_pieces[n].current_board_position = to;
      gamept->white_pieces[n].move_count++;

      if (special_move_info & SPECIAL_MOVE_PROMOTION_QUEEN)
        gamept->white_pieces[n].piece_id = QUEEN_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
        gamept->white_pieces[n].piece_id = ROOK_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
        gamept->white_pieces[n].piece_id = KNIGHT_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
        gamept->white_pieces[n].piece_id = BISHOP_ID;
      else if (special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->black_pieces[n].current_board_position == to - NUM_FILES)
            break;
        }

        if (n == NUM_PIECES_PER_PLAYER)
          return; // should never happen

        gamept->black_pieces[n].current_board_position = -1;
      }

      if (!(special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->black_pieces[n].current_board_position == to)
            break;
        }

        if (n < NUM_PIECES_PER_PLAYER)
          gamept->black_pieces[n].current_board_position = -1;
      }
    }
  }
  else {
    // it's Blacks's move
    if (special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      gamept->black_pieces[12].current_board_position = 62;
      gamept->black_pieces[12].move_count++;
      gamept->black_pieces[15].current_board_position = 61;
      gamept->black_pieces[15].move_count++;
    }
    else if (special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE) {
      gamept->black_pieces[12].current_board_position = 58;
      gamept->black_pieces[12].move_count++;
      gamept->black_pieces[8].current_board_position = 59;
      gamept->black_pieces[8].move_count++;
    }
    else {
      for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
        if (gamept->black_pieces[n].current_board_position == from)
          break;
      }

      if (n == NUM_PIECES_PER_PLAYER)
        return; // should never happen

      gamept->black_pieces[n].current_board_position = to;
      gamept->black_pieces[n].move_count++;

      if (special_move_info & SPECIAL_MOVE_PROMOTION_QUEEN)
        gamept->black_pieces[n].piece_id = QUEEN_ID* -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
        gamept->black_pieces[n].piece_id = ROOK_ID* -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
        gamept->black_pieces[n].piece_id = KNIGHT_ID* -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
        gamept->black_pieces[n].piece_id = BISHOP_ID* -1;
      else if (special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->white_pieces[n].current_board_position == to + NUM_FILES)
            break;
        }

        if (n == NUM_PIECES_PER_PLAYER)
          return; // should never happen

        gamept->white_pieces[n].current_board_position = -1;
      }

      if (!(special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->white_pieces[n].current_board_position == to)
            break;
        }

        if (n < NUM_PIECES_PER_PLAYER)
          gamept->white_pieces[n].current_board_position = -1;
      }
    }
  }

  if (debug_fptr && (debug_level == 4)) {
    fprintf(debug_fptr,"update_piece_info: curr_move = %d, num_moves = %d\n",gamept->curr_move,gamept->num_moves);
    fprint_piece_info(gamept,debug_fptr);
    populate_board_from_piece_info(gamept->white_pieces,gamept->black_pieces,board);
    fprint_bd3(board,gamept->orientation,debug_fptr);
  }
}

void fprint_piece_info(struct game *gamept,FILE *fptr)
{
  int n;

  fprintf(fptr,"White:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->white_pieces[n].current_board_position == -1) {
      fprintf(fptr,"  %s %d %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        gamept->white_pieces[n].current_board_position,
        gamept->white_pieces[n].move_count);
    }
    else {
      fprintf(fptr,"  %s %c%c %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].current_board_position),
        gamept->white_pieces[n].move_count);
    }
  }

  fprintf(fptr,"Black:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->black_pieces[n].current_board_position == -1) {
      fprintf(fptr,"  %s %d %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        gamept->black_pieces[n].current_board_position,
        gamept->black_pieces[n].move_count);
    }
    else {
      fprintf(fptr,"  %s %c%c %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].current_board_position),
        gamept->black_pieces[n].move_count);
    }
  }
}

void print_piece_info(struct game *gamept)
{
  int n;

  printf("White:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->white_pieces[n].current_board_position == -1) {
      printf("  %s %d %d %s %c%c\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        gamept->white_pieces[n].current_board_position,
        gamept->white_pieces[n].move_count,
        piece_names[gamept->white_pieces[n].original_piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].original_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].original_board_position));
    }
    else {
      printf("  %s %c%c %d %s %c%c\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].current_board_position),
        gamept->white_pieces[n].move_count,
        piece_names[gamept->white_pieces[n].original_piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].original_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].original_board_position));
    }
  }

  printf("Black:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->black_pieces[n].current_board_position == -1) {
      printf("  %s %d %d %s %c%c\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        gamept->black_pieces[n].current_board_position,
        gamept->black_pieces[n].move_count,
        piece_names[(gamept->black_pieces[n].original_piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].original_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].original_board_position));
    }
    else {
      printf("  %s %c%c %d %s %c%c\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].current_board_position),
        gamept->black_pieces[n].move_count,
        piece_names[(gamept->black_pieces[n].original_piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].original_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].original_board_position));
    }
  }
}

void print_piece_info2(struct piece_info *info_pt,bool bWhite,bool bAbbrev,bool bOnlyRemaining)
{
  int n;
  char piece_id;
  char original_piece_id;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    piece_id = info_pt[n].piece_id;

    if (piece_id < 0)
      piece_id *= -1;

    original_piece_id = info_pt[n].original_piece_id;

    if (original_piece_id < 0)
      original_piece_id *= -1;

    if (info_pt[n].current_board_position == -1) {
      if (!bOnlyRemaining) {
        if (!bAbbrev) {
          printf("  %s %d %d %s %d\n",
            piece_names[piece_id - 1],
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_names[original_piece_id - 1],
            info_pt[n].original_board_position);
        }
        else if (bWhite) {
          printf("  %c %d %d %c %d\n",
            piece_ids2[piece_id - 1] + ('a' - 'A'),
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_ids2[original_piece_id - 1] + ('a' - 'A'),
            info_pt[n].original_board_position);
        }
        else {
          printf("  %c %d %d %c %d\n",
            piece_ids2[piece_id - 1],
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_ids2[original_piece_id - 1],
            info_pt[n].original_board_position);
        }
      }
    }
    else {
      if (!bAbbrev) {
        printf("  %s %c%c %d %s %c%c\n",
          piece_names[piece_id - 1],
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_names[original_piece_id - 1],
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
      else if (bWhite) {
        printf("  %c %c%c %d %c %c%c\n",
          piece_ids2[piece_id - 1] + ('a' - 'A'),
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_ids2[original_piece_id - 1] + ('a' - 'A'),
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
      else {
        printf("  %c %c%c %d %c %c%c\n",
          piece_ids2[piece_id - 1],
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_ids2[original_piece_id - 1],
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
    }
  }
}

void fprint_piece_info2(FILE *fptr,struct piece_info *info_pt,bool bWhite,bool bAbbrev,bool bOnlyRemaining)
{
  int n;
  char piece_id;
  char original_piece_id;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    piece_id = info_pt[n].piece_id;

    if (piece_id < 0)
      piece_id *= -1;

    original_piece_id = info_pt[n].original_piece_id;

    if (original_piece_id < 0)
      original_piece_id *= -1;

    if (info_pt[n].current_board_position == -1) {
      if (!bOnlyRemaining) {
        if (!bAbbrev) {
          fprintf(fptr,"  %s %d %d %s %d\n",
            piece_names[piece_id - 1],
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_names[original_piece_id - 1],
            info_pt[n].original_board_position);
        }
        else if (bWhite) {
          fprintf(fptr,"  %c %d %d %c %d\n",
            piece_ids2[piece_id - 1] + ('a' - 'A'),
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_ids2[original_piece_id - 1] + ('a' - 'A'),
            info_pt[n].original_board_position);
        }
        else {
          fprintf(fptr,"  %c %d %d %c %d\n",
            piece_ids2[piece_id - 1],
            info_pt[n].current_board_position,
            info_pt[n].move_count,
            piece_ids2[original_piece_id - 1],
            info_pt[n].original_board_position);
        }
      }
    }
    else {
      if (!bAbbrev) {
        fprintf(fptr,"  %s %c%c %d %s %c%c\n",
          piece_names[piece_id - 1],
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_names[original_piece_id - 1],
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
      else if (bWhite) {
        fprintf(fptr,"  %c %c%c %d %c %c%c\n",
          piece_ids2[piece_id - 1] + ('a' - 'A'),
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_ids2[original_piece_id - 1] + ('a' - 'A'),
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
      else {
        fprintf(fptr,"  %c %c%c %d %c %c%c\n",
          piece_ids2[piece_id - 1],
          'a' + FILE_OF(info_pt[n].current_board_position),
          '1' + RANK_OF(info_pt[n].current_board_position),
          info_pt[n].move_count,
          piece_ids2[original_piece_id - 1],
          'a' + FILE_OF(info_pt[n].original_board_position),
          '1' + RANK_OF(info_pt[n].original_board_position));
      }
    }
  }
}

void populate_board_from_piece_info(struct piece_info *white_pt,struct piece_info *black_pt,unsigned char *board)
{
  int n;
  unsigned int bit_offset;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    board[n] = 0;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (white_pt[n].current_board_position != -1) {
      bit_offset = white_pt[n].current_board_position * BITS_PER_BOARD_SQUARE;
      set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,white_pt[n].piece_id);
    }

    if (black_pt[n].current_board_position != -1) {
      bit_offset = black_pt[n].current_board_position * BITS_PER_BOARD_SQUARE;
      set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,black_pt[n].piece_id);
    }
  }
}

int populate_piece_info_from_board(unsigned char *board,struct piece_info *white_pt,struct piece_info *black_pt)
{
  int m;
  int n;
  int piece;
  struct piece_info *info_pt;
  static int got_here;

  got_here++;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    white_pt[n].piece_id = EMPTY_ID;
    white_pt[n].current_board_position = -1;
    white_pt[n].move_count = 0;
    black_pt[n].piece_id = EMPTY_ID;
    black_pt[n].current_board_position = -1;
    black_pt[n].move_count = 0;
  }

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (!piece)
      continue;

    if (piece < 0)
      info_pt = black_pt;
    else
      info_pt = white_pt;

    for (m = 0; m < NUM_PIECES_PER_PLAYER; m++) {
      if (info_pt[m].piece_id != EMPTY_ID)
        continue;

      info_pt[m].piece_id = piece;
      info_pt[m].current_board_position = n;
      info_pt[m].original_piece_id = piece;
      info_pt[m].original_board_position = n;

      break;
    }

    if (m == NUM_PIECES_PER_PLAYER)
      return 1;
  }

  return 0;
}

int compare_boards(unsigned char *board1,unsigned char *board2)
{
  int n;

  for (n = 0; n < CHARS_IN_BOARD; n++) {
    if (board1[n] != board2[n])
      return 0;
  }

  return 1;
}

bool board_is_mirrored(struct game *gamept)
{
  int m;
  int n;
  int square1;
  int square2;
  int dbg;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  for (m = 0; m < (NUM_RANKS / 2); m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square1 = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      square2 = get_piece2(gamept->board,m,n);

      if (square1 + square2)
        return false;
    }
  }

  return true;
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

int get_piece2(unsigned char *board,int row,int column)
{
  int board_offset;

  board_offset = row * 8 + column;
  return get_piece1(board,board_offset);
}

void set_piece1(unsigned char *board,int board_offset,int piece)
{
  unsigned int bit_offset;
  int dbg;

  if (board_offset < 0)
    dbg = 1;

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;
  set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,piece);
}

void set_piece2(unsigned char *board,int row,int column,int piece)
{
  int board_offset;

  board_offset = row * 8 + column;
  set_piece1(board,board_offset,piece);
}

void copy_game(struct game *gamept_to,struct game *gamept_from)
{
  memcpy(gamept_to,gamept_from,sizeof (struct game));
}

void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}

static int fen2pos(char *line,int line_len,unsigned char *pos_buf,int *black_to_play)
{
  int m;
  int n;
  int p;
  int forward_slash_count;
  int rank_count;
  int chara;
  int contig_square_no_piece_count;
  int piece;

  m = 0;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    pos_buf[n] = 0;

  forward_slash_count = 0;
  rank_count = 0;

  *black_to_play = -1;

  for (n = 0; n < line_len; n++) {
    chara = line[n];

    if (chara == ' ') {
      forward_slash_count++;

      continue;
    }

    if (forward_slash_count == 8) {
      if (chara == 'w') {
        *black_to_play = 0;
        break;
      }
      if (chara == 'b') {
        *black_to_play = 1;
        break;
      }
    }

    if (chara == '/') {
      forward_slash_count++;

      if (rank_count != 8)
        return 1;

      rank_count = 0;
    }
    else if ((chara >= '1') && (chara <= '8'))
      rank_count += chara - '1' + 1;
    else {
      if ((chara == 'P') ||
          (chara == 'R') ||
          (chara == 'N') ||
          (chara == 'B') ||
          (chara == 'Q') ||
          (chara == 'K') ||
          (chara == 'p') ||
          (chara == 'r') ||
          (chara == 'n') ||
          (chara == 'b') ||
          (chara == 'q') ||
          (chara == 'k'))
        rank_count++;
      else
        return 2;
    }
  }

  if (*black_to_play == -1)
    return 3;

  p = 0;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      chara = line[p++];

      if ((chara >= '1') && (chara <= '8')) {
        contig_square_no_piece_count = chara - '1' + 1;
        n += contig_square_no_piece_count - 1;
      }
      else {
        if (chara == 'P')
          piece = PAWN_ID;
        else if (chara == 'R')
          piece = ROOK_ID;
        else if (chara == 'N')
          piece = KNIGHT_ID;
        else if (chara == 'B')
          piece = BISHOP_ID;
        else if (chara == 'Q')
          piece = QUEEN_ID;
        else if (chara == 'K')
          piece = KING_ID;
        else if (chara == 'p')
          piece = PAWN_ID * -1;
        else if (chara == 'r')
          piece = ROOK_ID * -1;
        else if (chara == 'n')
          piece = KNIGHT_ID * -1;
        else if (chara == 'b')
          piece = BISHOP_ID * -1;
        else if (chara == 'q')
          piece = QUEEN_ID * -1;
        else
          piece = KING_ID * -1;

        set_bits(4,pos_buf,((7 - m) * 8 * 4) + (n * 4),piece);
      }
    }

    if (m < 7)
      p++;
  }

  return 0;
}

#define MAX_LINE_LEN 256
static char line[MAX_LINE_LEN];

int populate_board_from_board_file(unsigned char *board,char *filename,int orientation)
{
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int chara;
  int piece;

  if ((fptr = fopen(filename,"r")) == NULL)
    return 1;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (line_len != 15)
      return 2;

    for (n = 0; n < NUM_FILES; n++) {
      chara = line[n*2];

      if (chara == '.') {
        piece = 0;

        if (!orientation)
          set_piece2(board,7 - line_no,n,piece);
        else
          set_piece2(board,line_no,7 - n,piece);
      }
      else {
        if (chara == 'p') {
          if (!orientation)
            set_piece2(board,7 - line_no,n,PAWN_ID);
          else
            set_piece2(board,line_no,7 - n,PAWN_ID);
        }
        else if (chara == 'P') {
          if (!orientation)
            set_piece2(board,7 - line_no,n,PAWN_ID * -1);
          else
            set_piece2(board,line_no,7 - n,PAWN_ID * -1);
        }
        else if (chara == 'e') {
          if (!orientation)
            set_piece2(board,7 - line_no,n,EMPTY_ID);
          else
            set_piece2(board,line_no,7 - n,EMPTY_ID);
        }
        else {
          for (m = 0; m < NUM_PIECE_TYPES; m++) {
            if (chara == piece_ids[m]) {
              piece = (m + 2) * -1;
              break;
            }
            else if (chara == piece_ids[m] - 'A' + 'a') {
              piece = (m + 2);
              break;
            }
          }

          if (m < NUM_PIECE_TYPES) {
            if (!orientation)
              set_piece2(board,7 - line_no,n,piece);
            else
              set_piece2(board,line_no,7 - n,piece);
          }
        }
      }
    }

    line_no++;
  }

  fclose(fptr);

  return 0;
}

int populate_initial_board_from_board_file(char *filename)
{
  return populate_board_from_board_file(custom_initial_board,filename,0);
}

int populate_board_from_bin_board_file(unsigned char *board,char *filename)
{
  struct stat stat_buf;
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if (stat(filename,&stat_buf) == -1)
    return 1;

  if (stat_buf.st_size != CHARS_IN_BOARD)
    return 2;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 3;

  bytes_to_read = CHARS_IN_BOARD;

  bytes_read = read(fhndl,(char *)board,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 4;
  }

  close(fhndl);

  return 0;
}

int populate_piece_counts_from_piece_count_file(int *piece_counts,char *filename)
{
  struct stat stat_buf;
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if (stat(filename,&stat_buf) == -1)
    return 1;

  if (stat_buf.st_size != (NUM_PIECE_TYPES_0 * 2) * sizeof (int))
    return 2;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 3;

  bytes_to_read = (NUM_PIECE_TYPES_0 * 2) * sizeof (int);

  bytes_read = read(fhndl,(char *)piece_counts,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 4;
  }

  close(fhndl);

  return 0;
}

int populate_initial_board_from_bin_board_file(char *filename)
{
  return populate_board_from_bin_board_file(custom_initial_board,filename);
}

int write_board_to_binfile(unsigned char *board,char *filename)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = CHARS_IN_BOARD;

  bytes_written = write(fhndl,(char *)board,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  close(fhndl);

  return 0;
}

int count_num_pieces(int color,struct game *gamept)
{
  int n;
  short piece;
  int count;

  count = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(gamept->board,n);

    if (!piece)
      continue;

    if (color == WHITE) {
      if (piece > 0)
        count++;
    }
    else {
      if (piece < 0)
        count++;
    }
  }

  return count;
}

void get_piece_counts(unsigned char *board,int *piece_counts)
{
  int n;
  short piece;

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    piece_counts[n] = 0;
  }

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (!piece)
      continue;

    if (piece > 0) {
      piece_counts[piece - 1]++;
    }
    else {
      piece *= -1;
      piece_counts[NUM_PIECE_TYPES_0 + piece - 1]++;
    }
  }
}

int piece_counts_match(int *piece_counts,int *match_piece_counts,bool bExactMatch)
{
  int n;

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    if (match_piece_counts[n] == -1) {
      if (piece_counts[n])
        return 0;
      else
        continue;
    }

    if (bExactMatch) {
      if (piece_counts[n] != match_piece_counts[n])
        return 0;
    }
    else {
      if (match_piece_counts[n] && (piece_counts[n] != match_piece_counts[n]))
        return 0;
    }
  }

  return 1;
}

void print_piece_counts(int *piece_counts)
{
  int n;
  int square;

  putchar(0x0a);

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    if (!n)
      square = 1;
    else if (n == NUM_PIECE_TYPES_0)
      square = -1;

    if (n < (NUM_PIECE_TYPES_0 * 2) - 1) {
      printf("%c %d ",format_square(square),piece_counts[n]);

      if (square > 0)
        square++;
      else
        square--;
    }
    else
      printf("%c %d\n",format_square(square),piece_counts[n]);
  }
}

void print_move_counts(struct game *gamept)
{
  int n;

  printf("white piece move counts: ");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (n < NUM_PIECES_PER_PLAYER - 1)
      printf("%d ",gamept->white_pieces[n].move_count);
    else
      printf("%d\n",gamept->white_pieces[n].move_count);
  }

  printf("black piece move counts: ");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (n < NUM_PIECES_PER_PLAYER - 1)
      printf("%d ",gamept->black_pieces[n].move_count);
    else
      printf("%d\n",gamept->black_pieces[n].move_count);
  }
}

void calculate_seirawan_counts(struct game *gamept)
{
  int m;
  int n;
  int o;
  int direction;
  int piece;
  int low;
  int high;

  for (m = 0; m < 2; m++) {
    seirawan_count[m] = 0;

    if (!m)
      direction = 1;
    else
      direction = -1;

    for (n = 0; n < NUM_BOARD_SQUARES; n++) {
      piece = get_piece1(gamept->board,n);

      if (piece * direction > 0) {
        if (piece < 0) {
          low = 0;
          high = 32;
        }
        else {
          low = 32;
          high = 64;
        }

        for (o = low; o < high; o++) {
          if (square_attacks_square(gamept->board,n,o))
            seirawan_count[m]++;
        }
      }
    }
  }

  calculate_force_counts(gamept);

  if (debug_fptr && (debug_level == 16)) {
    fprintf(debug_fptr,"calculate_seirawan_counts: board:\n");
    fprint_bd2(gamept->board,debug_fptr);
    fprintf(debug_fptr,"calculate_seirawan_counts: White: %d Black: %d\n",
      seirawan_count[0],seirawan_count[1]);
  }
}

void calculate_force_counts(struct game *gamept)
{
  int n;
  short piece;

  force_count[BLACK] = 0;
  force_count[WHITE] = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(gamept->board,n);

    if (!piece)
      continue;

    if (piece < 0)
      force_count[BLACK] += force_value_of(piece);
    else
      force_count[WHITE] += force_value_of(piece);
  }
}
