/* AskClaude.rexx - Ask Claude a question from ARexx
 *
 * Usage: rx AskClaude.rexx "What is the Amiga?"
 *
 * Sends a question to AmigaAI via ARexx and prints the response.
 * AmigaAI must be running.
 */

PARSE ARG question

IF question = '' THEN DO
    SAY 'Usage: rx AskClaude.rexx "Your question here"'
    EXIT 5
END

ADDRESS AMIGAAI
OPTIONS RESULTS

ASK question

IF RC = 0 THEN
    SAY RESULT
ELSE
    SAY 'Error: RC=' || RC

EXIT RC
