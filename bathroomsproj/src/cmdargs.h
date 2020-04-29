struct QArgs_struct {
    int nsecs, nplaces, nthreads;
    char  * fifoname;
};
typedef struct QArgs_struct QArgs;

struct UArgs_struct {
    int nsecs;
    char  * fifoname;
};
typedef struct UArgs_struct UArgs;


/**
 *  @brief Gets the commandLineArgs for Q 
 * 
 *  @param argc numero de argumentos
 *  @param argv array de strings (argumentos)
 */
QArgs getCommandLineArgsQ(int argc, char * argv[]);

/**
 *  @brief Gets the commansLineArgs for U
 * 
 *  @param argc numero de argumentos
 *  @param argv array de strings (argumentos)
 */
UArgs getCommandLineArgsU(int argc, char * argv[]);
