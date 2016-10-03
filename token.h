#define MAX_NUM_TOKENS 1024

////////////////////////////////////////////////////////////////////////////////
/// Given a string of characters provided in 'inputLine', splits the characters
/// into a series of tokens and puts them in the output array 'token'.
///
/// @pre   'inputLine' is a NULL-terminated string.
/// @pre   'token' is an array large enough to hold at least MAX_NUM_TOKENS
///
/// @param   inputLine   char *, string of characters to tokenise.
/// @param   token       char *[], tokens out.
////////////////////////////////////////////////////////////////////////////////
int tokenise(char *inputLine, char *token[]);
