#define MAX_NUM_TOKENS (100 * 1000)

////////////////////////////////////////////////////////////////////////////////
/// Given a string of characters provided in 'inputLine', splits the characters
/// into a series of tokens and puts them in the output array 'token'.
///
/// If successful, returns the number of tokens parsed. If an error occurs, one
/// of the following error codes will be returned:
///    -1 - if inputLine contains > MAX_NUM_TOKENS
///    -2 - if string is not properly closed, e.g. ("Hello world) instead of
///         ("Hello world")
///
/// @pre   'inputLine' is a NULL-terminated string.
/// @pre   'token' is an array large enough to hold at least MAX_NUM_TOKENS
///
/// @param   inputLine   char *, string of characters to tokenise.
/// @param   token       char *[], tokens out.
/// @return              int, >= 0 if successful, < 0 if error.
////////////////////////////////////////////////////////////////////////////////
int tokenise(char *inputLine, char *token[]);
