/* TestModel.rexx - Test SETMODEL command
 *
 * Tests: SETMODEL
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

/* --- SETMODEL: change to haiku (cheaper for testing) --- */
SAY "  Testing SETMODEL..."
SETMODEL "claude-haiku-4-5-20251001"
IF RC ~= 0 THEN DO
    SAY "  FAIL: SETMODEL returned RC=" || RC
    EXIT 5
END
SAY "  OK: SETMODEL to claude-haiku-4-5-20251001"

/* --- Restore default model --- */
SETMODEL "claude-sonnet-4-6"
IF RC ~= 0 THEN DO
    SAY "  FAIL: SETMODEL restore returned RC=" || RC
    EXIT 5
END
SAY "  OK: SETMODEL restored to claude-sonnet-4-6"

EXIT 0
