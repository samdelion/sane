#define MAX_NUM_COMMANDS 100

// Command separators
#define SEP_PIPE "|" // Pipe separator
#define SEP_CON "&"  // Concurrent execution separator "&"
#define SEP_SEQ ";"  // Sequential execution seperator ";"
// Input/output redirection symbols
#define REDIR_IN "<"
#define REDIR_OUT ">"

// Command structure
typedef struct command_t {
    int first; // index to the first token into  the array
    int last;  // index to the last token into the array
    const char
        *sep; // the command seperator that follows the command, must be one of
              // "|", "&", and ";"
    char **argv;       // an array of tokens that forms a command
    char *stdin_file;  // if not NULL, points to the file name for stdin
                       // redirection
    char *stdout_file; // if not NULL, points to the file name for stdout
                       // redirection
} command_t;

////////////////////////////////////////////////////////////////////////////////
/// Separate the list of null-terminated tokens in the "token" array into a
/// sequence of commands, which are stored in the "command" array.
///
/// @pre   The array "command" must have at least MAX_NUM_COMMANDS number of
///        elements.
///
/// @param   token       char *[], array of tokens.
/// @param   numTokens   int, number of tokens in token array.
/// @param   command     command_t [], array of commands out.
/// @return            int,
///  1) -1 if the array "command" is too small for all
///      commands.
///  2) < -1, if there are any of the following syntax errors in the list of
///     tokens:
///      a) -2, if any two successive commands are separated by more than one
///         command separator.
///      b) -3, if the first token is a command separator.
///      c) -4, if the last command is followed by the command separator "|"
///      d) -5, the redirection operator is the last token in any given command.
///  3) Else, the number of commands found in the list of tokens.
///
/// @todo Return error struct instead, with pos of token causing error.
///
/// @note   The last command may be followed by "&", ";", or nothing. If nothing
/// follows the last command, we assume it is followed by ";".
/// @note All members of the command struct should be initialized to 0 before
/// calling this function.
////////////////////////////////////////////////////////////////////////////////
int separateCommands(char *token[], int numTokens, command_t command[]);
