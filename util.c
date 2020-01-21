#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "util.h"

struct server_options as_server_options;

static char as_help_string[] = "AsyncWS - an asynchronous, non-blocking IO, HTTP server capable of serving static files"
                               ", PHP ready pages, being aimed for high performance and efficiency.\n\n"
                               "Usage (command line arguments):\n"
                               "-v         Verbose - prints all logs on standard input (console).\n"
                               "-p PORT    Select a specific port. It defaults to 8888.\n"
                               "-t         Target directory for the web application. Defaults to current working directory if not set.\n"
                               "Mutually exclusive group:\n"
                               "-d         Starts the server in the dynamic mode using asynchronous operations.\n"
                               "-s         Starts the server in static serving mode. This mode uses zero-copy to increase performance"
                               "and reduce memory usage.\n"
                               "-a         [Default]: Starts the server automatic mode. It will choose the best option for your files.\n";


const char *get_filename_extension(const char *filename) 
{
    const char *dot_position = strrchr(filename, '.');
    if(! dot_position || dot_position == filename) 
        return "";
    return dot_position + 1;
}


void as_logger(const char *message)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (as_server_options.verbose)
        printf("[%d-%02d-%02d %02d:%02d:%02d]: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, 
               tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
}


void as_logger_arg(char *message, char *opt)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    if (as_server_options.verbose)
        printf("[%d-%02d-%02d %02d:%02d:%02d]: %s %s\n", tm.tm_year + 1900, tm.tm_mon + 1, 
               tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message, opt);
}


static int as_is_valid_port(char *port)
{
    int i;
    if (port == NULL) 
        return 0;

    for (i = 0; port[i] != '\0'; ++i) 
        if (port[i] < '0' || '9' < port[i])
            return 0;

    return 1;
}


int as_directory_exists(char *path)
{
    DIR* dir = opendir(path);
    if (dir) {
        closedir(dir);
        return 1;
    }
    
    return 0;
}


void as_arg_parse(int argc, char **argv)
{
    int i;
    char *port_str;
    as_server_options.verbose = 0;
    as_server_options.port = DEFAULT_PORT;
    as_server_options.typeof_content = AUTOMATIC_DETECTION;
    as_server_options.target_directory = NULL;
    
    for (i = 1; i < argc; ++i) {
        if(! strcmp(argv[i], "-h")) {
            printf("%s", as_help_string);
            exit(EXIT_SUCCESS);
        }

        else if(! strcmp(argv[i], "-v")) 
            as_server_options.verbose = 1;

        else if(! strcmp(argv[i], "-s")) 
            as_server_options.typeof_content = STATIC_CONTENT;

        else if(! strcmp(argv[i], "-d")) 
            as_server_options.typeof_content = DYNAMIC_CONTENT;

        else if(! strcmp(argv[i], "-a")) 
            as_server_options.typeof_content = AUTOMATIC_DETECTION;
        
        else if(! strcmp(argv[i], "-p")) {
            if (i + 1 >= argc || as_is_valid_port(argv[i + 1]) == 0) {
                as_logger("Invalid port (-p) option");
                continue;
            }

            sscanf(argv[i + 1], "%d", &as_server_options.port);
            port_str = argv[i + 1];
        }    

        else if(! strcmp(argv[i], "-p")) {
            if (i + 1 >= argc || as_is_valid_port(argv[i + 1]) == 0) {
                as_logger("Invalid port (-p) option");
                continue;
            }

            sscanf(argv[i + 1], "%d", &as_server_options.port);
            port_str = argv[i + 1];
        }    

        else if(! strcmp(argv[i], "-t")) {
            if (i + 1 >= argc) {
                as_logger("Target directory (-t) option is missing");
                exit(EXIT_FAILURE);
            }
            else if(! as_directory_exists(argv[i + 1])) {
                as_logger_arg("Exiting. Invalid path for target directory (-t):", argv[i + 1]);
                exit(EXIT_FAILURE);
            }
            as_server_options.target_directory = argv[i + 1];
        }
    }

    if (as_server_options.port == DEFAULT_PORT) 
        as_logger("Server starting on default port.");
    else 
        as_logger_arg("Server starting on port", port_str);
    
    if (as_server_options.target_directory != NULL)
        as_logger_arg("Current target directory is", as_server_options.target_directory);
    else
        as_logger("No target directory specified. Resolving paths from current working directory");

    switch (as_server_options.typeof_content)
    {
    case STATIC_CONTENT:
        as_logger("Server serving STATIC content.");
        break;
    
    case DYNAMIC_CONTENT:
        as_logger("Server serving DYNAMIC content.");
        break;

    case AUTOMATIC_DETECTION:
        as_logger("Server automatically checks for type of content.");
        break;

    default:
        as_logger("Server automatically checks for type of content.");
        break;
    }
}
