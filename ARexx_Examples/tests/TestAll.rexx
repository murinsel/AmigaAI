/* TestAll.rexx - Run all AmigaAI ARexx tests
 *
 * Usage: rx AmigaAI:ARexx_Examples/tests/TestAll.rexx
 *
 * AmigaAI must be running.
 */

SAY "=== AmigaAI ARexx Test Suite ==="
SAY ""

passed = 0
failed = 0
total  = 0

/* --- Test HELP --- */
CALL RunTest "TestHelp.rexx"

/* --- Test WINDOWPOS / MOVE / RESIZE --- */
CALL RunTest "TestWindow.rexx"

/* --- Test CLEAR / ASK / GETLAST --- */
CALL RunTest "TestChat.rexx"

/* --- Test SETMODEL --- */
CALL RunTest "TestModel.rexx"

/* --- Test SETSYSTEM --- */
CALL RunTest "TestSystem.rexx"

/* --- Test MEMADD / MEMCOUNT / MEMORY / MEMCLEAR --- */
CALL RunTest "TestMemory.rexx"

/* --- Summary --- */
SAY ""
SAY "=== Results: " passed "/" total " passed," failed " failed ==="

IF failed > 0 THEN
    EXIT 5
EXIT 0

RunTest: PROCEDURE EXPOSE passed failed total
    PARSE ARG script
    total = total + 1
    SAY "--- Running " script " ---"
    ADDRESS COMMAND "rx AmigaAI:ARexx_Examples/tests/" || script
    IF RC = 0 THEN DO
        passed = passed + 1
        SAY "  PASSED"
    END
    ELSE DO
        failed = failed + 1
        SAY "  FAILED (RC=" || RC || ")"
    END
    SAY ""
    RETURN
