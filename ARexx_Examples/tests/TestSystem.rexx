/* TestSystem.rexx - Test SETSYSTEM command
 *
 * Tests: SETSYSTEM
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

/* --- SETSYSTEM: set a custom system prompt --- */
SAY "  Testing SETSYSTEM..."
SETSYSTEM "You are a helpful test assistant."
IF RC ~= 0 THEN DO
    SAY "  FAIL: SETSYSTEM returned RC=" || RC
    EXIT 5
END
SAY "  OK: SETSYSTEM set custom prompt"

/* --- SETSYSTEM: reset to empty (restores default) --- */
SETSYSTEM ""
IF RC ~= 0 THEN DO
    SAY "  FAIL: SETSYSTEM reset returned RC=" || RC
    EXIT 5
END
SAY "  OK: SETSYSTEM reset to default"

EXIT 0
