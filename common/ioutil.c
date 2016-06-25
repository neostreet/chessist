#include <stdio.h>
#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <fcntl.h>
#include <io.h>
#include <unistd.h>
#include <curses.h>
#include "ioutil.h"

extern char couldnt_get_status[];
extern char couldnt_open[];

int file_exists(char *filename)
{
  struct stat stat_buf;

  return (stat(filename,&stat_buf) != -1);
}

int read_bin_file(
char *filename,
char **buf,
unsigned int *num_chars,
int bMalloc,
int bPrintErrors
)
{
  struct stat stat_buf;
  unsigned int file_size;
  char *cpt;
  int fhndl;
  unsigned int bytes_read;

  if (stat(filename,&stat_buf) == -1) {
    if (bPrintErrors)
      printf(couldnt_get_status,filename);

    return 1;
  }

  file_size = (size_t)stat_buf.st_size;

  if (!bMalloc) {
    if (file_size != *num_chars)
      return 2;

    cpt = *buf;
  }
  else {
    if ((cpt = (char *)malloc(file_size)) == NULL) {
      if (bPrintErrors)
        printf("couldn't malloc %u bytes\n",file_size);

      return 3;
    }
  }

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 4;
  }

  bytes_read = read(fhndl,cpt,file_size);

  close(fhndl);

  if (bytes_read != file_size) {
    if (bPrintErrors)
      printf("couldn't read %u bytes\n",file_size);

    return 5;
  }

  if (bMalloc) {
    *buf = cpt;
    *num_chars = file_size;
  }

  return 0;
}

int write_bin_file(
char *filename,
char *buf,
unsigned int num_chars,
int bPrintErrors
)
{
  int fhndl;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 1;
  }

  bytes_written = write(fhndl,buf,num_chars);

  close(fhndl);

  if (bytes_written != num_chars)
    return 2;

  return 0;
}

int read_bin_array(
char *filename,
unsigned int *num_elems,
unsigned int elem_size,
void **buf,
int bPrintErrors
)
{
  struct stat stat_buf;
  unsigned int save_elem_size;
  unsigned int malloc_size;
  char *cpt;
  int fhndl;
  unsigned int bytes_read;

  save_elem_size = elem_size;

  if (stat(filename,&stat_buf) == -1) {
    if (bPrintErrors)
      printf(couldnt_get_status,filename);

    return 1;
  }

  if (stat_buf.st_size & 0xffff0000) {
    if (bPrintErrors)
      printf("file is too big for DOS memory routines\n");

    return 2;
  }

  if ((stat_buf.st_size - (sizeof (int) * 2)) % elem_size) {
    if (bPrintErrors)
      printf("array file %s not internally consistent\n",filename);

    return 3;
  }

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 4;
  }

  bytes_read = read(fhndl,num_elems,sizeof *num_elems);

  if (bytes_read != sizeof *num_elems) {
    printf("read of num_elems failed\n");
    return 5;
  }

  bytes_read = read(fhndl,&elem_size,sizeof elem_size);

  if (bytes_read != sizeof elem_size) {
    printf("read of elem_size failed\n");
    return 6;
  }

  if (save_elem_size != elem_size) {
    if (bPrintErrors)
      printf("element size differs from expected size\n");

    return 7;
  }

  malloc_size = *num_elems * elem_size;

  if (malloc_size + sizeof (int) * 2 != (unsigned int)stat_buf.st_size) {
    if (bPrintErrors)
      printf("array file %s not internally consistent\n",filename);

    return 8;
  }

  if ((cpt = (char *)malloc(malloc_size)) == NULL) {
    if (bPrintErrors)
      printf("couldn't malloc %u bytes\n",malloc_size);

    return 9;
  }

  bytes_read = read(fhndl,cpt,malloc_size);

  close(fhndl);

  if (bytes_read != malloc_size) {
    if (bPrintErrors)
      printf("couldn't read %u bytes\n",malloc_size);

    return 10;
  }

  *buf = (void *)cpt;

  return 0;
}

int write_bin_array(
char *filename,
unsigned int num_elems,
unsigned int elem_size,
void *buf,
int bPrintErrors
)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 1;
  }

  bytes_written = write(fhndl,&num_elems,sizeof num_elems);

  if (bytes_written != sizeof num_elems)
    return 2;

  bytes_written = write(fhndl,&elem_size,sizeof elem_size);

  if (bytes_written != sizeof elem_size)
    return 2;

  bytes_to_write = num_elems * elem_size;
  bytes_written = write(fhndl,buf,bytes_to_write);

  close(fhndl);

  if (bytes_written != bytes_to_write)
    return 2;

  return 0;
}

int write_sorted_bin_array(
char *filename,
unsigned int num_elems,
unsigned int elem_size,
void *buf,
unsigned int *sort_ixs,
int bPrintErrors
)
{
  unsigned int n;
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;
  char *cpt;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 1;
  }

  bytes_written = write(fhndl,&num_elems,sizeof num_elems);

  if (bytes_written != sizeof num_elems)
    return 2;

  bytes_written = write(fhndl,&elem_size,sizeof elem_size);

  if (bytes_written != sizeof elem_size)
    return 2;

  bytes_to_write = elem_size;

  for (n = 0; n < num_elems; n++) {
    cpt = (char *)buf;
    cpt += elem_size * sort_ixs[n];

    bytes_written = write(fhndl,cpt,bytes_to_write);

    if (bytes_written != bytes_to_write)
      break;
  }

  close(fhndl);

  if (bytes_written != bytes_to_write)
    return 2;

  return 0;
}

