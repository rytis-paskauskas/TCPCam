#ifndef __SKEL_H__
#define __SKEL_H__

#define INIT()	( program_name = \
		  strrchr( argv[ 0 ], '/' ) ) ?	\
    program_name++ :				\
    ( program_name = argv[ 0 ] )
#define	CLOSE(s)	if (close( s )) error( 1, errno, "close failed")
#define	set_errno(e)	errno = ( e )
#define isvalidsock(s)	( ( s ) >= 0 )
/* #define SOCKET int */
typedef int SOCKET;
#endif	/* __SKEL_H__ */
