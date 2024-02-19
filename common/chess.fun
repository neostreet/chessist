/*** chess function declarations ***/

int read_binary_game(char *filename,struct game *gamept);
int write_binary_game(char *filename,struct game *gamept);
char xlate_piece(char);
int get_word(FILE *fptr,char *word,int maxlen,int *wordlenpt);
int get_draw_input(struct game *gamept);
int get_xstart(struct game *gamept,int board_offset);
int get_ystart(struct game *gamept,int board_offset);
int get_color(int);
void put_square(struct game *gamept,int what,int where);
void update_move_number(struct game *gamept);
void copy_game(struct game *gamept_to,struct game *gamept_from);

int do_pawn_move(struct game *gamept);
int get_piece_id_ix(char piece);
int do_piece_move(struct game *gamept);
int allow_user_moves(struct game *gamept);

int rook_move(struct game *,int,int,int,int);
int rook_move2(struct game *);
int knight_move(struct game *,int,int,int,int);
int knight_move2(struct game *);
int bishop_move(struct game *,int,int,int,int);
int bishop_move2(struct game *);
int queen_move(struct game *,int,int,int,int);
int queen_move2(struct game *);
int king_move(struct game *,int,int,int,int);
int king_move2(struct game *);

int get_to_position(char *word,int wordlen,int *to_filept,int *to_rankpt);

void populate_initial_board(struct game *gamept);
void set_initial_board(struct game *gamept);
void position_game(struct game *gamept,int move);
void update_board(struct game *gamept,int *invalid_squares,int *num_invalid_squares);
int get_piece1(unsigned char *board,int board_offset);
int get_piece2(unsigned char *board,int row,int column);
void set_piece1(unsigned char *board,int board_offset,int piece);
void set_piece2(unsigned char *board,int row,int column,int piece);

void print_bd(struct game *gamept);
void fprint_game(struct game *gamept,char *filename);
void fprint_game2(struct game *gamept,FILE *fptr);
void fprint_bd(struct game *gamept,char *filename);
void fprint_bd2(struct game *gamept,FILE *fptr);
void fprint_moves(struct game *gamept,char *filename);
void fprint_moves2(struct game *gamept,FILE *fptr);

void print_game(struct game *gamept);
void fprintf_move(FILE *fptr,struct game *gamept);
void sprintf_move(struct game *gamept,char *buf,int buf_len);

int square_attacks_square(unsigned char *board,int square1,int square2);
int pawn_attacks_square(unsigned char *board,int square1,int color,int square2);
int rook_attacks_square(unsigned char *board,int square1,int square2);
int knight_attacks_square(unsigned char *board,int square1,int square2);
int bishop_attacks_square(unsigned char *board,int square1,int square2);
int queen_attacks_square(unsigned char *board,int square1,int square2);
int king_attacks_square(unsigned char *board,int square1,int square2);
bool player_is_in_check(bool bBlack,unsigned char *board);
int calc_square(char *algebraic_notation);
