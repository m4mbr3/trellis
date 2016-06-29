/* privsep library header */
#include <unistd.h>
extern void *privsep_malloc (size_t size, unsigned int privlev);
extern void  privsep_free (void *p);

