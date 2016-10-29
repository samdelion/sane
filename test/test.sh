#!/usr/bin/expect -f
set timeout 2 
log_user 0
set numTests 0
set currTestSuiteName "None"
set extraNewLine ""

### Procedures ###
proc performTest { testInput expectedOutput prompt } {
    global numTests
    global extraNewLine

    send_user "\[ RUN      ] $testInput \n"
    send "$testInput\r"
    expect {
        timeout { send_user "$extraNewLine\[     FAIL ] $testInput\n"; exit 1; }
        $expectedOutput
    }
    expect {
        timeout { send_user "$extraNewLine\[     FAIL ] $testInput\n"; exit 1; }
        $prompt
    }
    send_user "$extraNewLine\[       OK ] $testInput\n"

    # Increment number of successful tests
    set numTests [expr $numTests + 1]
}

proc startTestSuite { testSuiteName } {
    global currTestSuiteName
    global numTests

    if {$numTests != 0} {
        send_user "ERROR: Test suite: \"$currTestSuiteName\" wasn't ended! Call 'endTestSuite'\n"
        exit 1;
    }

    send_user "\[\-\-\-\-\-\-\-\-\-\-\] Test suite: $testSuiteName\n"

    set currTestSuiteName $testSuiteName
}

proc endTestSuite { } {
    global numTests
    global currTestSuiteName

    send_user "\[\-\-\-\-\-\-\-\-\-\-\] $numTests tests completed successfully from test suite: $currTestSuiteName\n\n"

    # Reset number of tests
    set numTests 0
    set currTestSuiteName "None"
}

proc setVerbose { boolean } {
    global extraNewLine

    if {$boolean == "true"} {
        log_user 1
        set extraNewLine "\n"
    }
}

### Setup ###
set argument [lindex $argv 0]

if {$argument == "v"} {
    setVerbose "true"
}

send_user "\[\=\=\=\=\=\=\=\=\=\=\] Running tests\n"
send_user "\[\-\-\-\-\-\-\-\-\-\-\] Test environment setup... "

spawn ../bin/sane
expect {
    timeout { send_user "FAIL: Startup failed\n"; exit 1; }
    "%"
}

send_user "Done\n"

set prompt "%"

### Strings ###
startTestSuite "Strings"

performTest "echo Wo'r'ld" "World" $prompt
performTest "echo Wo\"r\"ld" "World" $prompt
performTest "echo \\\"Hello"       "\"Hello"                 $prompt
performTest "echo \"Hell'o\""      "Hell'o"                  $prompt
performTest "echo \\\"Hell\"o\\\"" "sane: string not closed" $prompt
performTest "echo Hell'o"          "sane: string not closed" $prompt
performTest "echo \\\"Hell'o\\\""  "sane: string not closed" $prompt
performTest "echo \"Hell\"o\""     "sane: string not closed" $prompt
performTest "echo Hello\\ World"   "Hello World"             $prompt
performTest "echo Hello\\" "Hello" $prompt
performTest "echo Hello\\ World\\" "Hello World" $prompt
performTest "echo \"Hello\"o\"p\"" "Helloop" $prompt
performTest "echo \"Hello'o'p\"" "Hello'o'p" $prompt
performTest "echo \\\"Hello\"o\"p\\\"" "\"Helloop\"" $prompt
performTest "echo \\\"Hello'o'p\\\"" "\"Helloop\"" $prompt
performTest "echo \"Hello\\\"o\\\"p\"" "Hello\"o\"p" $prompt
performTest "echo \\\"Hello\\\"o\\\"p\\\"" "\"Hello\"o\"p\"" $prompt
performTest "echo 'Hell'o'p'" "Hellop" $prompt
performTest "echo 'Hell\"o\"p'" "Hell\"o\"p" $prompt
performTest "echo \\'Hello'o'p\\'" "'Helloop'" $prompt
performTest "echo \\'Hello\"o\"p\\'" "'Helloop'" $prompt
performTest "echo \"Hello\"o\"p\"hole\" Wo\"r\"ld\"" "Helloophole World" $prompt
performTest "echo \"Hello'o'p'hole' Wo'r'ld\"" "Hello'o'p'hole' Wo'r'ld" $prompt
performTest "echo \\\"Hello\"o\"p\"hole\" Wo\"r\"ld\\\"" "\"Helloophole World\"" $prompt
performTest "echo \\\"Hello'o'p'hole' Wo'r'ld\\\"" "\"Helloophole World\"" $prompt
performTest "echo \"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\"" "Hello\"o\"p\"hole\" World" $prompt
performTest "echo \\\"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\\\"" "\"Hello\"o\"p\"hole\" World\"" $prompt
performTest "echo 'Hell'o'p'hole' Wo'r'ld'" "Hellophole World" $prompt
performTest "echo 'Hell\"o\"p\"hole\" Wo\"r\"ld'" "Hell\"o\"p\"hole\" Wo\"r\"ld" $prompt
performTest "echo \\'Hello'o'p'hole' Wo'r'ld\\'" "'Helloophole World'" $prompt
performTest "echo \\'Hello\"o\"p\"hole\" Wo\"r\"ld\\'" "'Helloophole World'" $prompt

endTestSuite

### Prompt ###
startTestSuite "Prompt"

performTest "prompt >" "sane: syntax error, expected path after token '>'" $prompt  
performTest "prompt \\>" "" "> "
performTest "prompt \"orchid $\"" "" "orchid $ "
performTest "prompt %" "" "% "

endTestSuite

### Marking Guide ###
# startTestSuite "Marking Guide"

# performTest "" "" $prompt 
# # performTest "" "" $prompt 

# endTestSuite

# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
