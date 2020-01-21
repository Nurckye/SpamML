#ifndef UT_HEADER
#define UT_HEADER		

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT	8888
#define STDIN_FILENO	0
#define STDOUT_FILENO	1


enum content_type {
	STATIC_CONTENT,
	DYNAMIC_CONTENT,
	AUTOMATIC_DETECTION
};

struct server_options {
	int verbose;
	int port;
	enum content_type typeof_content;
	char *target_directory;
};


/* error printing macro */
#define ERR(call_description)				\
	do {						\
		fprintf(stderr, "(%s, %d): ",		\
			__FILE__, __LINE__);		\
		perror(call_description);		\
	} while (0)


/* print error (call ERR) and exit */
#define DIE(assertion, call_description)		\
	do {						\
		if (assertion) {			\
			ERR(call_description);		\
			exit(EXIT_FAILURE);		\
		}					\
	} while (0)


const char *get_filename_extension(const char*);
void as_logger(const char*);
void as_logger_arg(char*, char*);
void as_arg_parse(int, char**);
int as_directory_exists(char*);


#ifdef __cplusplus
}
#endif

#endif