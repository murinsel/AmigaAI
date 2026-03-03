/* TestChat.rexx - Test chat commands
 *
 * Tests: CLEAR, ASK, GETLAST
 * NOTE: ASK makes a real API call! Requires valid API key.
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

/* --- CLEAR: clear conversation --- */
SAY "  Testing CLEAR..."
CLEAR
IF RC ~= 0 THEN DO
    SAY "  FAIL: CLEAR returned RC=" || RC
    EXIT 5
END
SAY "  OK: CLEAR"

/* --- GETLAST after CLEAR: should be empty --- */
SAY "  Testing GETLAST (after CLEAR)..."
GETLAST
IF RC ~= 0 THEN DO
    SAY "  FAIL: GETLAST returned RC=" || RC
    EXIT 5
END

IF RESULT ~= "" THEN
    SAY "  WARN: GETLAST not empty after CLEAR:" LEFT(RESULT, 40)
ELSE
    SAY "  OK: GETLAST is empty after CLEAR"

/* --- ASK: send a simple question --- */
SAY "  Testing ASK (API call)..."
SAY "  (This may take a few seconds...)"
ASK "Reply with exactly: TEST_OK"
IF RC ~= 0 THEN DO
    SAY "  FAIL: ASK returned RC=" || RC
    SAY "  Check that ENV:AmigaAI/api_key is set."
    EXIT 5
END

SAY "  OK: ASK returned:" LEFT(RESULT, 60)

/* --- GETLAST: should return same as ASK --- */
SAY "  Testing GETLAST (after ASK)..."
saved_result = RESULT
GETLAST
IF RC ~= 0 THEN DO
    SAY "  FAIL: GETLAST returned RC=" || RC
    EXIT 5
END

IF RESULT = "" THEN DO
    SAY "  FAIL: GETLAST empty after ASK"
    EXIT 5
END

SAY "  OK: GETLAST returned:" LEFT(RESULT, 60)

/* --- Clean up --- */
CLEAR

EXIT 0
