# AMIGAAI ARexx Port

Port name: AMIGAAI. This is your own ARexx port. Other programs can send commands to control you.

## Commands
- `ASK <text>` -- Send a question to Claude and return the response
- `GETLAST` -- Return the last response from a previous ASK command
- `CLEAR` -- Clear conversation history and last response
- `SETMODEL <model>` -- Change the AI model (e.g. claude-sonnet-4-6)
- `SETSYSTEM <prompt>` -- Change the system prompt
- `MEMADD <text>` -- Add a persistent memory entry
- `MEMCLEAR` -- Clear all persistent memory entries
- `MEMCOUNT` -- Return the number of memory entries
- `MEMORY` -- Return all memory entries as text
- `QUIT` -- Quit AmigaAI

## Return Codes
- 0 = Success (result string available if RXFF_RESULT set)
- 5 = Unknown command
- 10 = Error (ASK failed, memory full)
- 20 = Out of memory (CLEAR failed)

## Example ARexx Script
```rexx
/* Ask Claude a question from another program */
ADDRESS AMIGAAI
OPTIONS RESULTS
ASK "What is the Amiga?"
SAY RESULT
```
