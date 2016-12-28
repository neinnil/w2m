#ifndef __NIL_DEBUG_H
#define __NIL_DEBUG_H


#include <stdio.h>

#ifndef NIL_DEBUG_FLAG
#define NIL_DEBUG_FLAG 0
#endif

#if !NIL_DEBUG_FLAG
#define NIL_DEBUG_OFF
#endif

#if defined(NDEBUG) || defined(NIL_DEBUG_OFF)
#define NIL_DEBUG(fmt...)  do { 	\
	; \
}while(0)
#else
#define NIL_DEBUG(fmt...)  do {		\
	fprintf(stdout, fmt);			\
}while(0)
#endif

#define NIL_ERROR(fmt...) 	do { 	   \
	fprintf(stderr, fmt); \
}while(0)


#endif
