/* Function to compute combinations n-choose-k in lexicographic order */
/* Algorithm by Donald Knuth. */
/* C implementation by Glenn C. Rhoads */

#include <stdio.h>
#include <stdlib.h>

void nchoosek(int n, int k)
{
 int i, j = 1, *c, x;
 c = malloc( (k+3) * sizeof(int));
    
 for (i = 1; i <= k; i++)
  c[i] = i;
 
 c[k+1] = n+1;
 c[k+2] = 0;
 j = k;

 visit:
  for (i = k; i >= 1; i--) 
   printf( "%3d", c[i]);
 
  printf( "\n");
  if (j > 0) 
  {
   x = j + 1;
   goto incr;
  }
 
  if(c[1] + 1 < c[2])
  {
   c[1] += 1;
   goto visit;
  }
  j = 2;

 do_more:
  c[j-1] = j-1;
  x = c[j] + 1;
  if (x == c[j+1])
  {
   j++; 
   goto do_more;
  }
  if (j > k) 
   exit(0);

 incr:
  c[j] = x;
  j--;
  goto visit;
}


void main( void)
{
 nchoosek(3,2);
}


