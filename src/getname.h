#ifndef _GET_NAME_H
#define _GET_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

extern int existFile (char *filename);
extern char* get_suggested_filename (char *src, char *dst_ext);


#ifdef __cplusplus
}
#endif

#endif
