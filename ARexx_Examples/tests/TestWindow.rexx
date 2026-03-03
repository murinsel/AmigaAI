/* TestWindow.rexx - Test window management commands
 *
 * Tests: WINDOWPOS, MOVE, RESIZE, WINDOWTOFRONT, WINDOWTOBACK
 */

ADDRESS AMIGAAI
OPTIONS RESULTS

/* --- WINDOWPOS: get current position --- */
SAY "  Testing WINDOWPOS..."
WINDOWPOS
IF RC ~= 0 THEN DO
    SAY "  FAIL: WINDOWPOS returned RC=" || RC
    EXIT 5
END

PARSE VAR RESULT orig_left orig_top orig_width orig_height
SAY "  OK: Position =" orig_left orig_top orig_width orig_height

/* --- MOVE: move window --- */
SAY "  Testing MOVE..."
MOVE 50 50
IF RC ~= 0 THEN DO
    SAY "  FAIL: MOVE returned RC=" || RC
    EXIT 5
END

/* Verify it moved */
WINDOWPOS
PARSE VAR RESULT new_left new_top .
IF new_left ~= 50 | new_top ~= 50 THEN DO
    SAY "  WARN: Position after MOVE is" new_left new_top "(expected 50 50)"
END
ELSE
    SAY "  OK: MOVE to 50 50 confirmed"

/* --- RESIZE: resize window --- */
SAY "  Testing RESIZE..."
RESIZE 500 400
IF RC ~= 0 THEN DO
    SAY "  FAIL: RESIZE returned RC=" || RC
    EXIT 5
END

WINDOWPOS
PARSE VAR RESULT . . new_w new_h
IF new_w ~= 500 | new_h ~= 400 THEN
    SAY "  WARN: Size after RESIZE is" new_w new_h "(expected 500 400)"
ELSE
    SAY "  OK: RESIZE to 500x400 confirmed"

/* --- WINDOWTOBACK / WINDOWTOFRONT --- */
SAY "  Testing WINDOWTOBACK..."
WINDOWTOBACK
IF RC ~= 0 THEN DO
    SAY "  FAIL: WINDOWTOBACK returned RC=" || RC
    EXIT 5
END
SAY "  OK: WINDOWTOBACK"

CALL Delay(25)   /* brief pause so user can see it */

SAY "  Testing WINDOWTOFRONT..."
WINDOWTOFRONT
IF RC ~= 0 THEN DO
    SAY "  FAIL: WINDOWTOFRONT returned RC=" || RC
    EXIT 5
END
SAY "  OK: WINDOWTOFRONT"

/* --- Restore original position and size --- */
MOVE orig_left orig_top
RESIZE orig_width orig_height

SAY "  OK: Window restored to original position"
EXIT 0
