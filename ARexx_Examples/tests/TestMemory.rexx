/* TestMemory.rexx - Test memory commands
 *
 * Tests: MEMCLEAR, MEMCOUNT, MEMADD, MEMORY
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

/* --- MEMCLEAR: start clean --- */
SAY "  Testing MEMCLEAR..."
MEMCLEAR
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMCLEAR returned RC=" || RC
    EXIT 5
END
SAY "  OK: MEMCLEAR"

/* --- MEMCOUNT: should be 0 --- */
SAY "  Testing MEMCOUNT (expect 0)..."
MEMCOUNT
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMCOUNT returned RC=" || RC
    EXIT 5
END

IF RESULT ~= 0 THEN DO
    SAY "  FAIL: MEMCOUNT =" RESULT "(expected 0)"
    EXIT 5
END
SAY "  OK: MEMCOUNT = 0"

/* --- MEMADD: add entries --- */
SAY "  Testing MEMADD..."
MEMADD "Test entry one"
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMADD #1 returned RC=" || RC
    EXIT 5
END
SAY "  OK: MEMADD #1"

MEMADD "Test entry two"
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMADD #2 returned RC=" || RC
    EXIT 5
END
SAY "  OK: MEMADD #2"

/* --- MEMCOUNT: should be 2 --- */
SAY "  Testing MEMCOUNT (expect 2)..."
MEMCOUNT
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMCOUNT returned RC=" || RC
    EXIT 5
END

IF RESULT ~= 2 THEN DO
    SAY "  FAIL: MEMCOUNT =" RESULT "(expected 2)"
    EXIT 5
END
SAY "  OK: MEMCOUNT = 2"

/* --- MEMORY: get all entries --- */
SAY "  Testing MEMORY..."
MEMORY
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMORY returned RC=" || RC
    EXIT 5
END

IF RESULT = "" THEN DO
    SAY "  FAIL: MEMORY returned empty"
    EXIT 5
END

SAY "  OK: MEMORY returned:" LEFT(RESULT, 60)

/* --- MEMCLEAR: clean up --- */
MEMCLEAR
IF RC ~= 0 THEN DO
    SAY "  FAIL: MEMCLEAR cleanup returned RC=" || RC
    EXIT 5
END

/* Verify it's clean */
MEMCOUNT
IF RESULT ~= 0 THEN DO
    SAY "  FAIL: MEMCOUNT after clear =" RESULT "(expected 0)"
    EXIT 5
END
SAY "  OK: MEMCLEAR + MEMCOUNT verified"

EXIT 0