int read_bin_linked_list(
char *filename,
unsigned int *num_elems,
unsigned int elem_size,
void **first_elem,
void **last_elem,
int bPrintErrors
)
{
  struct stat stat_buf;
  unsigned int save_elem_size;
  int fhndl;
  int retval;

  save_elem_size = elem_size;

  if (stat(filename,&stat_buf) == -1) {
    if (bPrintErrors)
      printf(couldnt_get_status,filename);

    return 1;
  }

  if (stat_buf.st_size & 0xffff0000) {
    if (bPrintErrors)
      printf("file is too big for DOS memory routines\n");

    return 2;
  }

  if (elem_size - 2 * sizeof (char *)) {
    if ((stat_buf.st_size - (sizeof (int) * 2)) % (elem_size - 2 * sizeof (char *))) {
      if (bPrintErrors)
        printf("linked list file %s not internally consistent\n",filename);

      return 3;
    }
  }

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 4;
  }

  retval = read_bin_linked_list_fhndl(fhndl,num_elems,elem_size,
    first_elem,last_elem,bPrintErrors);

  close(fhndl);

  return retval;
}

int read_bin_linked_list_fhndl(
int fhndl,
unsigned int *num_elems,
unsigned int elem_size,
void **first_elem,
void **last_elem,
int bPrintErrors
)
{
  unsigned int n;
  unsigned int save_elem_size;
  unsigned int malloc_size;
  char *cpt;
  char **cppt;
  unsigned int bytes_to_read;
  unsigned int bytes_read;
  void *prev_elem;

  save_elem_size = elem_size;

  bytes_read = read(fhndl,num_elems,sizeof *num_elems);

  if (bytes_read != sizeof *num_elems) {
    printf("read of num_elems failed\n");
    return 1;
  }

  bytes_read = read(fhndl,&elem_size,sizeof elem_size);

  if (bytes_read != sizeof elem_size) {
    printf("read of elem_size failed\n");
    return 2;
  }

  if (elem_size + 2 * sizeof (char *) != save_elem_size) {
    if (bPrintErrors)
      printf("element size differs from expected size\n");

    return 3;
  }

  if (!*num_elems) {
    *first_elem = NULL;
    *last_elem = NULL;
    return 0;
  }

  prev_elem = NULL;
  malloc_size = save_elem_size;
  bytes_to_read = elem_size;

  for (n = 0; n < *num_elems; n++) {
    if ((cpt = (char *)malloc(malloc_size)) == NULL) {
      if (bPrintErrors)
        printf("couldn't malloc %u bytes\n",malloc_size);

      return 4;
    }

    bytes_read = read(fhndl,&cpt[2 * sizeof (char *)],bytes_to_read);

    if (bytes_read != bytes_to_read) {
      if (bPrintErrors)
        printf("couldn't read %u bytes\n",bytes_to_read);

      return 5;
    }

    if (!n)
      *first_elem = (void *)cpt;
    else {
      cppt = (char **)prev_elem;
      cppt[0] = cpt;
    }

    cppt = (char **)cpt;
    cppt[1] = (char *)prev_elem;

    if (n < *num_elems - 1)
      prev_elem = (void *)cpt;
    else {
      *last_elem = (void *)cpt;
      cppt = (char **)cpt;
      cppt[0] = NULL;
    }
  }

  return 0;
}

int write_bin_linked_list(
char *filename,
unsigned int num_elems,
unsigned int elem_size,
void *first_elem,
int bPrintErrors
)
{
  int fhndl;
  int retval;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1) {
    if (bPrintErrors)
      printf(couldnt_open,filename);

    return 1;
  }

  retval = write_bin_linked_list_fhndl(fhndl,num_elems,elem_size,
    first_elem,bPrintErrors);

  close(fhndl);

  return retval;
}

int write_bin_linked_list_fhndl(
int fhndl,
unsigned int num_elems,
unsigned int elem_size,
void *first_elem,
int bPrintErrors
)
{
  unsigned int n;
  char *curr_elem;
  char *cpt;
  unsigned int adjusted_size;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  bytes_written = write(fhndl,&num_elems,sizeof num_elems);

  if (bytes_written != sizeof num_elems)
    return 1;

  adjusted_size = elem_size - 2 * sizeof (char *);
  bytes_written = write(fhndl,&adjusted_size,sizeof adjusted_size);

  if (bytes_written != sizeof adjusted_size)
    return 2;

  if (!num_elems)
    return 0;

  curr_elem = (char *)first_elem;
  bytes_to_write = adjusted_size;

  for (n = 0; n < num_elems; n++) {
    cpt = &curr_elem[2 * sizeof (char *)];
    bytes_written = write(fhndl,cpt,bytes_to_write);

    if (bytes_written != bytes_to_write)
      return 3;

    if (n < num_elems - 1)
      curr_elem = *(char **)curr_elem;
  }

  return 0;
}

void free_linked_list(int num_elems,void *first_elem)
{
  int n;
  void *curr_elem;
  void *work;

  curr_elem = first_elem;

  for (n = 0; n < num_elems; n++) {
    work = curr_elem;

    if (n < num_elems - 1)
      curr_elem = *(void **)curr_elem;

    free(work);
  }
}

int my_getch(void)
{
  int ret;

  ret = getch();

  if (ret)
    return ret;

  ret = getch();
  return ret | EXTENDED_BIT;
}
