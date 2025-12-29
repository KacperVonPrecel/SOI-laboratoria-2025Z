#include <stdio.h>
#include <unistd.h>
#include <lib.h>
                                                                                
PUBLIC int hole_map( void *buffer, size_t nbytes)
{
	message m;
	m.m1_p1 = (char *)buffer;
	m.m1_i1 = nbytes;
	return _syscall(MM, HOLE_MAP, & m);
}
                                                                                
int
main( void )
{
        unsigned int    b[1024];
        unsigned int    *p, a, l;
        int     res;

	memset(b, 0, sizeof(b));

	res = hole_map( b, sizeof( b ) );
	
        printf( "[%d]\t", res );
        if (res < 0) {
		printf("Błąd wywołania systemowego: %d\n", res);
		return 1;
    	}
        
        p = b;
        while( *p && p < &b[1023])
        {
                l = *p++;
                a = *p++; /* tu niewykorzystywane */
                printf( "%d\t", l );
        }
        printf( "\n" );
        return 0;
}

