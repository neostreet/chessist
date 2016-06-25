#ifndef _IOUTIL_H_
#define _IOUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

int file_exists(char *filename);

int read_bin_file(
  char *filename,
  char **buf,
  unsigned int *num_chars,
  int bMalloc,
  int bPrintErrors
  );

int write_bin_file(
  char *filename,
  char *buf,
  unsigned int num_chars,
  int bPrintErrors
  );

int read_bin_array(
  char *filename,
  unsigned int *num_elems,
  unsigned int elem_size,
  void **buf,
  int bPrintErrors
  );

int write_bin_array(
  char *filename,
  unsigned int num_elems,
  unsigned int elem_size,
  void *buf,
  int bPrintErrors
  );

int write_sorted_bin_array(
  char *filename,
  unsigned int num_elems,
  unsigned int elem_size,
  void *buf,
  unsigned int *sort_ixs,
  int bPrintErrors
  );

int read_bin_linked_list(
  char *filename,
  unsigned int *num_elems,
  unsigned int elem_size,
  void **first_elem,
  void **last_elem,
  int bPrintErrors
  );

int read_bin_linked_list_fhndl(
  int fhndl,
  unsigned int *num_elems,
  unsigned int elem_size,
  void **first_elem,
  void **last_elem,
  int bPrintErrors
  );

int write_bin_linked_list(
  char *filename,
  unsigned int num_elems,
  unsigned int elem_size,
  void *first_elem,
  int bPrintErrors
  );

int write_bin_linked_list_fhndl(
  int fhndl,
  unsigned int num_elems,
  unsigned int elem_size,
  void *first_elem,
  int bPrintErrors
  );

void free_linked_list(
  int num_elems,
  void *first_elem
  );

#define EXTENDED_BIT 0x100

int my_getch(void);

#ifdef __cplusplus
}
#endif

#endif  /* _IOUTIL_H_ */
