#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "--- pttest: Initial state ---\n");
  vmprint();

  printf(1, "\n--- pttest: Allocating 4 new pages via sbrk(4 * 4096) ---\n");
  char *p = sbrk(4 * 4096);
  if(p == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }

  vmprint();

  exit();
}