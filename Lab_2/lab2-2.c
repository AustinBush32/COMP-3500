#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int nloop = 50;

/**********************************************************\
 * Function: increment a counter by some amount one by one *
 * argument: ptr (address of the counter), increment       *
 * output  : nothing                                       *
 **********************************************************/
void add_n(int *ptr, int increment){
  int i,j;
  for (i=0; i < increment; i++){
    *ptr = *ptr + 1;
    for (j=0; j < 1000000;j++);
  }
}

int main(){
  int pid;        /* Process ID                     */

  int *countptr;  /* pointer to the counter         */
  int *interestptr0;
  int *interestptr1;
  int *turnptr;

  int fd;     /* file descriptor to the file "containing" my counter */
  int fdi0;
  int fdi1;
  int fdt;
  int zero = 0; /* a dummy variable containing 0 */

  system("rm -f counter");
  system("rm -f interested0");
  system("rm -f interested1");
  system("rm -f turn");

  /* create a file which will "contain" my shared variable */
  fd = open("counter",O_RDWR | O_CREAT);
  fdi0 = open("interested0", O_RDWR | O_CREAT);
  fdi1 = open("interested1", O_RDWR | O_CREAT);
  fdt = open("turn", O_RDWR | O_CREAT);
  write(fd,&zero,sizeof(int));
  write(fdi0,&zero,sizeof(int));
  write(fdi1,&zero,sizeof(int));
  write(fdt,&zero,sizeof(int));

  /* map my file to memory */
  countptr = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);
  interestptr0 = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fdi0,0);
  interestptr1 = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fdi1,0);
  turnptr = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fdt,0);


 
  if (!countptr || !interestptr0 || !interestptr1 || !turnptr) {
    printf("Mapping failed\n");
    exit(1);
  }
  *countptr = 0;

  close(fd);
  close(fdi0);
  close(fdi1);
  close(fdt);


  setbuf(stdout,NULL);

  pid = fork();
  if (pid < 0){
    printf("Unable to fork a process\n");
    exit(1);
  }

  if (pid == 0) {
    /* The child increments the counter by two's */
    do {
      *turnptr = 0;
      *interestptr0 = 1;
      while (*interestptr0 && *turnptr == 0);
      add_n(countptr,2);
      printf("Child process -->> counter= %d\n",*countptr);
      *interestptr0 = 0;
    } while (*countptr < nloop);
    close(fd);
    close(fdi0);
    close(fdi1);
    close(fdt);
  }
  else {
    /* The parent increments the counter by twenty's */
    do {
      *turnptr = 1;
      *interestptr1 = 1;
      while (*interestptr1 && *turnptr == 1);
      add_n(countptr,20);
      printf("Parent process -->> counter = %d\n",*countptr);
      *interestptr1 = 0;
    } while (*countptr < nloop);
    close(fd);
    close(fdi0);
    close(fdi1);
    close(fdt);
  }
}









