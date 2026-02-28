#ifndef AMIGAAI_TOOLS_H
#define AMIGAAI_TOOLS_H

#include "cJSON.h"

#define TOOLS_MAX_OUTPUT   16384   /* Max bytes returned from a tool */
#define TOOLS_MAX_ITERATIONS  10   /* Max tool-use rounds per user message */

/* Build the "tools" JSON array for the Claude API request.
 * Returns a cJSON array (caller owns it). */
cJSON *tools_build_json(void);

/* Execute a tool by name with the given input object.
 * Returns a newly allocated result string (caller must free).
 * Sets *is_error to 1 if the tool execution failed. */
char *tool_execute(const char *name, cJSON *input, int *is_error);

#endif /* AMIGAAI_TOOLS_H */
