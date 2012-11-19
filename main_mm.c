/***********************
Author: Hemant Yadav
***********************/
#include "mm_public.h"

int main (int argc, char **argv)
{
  int j, i;
  struct timeval times, timee;
 void *mychunk = NULL;
  mm_t MM;

  j = gettimeofday (&times, (void *)NULL);
  if (mm_init(&MM, 32) < 0)
  {
    perror("mm_init");
    exit(EXIT_FAILURE);
  }
for (i=0; i<HOW; i++) { 
    mychunk = mm_get(&MM, i+1);
    mm_put(&MM,mychunk);
  
}
mm_release(&MM);
j = gettimeofday (&timee, (void *)NULL);
fprintf (stderr, "MM time took %f msec\n",comp_time (times, timee)/1000.0);
}
