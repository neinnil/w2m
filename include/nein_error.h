#ifndef _NEIN_ERROR_H
#define _NEIN_ERROR_H

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_NEINERROR(n,e)	((n)<<8 | (e))
#define GET_NEINERROR(x)	(((x)>>8)&0xFF)
#define GET_ERROR(x)		((x)&0xFF)

#define NEIN_NOT_SUPPORTED	0x04
#define NEIN_IS_SYMLINK		0x08

extern void error_print(arg...);

#ifdef __cplusplus
}
#endif

#endif /* _NEIN_ERROR_H */
