#include "types.h"
#include "stat.h"
#include "user.h"

#define MAX_FRAMES 10
#define MAX_REF 50

void run_fifo(int frames, int refs[], int n_refs, int verbose) {
  int frame_arr[MAX_FRAMES];
  int fifo_ptr = 0;
  int count = 0;
  int faults = 0, hits = 0;

  for(int i=0; i<frames; i++) frame_arr[i] = -1;

  if(verbose) printf(1, "\n=== FIFO Algorithm (%d Frames) ===\n", frames);
  
  for(int i=0; i<n_refs; i++) {
    int page = refs[i];
    int found = 0;

    for(int j=0; j<frames; j++) {
      if(frame_arr[j] == page) {
        found = 1;
        break;
      }
    }

    if(found) {
      hits++;
      if(verbose) printf(1, "Ref: %d -> Hit     | Frames: [ ", page);
    } else {
      faults++;
      if(count < frames) {
        frame_arr[count++] = page;
      } else {
        frame_arr[fifo_ptr] = page;
        fifo_ptr = (fifo_ptr + 1) % frames;
      }
      if(verbose) printf(1, "Ref: %d -> Fault   | Frames: [ ", page);
    }

    if(verbose) {
      for(int j=0; j<frames; j++) {
        if(frame_arr[j] == -1) printf(1, "- ");
        else printf(1, "%d ", frame_arr[j]);
      }
      printf(1, "]\n");
    }
  }
  if(verbose) {
    printf(1, "FIFO Summary -> Faults: %d, Hits: %d, Hit Ratio: %d%%\n", 
           faults, hits, (hits * 100) / n_refs);
  }
}

void run_lru(int frames, int refs[], int n_refs, int verbose) {
  int frame_arr[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  int count = 0;
  int faults = 0, hits = 0;

  for(int i=0; i<frames; i++) {
    frame_arr[i] = -1;
    last_used[i] = 0;
  }

  if(verbose) printf(1, "\n=== LRU Algorithm (%d Frames) ===\n", frames);

  for(int t=1; t<=n_refs; t++) {
    int page = refs[t-1];
    int found = 0;
    int hit_idx = -1;

    for(int j=0; j<frames; j++) {
      if(frame_arr[j] == page) {
        found = 1;
        hit_idx = j;
        break;
      }
    }

    if(found) {
      hits++;
      last_used[hit_idx] = t;
      if(verbose) printf(1, "Ref: %d -> Hit     | Frames: [ ", page);
    } else {
      faults++;
      if(count < frames) {
        frame_arr[count] = page;
        last_used[count] = t;
        count++;
      } else {
        int lru_idx = 0;
        int min_time = last_used[0];
        for(int j=1; j<frames; j++) {
          if(last_used[j] < min_time) {
            min_time = last_used[j];
            lru_idx = j;
          }
        }
        frame_arr[lru_idx] = page;
        last_used[lru_idx] = t;
      }
      if(verbose) printf(1, "Ref: %d -> Fault   | Frames: [ ", page);
    }

    if(verbose) {
      for(int j=0; j<frames; j++) {
        if(frame_arr[j] == -1) printf(1, "- ");
        else printf(1, "%d ", frame_arr[j]);
      }
      printf(1, "]\n");
    }
  }
  if(verbose) {
    printf(1, "LRU Summary  -> Faults: %d, Hits: %d, Hit Ratio: %d%%\n", 
           faults, hits, (hits * 100) / n_refs);
  }
}

int get_fifo_faults(int frames, int refs[], int n_refs) {
  int frame_arr[MAX_FRAMES];
  int fifo_ptr = 0, count = 0, faults = 0;
  for(int i=0; i<frames; i++) frame_arr[i] = -1;
  for(int i=0; i<n_refs; i++) {
    int page = refs[i], found = 0;
    for(int j=0; j<frames; j++) if(frame_arr[j] == page) { found = 1; break; }
    if(!found) {
      faults++;
      if(count < frames) frame_arr[count++] = page;
      else { frame_arr[fifo_ptr] = page; fifo_ptr = (fifo_ptr + 1) % frames; }
    }
  }
  return faults;
}

int get_lru_faults(int frames, int refs[], int n_refs) {
  int frame_arr[MAX_FRAMES], last_used[MAX_FRAMES];
  int count = 0, faults = 0;
  for(int i=0; i<frames; i++) { frame_arr[i] = -1; last_used[i] = 0; }
  for(int t=1; t<=n_refs; t++) {
    int page = refs[t-1], found = 0, hit_idx = -1;
    for(int j=0; j<frames; j++) if(frame_arr[j] == page) { found = 1; hit_idx = j; break; }
    if(found) { last_used[hit_idx] = t; }
    else {
      faults++;
      if(count < frames) { frame_arr[count] = page; last_used[count] = t; count++; }
      else {
        int lru_idx = 0, min_time = last_used[0];
        for(int j=1; j<frames; j++) if(last_used[j] < min_time) { min_time = last_used[j]; lru_idx = j; }
        frame_arr[lru_idx] = page; last_used[lru_idx] = t;
      }
    }
  }
  return faults;
}

int main(int argc, char *argv[]) {
  int default_refs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int n_refs = 20;
  int frames = 3;

  if(argc > 1) {
    frames = atoi(argv[1]);
  }

  printf(1, "Reference String: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");
  
  // Run verbose trace for the specified frames (default 3)
  run_fifo(frames, default_refs, n_refs, 1);
  run_lru(frames, default_refs, n_refs, 1);

  // Print comparison table for 3, 4, and 5 frames
  printf(1, "\n==================================================\n");
  printf(1, " Frames | Algorithm | Page Faults | Hit Ratio (%%)\n");
  printf(1, "==================================================\n");
  
  int f_list[] = {3, 4, 5};
  for(int k=0; k<3; k++) {
    int f = f_list[k];
    int ff = get_fifo_faults(f, default_refs, n_refs);
    int lf = get_lru_faults(f, default_refs, n_refs);
    printf(1, "   %d    | FIFO      |     %d       |     %d%%\n", f, ff, ((n_refs - ff) * 100) / n_refs);
    printf(1, "   %d    | LRU       |     %d       |     %d%%\n", f, lf, ((n_refs - lf) * 100) / n_refs);
  }
  printf(1, "==================================================\n");

  exit();
}