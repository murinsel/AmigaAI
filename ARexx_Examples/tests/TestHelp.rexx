/* TestHelp.rexx - Test HELP command
 *
 * Verifies that HELP returns a list of available commands.
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

SAY "  Testing HELP..."
HELP
IF RC ~= 0 THEN DO
    SAY "  FAIL: HELP returned RC=" || RC
    EXIT 5
END

IF RESULT = "" THEN DO
    SAY "  FAIL: HELP returned empty result"
    EXIT 5
END

SAY "  OK: HELP returned command list"
EXIT 0
