#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER		

#ifdef __cplusplus
extern "C" {
#endif

#define AWS_DOCUMENT_ROOT	"./"
#define AWS_ABS_STATIC_FOLDER	(AWS_DOCUMENT_ROOT AWS_REL_STATIC_FOLDER)
#define AWS_ABS_DYNAMIC_FOLDER	(AWS_DOCUMENT_ROOT AWS_REL_DYNAMIC_FOLDER)

#define UNINIT_FD   -1

#ifdef __cplusplus
}
#endif

#endif 
