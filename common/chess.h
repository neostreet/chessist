#ifdef MAKE_GLOBALS_HERE
#define GLOBAL /* nothing = real object declaration */
#else
#define GLOBAL extern
#endif

GLOBAL char err_msg[80];

GLOBAL int dbg_move;
GLOBAL int debug_level;
GLOBAL FILE *debug_fptr;

GLOBAL int bBig;

GLOBAL int highlight_rank;
GLOBAL int highlight_file;

GLOBAL int move_start_square;
GLOBAL int move_start_square_piece;
GLOBAL int move_end_square;
GLOBAL int move_end_square_piece;

GLOBAL int do_castle_failures;
GLOBAL int do_castle_successes;
GLOBAL int seed;
GLOBAL bool bBinaryFormat;

#ifdef MAKE_GLOBALS_HERE
char piece_ids[] = "RNBQK";
char *piece_names[] = {
  "PAWN",
  "ROOK",
  "KNIGHT",
  "BISHOP",
  "QUEEN",
  "KING"
};

char fmt_str[] = "%s\n";
char *special_moves[] = {
  "SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE",
  "SPECIAL_MOVE_KINGSIDE_CASTLE",
  "SPECIAL_MOVE_QUEENSIDE_CASTLE",
  "SPECIAL_MOVE_CHECK",
  "SPECIAL_MOVE_CAPTURE",
  "SPECIAL_MOVE_EN_PASSANT_CAPTURE",
  "SPECIAL_MOVE_PROMOTION_QUEEN",
  "SPECIAL_MOVE_PROMOTION_ROOK",
  "SPECIAL_MOVE_PROMOTION_KNIGHT",
  "SPECIAL_MOVE_PROMOTION_BISHOP",
  "SPECIAL_MOVE_MATE"
};
int num_special_moves = (sizeof special_moves / sizeof (char *));
struct move legal_moves[MAX_LEGAL_MOVES];
int legal_moves_count;
#else
extern char piece_ids[];
extern char *piece_names[];
extern char fmt_str[];
extern char *special_moves[];
extern int num_special_moves;
extern struct move legal_moves[];
extern int legal_moves_count;
#endif
