#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "token.h"

////////////////////////////////////////////////////////////////////////////////
/// Takes a NULL-terminated string currently pointing at the escape character
/// and expands the escape character and it's following character into the
/// required character, shortening the string as necessary.
///
/// For example, will expand \" to " and \' to '
///
/// @param   str   char *, pointer to NULL-terminated string, currently pointing
/// at escape character ('\').
////////////////////////////////////////////////////////////////////////////////
void expandEscapeCharacter(char *str)
{
    if (*str && *(str + 1)) {
        if (*str == '\\') {
            if (*(str + 1) == '"' || *(str + 1) == '\'') {
                *str = *(str + 1);
            } else { // Handle more escape expansions
                // Nothing to expand
                return;
            }

            // Shorten rest of string, replacing each character with it's
            // successive character, shortening the string.
            // E.g. [\\a\0] becomes [\a\0\0] (note two NULL-terminators)
            while (*(++str)) {
                *str = *(str + 1);
            }
        }
    }
}

int tokenise(char *inputLine, char *token[])
{
    int numTokens = 0;

    char *it = inputLine;
    while (*it && numTokens < MAX_NUM_TOKENS) {
        // Skip characters we aren't interested in such as space, tab, newline
        while (*it && ((*it <= 32) || (*it > 126))) {
            ++it;
        }

        if (*it) {
            if ((*it == '"') | (*it == '\'')) {
                // Quote type is either " or '
                char quoteType = (*it == '"') ? '"' : '\'';

                // Assign start address of string (after quote) to array
                // token[numTokens] = ++it;
                token[numTokens] = it++;
                ++numTokens;

                // Don't stop consuming characters until we find end of input or
                // end of string
                while (*it) {
                    // If escape character, skip over next character (ignore it)
                    if (*it == '\\') {
                        /* expandEscapeCharacter(it); */
                        ++it;
                    } else if (*it == '"' || *it == quoteType) {
                        char innerQuoteType = *it;
                        char *itStr = ++it;
                        int isStringClosed = 0;

                        while (*itStr) {
                            // Skip over escaped characters
                            if (*itStr == '\\') {
                                ++itStr;
                            } else if (*itStr == innerQuoteType) {
                                isStringClosed = 1;
                                break;
                            }
                            ++itStr;
                        }

                        if (!isStringClosed) { // This is the end of our string
                            --it; // Go back one so rest of fn sees we reached end of our string
                            break;
                        } else {
                            it = itStr;
                        }
                    }
                    ++it;
                }

                // If we reached end of input before string was closed, return
                // error
                if (!(*it)) {
                    return -2;
                }

                ++it;
            } else {
                // Assign start address of token to array
                token[numTokens] = it;
                ++numTokens;

                // Skip characters we are interested in ('a', 'b', '!', etc.)
                // (not including space, tab, newline etc.)
                while (*it && ((*it > 32) && (*it <= 126))) {
                    // Skip escaped characters
                    if (*it == '\\') {
                        ++it;
                    } else if ((*it == '"') | (*it == '\'')) {
                        // Ensure that string is closed
                        char quoteType = *it;
                        char *itStr = ++it;
                        int stringClosed = 0;
                        while (*itStr) {
                            // Skip over escaped characters
                            if (*itStr == '\\') {
                                ++itStr;
                            } else if (*itStr == quoteType) {
                                stringClosed = 1;
                                break;
                            }
                            ++itStr;
                        }

                        // Return error if string not closed
                        if (!stringClosed) {
                            return -2;
                        } else {
                            it = itStr;
                        }
                    }
                    ++it;
                }
            }

            // Put null-terminator at end of token
            if (*it) {
                *it = '\0';
                // Advance iterator one character
                ++it;
            }
        }

        // If we've reached maximum number of tokens and still have more input
        // to tokenise, return error
        if (numTokens == MAX_NUM_TOKENS && *it) {
            return -1;
        }
    }

    return numTokens;
}
