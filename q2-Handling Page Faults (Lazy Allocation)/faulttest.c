#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  printf(1, "--- faulttest: starting ---\n");

  char *p = sbrk(32 * 4096);
  if(p == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }

  printf(1, "Initial page faults: %d\n", getfaults());

  // Write to 5 different pages
  p[0] = 'A';           // Page 1
  p[4096] = 'B';      // Page 2
  p[8192] = 'C';      // Page 3
  p[12288] = 'D';     // Page 4
  p[16384] = 'E';     // Page 5

  int faults = getfaults();
  printf(1, "Accessed 5 pages. Total page faults: %d (Expected: 5)\n", faults);

  // Test invalid access above allocated size
  printf(1, "Testing invalid access above sbrk size (should kill process)...\n");
  char *bad = p + (32 * 4096) + 4096;
  *bad = 'X'; 

  printf(1, "ERROR: process should have been killed by page fault!\n");
  exit();
}