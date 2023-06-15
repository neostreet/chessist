#define FALSE 0
#define TRUE  1

#define WHITE 0
#define BLACK 1
#define NUM_PLAYERS 2

#define NUM_RANKS 8
#define NUM_FILES 8

#define NUM_BOARD_SQUARES (NUM_RANKS * NUM_FILES)

#define CHARS_IN_BOARD \
(NUM_BOARD_SQUARES / 2)  // 64 squares / 2 (nibbles per char)

#define PAWN_ID       1
#define ROOK_ID       2
#define KNIGHT_ID     3
#define BISHOP_ID     4
#define QUEEN_ID      5
#define KING_ID       6

#define SPECIAL_MOVE_KINGSIDE_CASTLE  201
#define SPECIAL_MOVE_QUEENSIDE_CASTLE 202
#define SPECIAL_MOVE_CAPTURE          203
#define SPECIAL_MOVE_EN_PASSANT       204
#define SPECIAL_MOVE_PROMOTION_QUEEN  205
#define SPECIAL_MOVE_PROMOTION_ROOK   206
#define SPECIAL_MOVE_PROMOTION_KNIGHT 207
#define SPECIAL_MOVE_PROMOTION_BISHOP 208

#define WORDLEN 80
#define MAX_MOVES 400

#define WIDTH_IN_PIXELS 50
#define HEIGHT_IN_PIXELS 50

#define SHRUNK_WIDTH_IN_PIXELS (WIDTH_IN_PIXELS / 2)
#define SHRUNK_HEIGHT_IN_PIXELS (HEIGHT_IN_PIXELS / 2)

#define BOARD_WIDTH (NUM_FILES * width_in_pixels)
#define BOARD_HEIGHT (NUM_RANKS * height_in_pixels)

#define NUM_PIECE_TYPES 5

struct move {
  char from;
  char to;
  int special_move_info;
};

#define BITS_PER_BOARD_SQUARE 4

#define FONT_HEIGHT 12
#define FONT_WIDTH 9

#define Y_PIXELS 200

#define MAX_ANNOTATION_LINE_LEN 25
#define MAX_ANNOTATION_LINES (Y_PIXELS / FONT_HEIGHT)

#define ANNOTATION_X (8 * XLEN + 2 + FONT_WIDTH)
#define ANNOTATION_Y 5

#define MAX_FILE_NAME_LEN 256

struct game {
  char chessfilename[MAX_FILE_NAME_LEN];
  char title[80];
  int orientation;
  int num_moves;
  int curr_move;
  int bBig;
  int highlight_rank;
  int highlight_file;
  int move_start_square;
  int move_start_square_piece;
  int move_end_square;
  int move_end_square_piece;
  unsigned char board[CHARS_IN_BOARD];  /* 10 columns * 10 rows / 2 (nibbles per char) */
  int debug_level;
  FILE *debug_fptr;
  struct move moves[MAX_MOVES];
};

typedef char (*CHESS_FILE_LIST)[MAX_FILE_NAME_LEN];
