# REXX ARexx Port (RexxMast)

Port name: REXX. This is the ARexx interpreter/server itself.

## Commands
- `scriptname[.rexx] [parameters...]` -- Execute an ARexx script file (.rexx extension optional)
- `"ARexx commands"` or `'ARexx commands'` -- Execute inline ARexx commands directly

## CRITICAL: Quoting Rules for ARexx

When sending commands to ANY external ARexx port (YAM, IBROWSE, SGRAB, etc.), commands MUST be enclosed in single quotes. This applies everywhere: arexx_command tool AND ARexx scripts.

**Why:** In ARexx, unquoted words are treated as variable names and get substituted. `MAILWRITE` without quotes becomes the value of variable MAILWRITE (empty string). `'MAILWRITE'` sends the literal command.

**ARexx script template for sending commands to a port:**
```
/* Script comment required on first line */
OPTIONS RESULTS
ADDRESS 'PORTNAME'
'COMMAND "argument"'
result = RC
```

**WRONG:**
```
ADDRESS 'YAM'
MAILWRITE           /* ARexx sees variable MAILWRITE = "" */
WRITETO "foo@bar"   /* ARexx sees variable WRITETO, then "foo@bar" */
```

**CORRECT:**
```
ADDRESS 'YAM'
'MAILWRITE'                      /* sends command MAILWRITE */
'WRITETO "foo@bar.com"'          /* sends command with string arg */
'WRITESUBJECT "Hello World"'     /* sends command with string arg */
```
