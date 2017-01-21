#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "token.h"

////////////////////////////////////////////////////////////////////////////////
/// \returns 1 if \p c is a '"' or "'" character, 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
static int isQuoteCharacter(char c)
{
    if (c == '\'' || c == '"') {
        return 1;
    } else {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Find any strings in the \p inputLine that are not closed. Strings may be
/// opened with a "'" or '"' character.
///
/// \param [out] openStringPosition The position (relative to the beginning of
/// the \p inputLine) of the string opening that does not have a matching close
/// will be put here if parameter is not NULL. Value may still change even if no
/// open strings.
///
/// \returns 0 if no open strings, other if any open strings.
////////////////////////////////////////////////////////////////////////////////
static int findOpenStrings(const char *inputLine, int *openStringPosition)
{
    const char *it = inputLine;
    while (*it) {
        // Ignore escape characters
        if (*it == '\\') {
            ++it;
        } else {
            // A string has been opened
            if (isQuoteCharacter(*it)) {
                char quoteType = *it;
                if (openStringPosition != NULL) {
                    *openStringPosition = (int)labs(it - inputLine); // position relative to begining of inputLine
                }
            
                // Advance into string
                ++it;
            
                // Attempt to find a close to the string
                while (*it != quoteType) {
                    // Ignore escape characters
                    if (*it == '\\') {
                        ++it;
                    // If we don't find a close, return 1
                    } else if (*it == '\0') {
                        return 1;
                    }
                
                    ++it;
                }
                // If we do find a close, continue analyzing input
            }
        }
        ++it;
    }
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// \returns 0 if \p c is a whitespace character, 1 otherwise
////////////////////////////////////////////////////////////////////////////////
static int isWhitespaceCharacter(char c)
{
    if (c <= 32 || c > 126) {
        return 1;
    } else {
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Given a character pointer \p it, this function will move the character
/// pointer to the next non-whitespace character and return it.
////////////////////////////////////////////////////////////////////////////////
static char *skipWhitespace(char *it) {
    while (*it && isWhitespaceCharacter(*it)) {
        ++it;
    }
    
    return it;
}

int tokenise(char *inputLine, char *token[])
{
    int numTokens = 0;
    
    char currentQuoteType = '\0';
    int isInString = 0;
    
    if (findOpenStrings(inputLine, NULL) != 0) {
        return -2;
    }
    
    char *it = inputLine;

    if (*it) {
        // Assign start address of input line to token array
        token[numTokens] = it;
        ++numTokens;
    }
    
    // Loop thru input
    while (*it && numTokens < MAX_NUM_TOKENS) {
        // If escape character, skip over it and next character
        if (*it == '\\') {
            ++it;
            ++it;
        } else {
            // If is quote character
            if (isQuoteCharacter(*it)) {
                if (isInString == 0) {
                    // If we are not already inside a string, open a new one
                    currentQuoteType = *it;
                    isInString = 1;
                } else if (*it == currentQuoteType) {
                    // Edge case: if previous character was a quote (i.e. we have an empty string)
                    // we need to strip the empty string from the input line
                    if (*(it - 1) == currentQuoteType) {
                        char *tmpIt = (it - 1); // pointer to opening of empty string
                        // 'it' currently points to the end of the empty string, so it + i is anything after the empty string.
                        // Shift all characters after the empty string in the input line back two characters
                        // to effectively erase the empty string.
                        for (int i = 1; *(it + i) != '\0'; ++i) {
                            *tmpIt = *(it + i);
                            ++tmpIt;
                        }
                        // Make sure to set the null-terminator
                        *tmpIt = '\0';
                    }
                
                    // If we are already inside a string and this quote matches
                    // the current quote type, this is the end of the string
                    isInString = 0;
                    currentQuoteType = '\0';
                }
            }
        
            // If is whitespace not inside a string
            if (isWhitespaceCharacter(*it) && !isInString) {
                // insert '\0' into input line
                *it = '\0';
                ++it;
                // advance to next readable character
                it = skipWhitespace(it);
            
                // Start new token
                token[numTokens] = it;
                ++numTokens;
            // Else keep looping thru input
            } else {
                ++it;
            }
        }
    }
    
    // If we've reached maximum number of tokens and still have more input
    // to tokenise, return error
    if (numTokens == MAX_NUM_TOKENS && *it) {
        return -1;
    }
    
    // Else return number of tokens
    return numTokens;
}
