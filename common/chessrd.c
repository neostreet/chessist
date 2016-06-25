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

static unsigned char initial_board[] = {
  (char)0x23, (char)0x45, (char)0x64, (char)0x32,
  (char)0x11, (char)0x11, (char)0x11, (char)0x11,
  (char)0x00, (char)0x00, (char)0x00, (char)0x00,
  (char)0x00, (char)0x00, (char)0x00, (char)0x00,
  (char)0x00, (char)0x00, (char)0x00, (char)0x00,
  (char)0x00, (char)0x00, (char)0x00, (char)0x00,
  (char)0xff, (char)0xff, (char)0xff, (char)0xff,
  (char)0xed, (char)0xcb, (char)0xac, (char)0xde
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

extern char piece_ids[]; /* "RNBQK" */

static char *bad_piece_move[] = {
  "bad rook move",
  "bad knight move",
  "bad bishop move",
  "bad queen move",
  "bad king move"
};

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);
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

      return FALSE;
    }
  }

  return TRUE;
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
  short piece;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    gamept->board[n] = initial_board[n];

  for (n = 0; n < 2; n++) {
    gamept->seirawan_count[n] = 0;
    gamept->force_count[n] = 0;
  }

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(gamept,n);

    if (!piece)
      continue;

    if (piece < 0)
      gamept->force_count[BLACK] += force_value_of(piece);
    else
      gamept->force_count[WHITE] += force_value_of(piece);
  }
}

int read_game(char *filename,struct game *gamept,char *err_msg)
{
  FILE *fptr;
  int chara;
  int end_of_file;
  int m;
  int n;
  char word[WORDLEN];
  int wordlen;
  int bHaveFirstWord;
  int direction;
  int word_no;
  int dbg;
  int retval;
  int got_error;

  if ((fptr = fopen(filename,"r")) == NULL)
    return 1;

  strcpy(gamept->chessfilename,filename);

  fscanf(fptr,"%d",&gamept->orientation);  /* get board orientation */
                                   /* 0 = standard, 1 = black on bottom */

  end_of_file = get_word(fptr,word,WORDLEN,&wordlen);

  if (!strncmp(word,"FEN:",4)) {
    retval = read_fen(fptr,gamept);

    if (retval) {
      printf("read_fen() failed: %d\n",retval);
      return 3;
    }

    bHaveFirstWord = FALSE;
  }
  else
    bHaveFirstWord = TRUE;

  set_initial_board(gamept);

  end_of_file = 0;
  gamept->curr_move = 0;

  gamept->title[0] = 0;
  word_no = 0;
  got_error = 0;

  for ( ; ; ) {
    if (word_no || !bHaveFirstWord)
      end_of_file = get_word(fptr,word,WORDLEN,&wordlen);

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

        if (n < 79)
          if (chara == '\\')
            gamept->title[n++] = ' ';
          else
            gamept->title[n++] = chara;
      }

      gamept->title[n] = 0;

      continue;
    }

    if (gamept->curr_move & 0x1)
      direction = -1;           /* black's move */
    else
      direction = 1;            /* white's move */

    switch(word[0]) {
      case 'O':
      case '0':
        retval = do_castle(gamept,direction,word,wordlen);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_castle);

          got_error = 1;
        }

        break;

      case 'R':
      case 'N':
      case 'B':
      case 'Q':
      case 'K':
        retval = do_piece_move(gamept,direction,word,wordlen);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_piece_move[get_piece_type_ix(word[0])]);

          got_error = 1;
        }

        break;

      default:
        retval = do_pawn_move(gamept,direction,word,wordlen);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_pawn_move);

          got_error = 1;
        }

        break;
    }

    if (got_error)
      break;

    update_board(gamept,FALSE);
    gamept->curr_move++;
  }

  gamept->num_moves = gamept->curr_move;
  calculate_seirawan_counts(gamept);

  return 0;
}

int read_binary_game(char *filename,struct game *gamept)
{
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 1;

  bytes_to_read = sizeof (struct game) - sizeof gamept->moves;

  bytes_read = read(fhndl,(char *)gamept,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 2;
  }

  bytes_to_read = gamept->num_moves * sizeof (struct move);

  bytes_read = read(fhndl,(char *)gamept->moves,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 3;
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

  bytes_to_write = sizeof (struct game) - sizeof (struct move *);

  bytes_written = write(fhndl,(char *)gamept,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  bytes_to_write = gamept->num_moves * sizeof (struct move);

  bytes_written = write(fhndl,(char *)gamept->moves,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 3;
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
    (chara == '+'))
    return TRUE;

  return FALSE;
}

int get_word(FILE *fptr,char *word,int maxlen,int *wordlenpt)
{
  int chara;
  int started;
  int comment;
  int end_of_file;
  int wordlen;

  wordlen = 0;
  started = 0;
  comment = 0;
  end_of_file = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    /* end of file ? */
    if (feof(fptr)) {
      end_of_file = 1;
      fclose(fptr);
      break;
    }

    // ignore carriage returns and other characters
    if (ignore_character(chara))
      continue;

    /* end of line ? */
    if (chara == 0x0a) {
      if (started)
        break;
      else {
        comment = 0;
        continue;
      }
    }

    /* in comment ? */
    if (comment)
      continue;

    /* comment marker ? */
    if (chara == '/') {
      comment = 1;
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

  return end_of_file;
}

void update_board(struct game *gamept,short bCalcCounts)
{
  set_piece1(gamept,gamept->moves[gamept->curr_move].to,
    get_piece1(gamept,gamept->moves[gamept->curr_move].from));

  set_piece1(gamept,gamept->moves[gamept->curr_move].from,0);  /* vacate previous square */

  if (bCalcCounts)
    calculate_seirawan_counts(gamept);
}

int get_piece1(struct game *gamept,int board_offset)
{
  unsigned int bit_offset;
  unsigned short piece;
  int piece_int;

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;

  piece = get_bits(BITS_PER_BOARD_SQUARE,gamept->board,bit_offset);
  piece_int = piece;

  if (piece & 0x8)
    piece_int |= 0xfffffff0;

  return piece_int;
}

int get_piece2(struct game *gamept,int row,int column)
{
  int board_offset;

  board_offset = row * 8 + column;
  return get_piece1(gamept,board_offset);
}

void set_piece1(struct game *gamept,int board_offset,int piece)
{
  unsigned int bit_offset;

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;
  set_bits(BITS_PER_BOARD_SQUARE,gamept->board,bit_offset,piece);
}

void set_piece2(struct game *gamept,int row,int column,int piece)
{
  int board_offset;

  board_offset = row * 8 + column;
  set_piece1(gamept,board_offset,piece);
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
    gamept->seirawan_count[m] = 0;

    if (!m)
      direction = 1;
    else
      direction = -1;

    for (n = 0; n < NUM_BOARD_SQUARES; n++) {
      piece = get_piece1(gamept,n);

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
          if (square_attacks_square(gamept,n,o))
            gamept->seirawan_count[m]++;
        }
      }
    }
  }
}

#define MAX_FEN_LINE_LEN 256
static char fen_line[MAX_FEN_LINE_LEN];

int read_fen(FILE *fptr,struct game *gamept)
{
  int fen_line_len;
  int retval;

  GetLine(fptr,fen_line,&fen_line_len,MAX_FEN_LINE_LEN);

  retval = fen2pos(fen_line,fen_line_len,initial_board,
    &gamept->black_to_play);

  if (retval)
    return retval;

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
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
