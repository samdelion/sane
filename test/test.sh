#!/usr/bin/expect -f
set timeout 2 
log_user 0
set numTests 0
set currTestSuiteName "None"
set verbose "false"

### Procedures ###
proc performTest { testInput expectedOutput prompt purpose } {
    global numTests
    global verbose
    global currTestSuiteName

    if { $verbose == "true" } {
        send_user "\n"
        send_user "\[-----------------\]\n"
        # send_user "\[$currTestSuiteName\]\n\[$testInput\]\n\[Expect: $expectedOutput\]"
        send_user "\[ Test suite      \]: $currTestSuiteName\n"
        send_user "\[ Purpose         \]: $purpose\n"
        send_user "\[ Input           \]: $testInput\n"
        send_user "\[ Expected output \]: $expectedOutput\n"
        send_user "\n$prompt "
        send "$testInput\r"
        expect {
            timeout {
                send_user "\n\[-----------------\] FAIL\n\n"
                exit 1;
            }
            $expectedOutput
        }
        expect {
            send_user "\n\[-----------------\] FAIL\n\n"
            $prompt
        }
        send_user "\n\[-----------------\] PASS\n"
    } else {
        send_user "\[ RUN      ] $testInput \n"
        send "$testInput\r"
        expect {
            timeout { send_user "\[     FAIL ] $testInput\n"; exit 1; }
            $expectedOutput
        }
        expect {
            timeout { send_user "\[     FAIL ] $testInput\n"; exit 1; }
            $prompt
        }
        send_user "\[       OK ] $testInput\n"
    }

    # Increment number of successful tests
    set numTests [expr $numTests + 1]
}

proc startTestSuite { testSuiteName } {
    global currTestSuiteName
    global numTests
    global verbose

    if {$numTests != 0} {
        send_user "ERROR: Test suite: \"$currTestSuiteName\" wasn't ended! Call 'endTestSuite'\n"
        exit 1;
    }

    if { $verbose == "false" } {
        send_user "\[\-\-\-\-\-\-\-\-\-\-\] Test suite: $testSuiteName\n"
    } else {
        send_user "\n\nStart test suite: $testSuiteName\n"
        send_user "\[=================\]"
        # send_user "\[\-\-\-\-\-\-\-\-\-\-\] Test suite: $testSuiteName\n"
    }

    set currTestSuiteName $testSuiteName
}

proc endTestSuite { } {
    global numTests
    global currTestSuiteName
    global verbose

    if { $verbose == "false" } {
        send_user "\[\-\-\-\-\-\-\-\-\-\-\] $numTests tests completed successfully from test suite: $currTestSuiteName\n\n"
    } else {
        send_user "\[=================\]\n"
        send_user "$numTests tests completed successfully from test suite: $currTestSuiteName\n\n"
    }

    # Reset number of tests
    set numTests 0
    set currTestSuiteName "None"
}

proc setVerbose { boolean } {
    global verbose 

    if {$boolean == "true"} {
        log_user 1
        set verbose "true"
    }
}
### Setup ###
set argument [lindex $argv 0]

if {$argument == "v"} {
    setVerbose "true"
}

if { $verbose == "false" } {
    send_user "\[\=\=\=\=\=\=\=\=\=\=\] Running tests\n"
    send_user "\[\-\-\-\-\-\-\-\-\-\-\] Test environment setup... "
}

spawn ../bin/sane
expect {
    timeout { send_user "FAIL: Startup failed\n"; exit 1; }
    "%"
}

send_user "Done\n"

set prompt "%"

### Strings ###
startTestSuite "Strings"

# performTest "echo Wo'r'ld" "World" $prompt
# performTest "echo Wo\"r\"ld" "World" $prompt
# performTest "echo \\\"Hello"       "\"Hello"                 $prompt
# performTest "echo \"Hell'o\""      "Hell'o"                  $prompt
# performTest "echo \\\"Hell\"o\\\"" "sane: string not closed" $prompt
# performTest "echo Hell'o"          "sane: string not closed" $prompt
# performTest "echo \\\"Hell'o\\\""  "sane: string not closed" $prompt
# performTest "echo \"Hell\"o\""     "sane: string not closed" $prompt
# performTest "echo Hello\\ World"   "Hello World"             $prompt
# performTest "echo Hello\\" "Hello" $prompt
# performTest "echo Hello\\ World\\" "Hello World" $prompt
# performTest "echo \"Hello\"o\"p\"" "Helloop" $prompt
# performTest "echo \"Hello'o'p\"" "Hello'o'p" $prompt
# performTest "echo \\\"Hello\"o\"p\\\"" "\"Helloop\"" $prompt
# performTest "echo \\\"Hello'o'p\\\"" "\"Helloop\"" $prompt
# performTest "echo \"Hello\\\"o\\\"p\"" "Hello\"o\"p" $prompt
# performTest "echo \\\"Hello\\\"o\\\"p\\\"" "\"Hello\"o\"p\"" $prompt
# performTest "echo 'Hell'o'p'" "Hellop" $prompt
# performTest "echo 'Hell\"o\"p'" "Hell\"o\"p" $prompt
# performTest "echo \\'Hello'o'p\\'" "'Helloop'" $prompt
# performTest "echo \\'Hello\"o\"p\\'" "'Helloop'" $prompt
# performTest "echo \"Hello\"o\"p\"hole\" Wo\"r\"ld\"" "Helloophole World" $prompt
# performTest "echo \"Hello'o'p'hole' Wo'r'ld\"" "Hello'o'p'hole' Wo'r'ld" $prompt
# performTest "echo \\\"Hello\"o\"p\"hole\" Wo\"r\"ld\\\"" "\"Helloophole World\"" $prompt
# performTest "echo \\\"Hello'o'p'hole' Wo'r'ld\\\"" "\"Helloophole World\"" $prompt
# performTest "echo \"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\"" "Hello\"o\"p\"hole\" World" $prompt
# performTest "echo \\\"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\\\"" "\"Hello\"o\"p\"hole\" World\"" $prompt
# performTest "echo 'Hell'o'p'hole' Wo'r'ld'" "Hellophole World" $prompt
# performTest "echo 'Hell\"o\"p\"hole\" Wo\"r\"ld'" "Hell\"o\"p\"hole\" Wo\"r\"ld" $prompt
# performTest "echo \\'Hello'o'p'hole' Wo'r'ld\\'" "'Helloophole World'" $prompt
# performTest "echo \\'Hello\"o\"p\"hole\" Wo\"r\"ld\\'" "'Helloophole World'" $prompt
# # * Check if string not closed works correctly
# performTest "echo \"Hello" "sane: string not closed" $prompt

endTestSuite

### Prompt ###
startTestSuite "Prompt"

# performTest "prompt >" "sane: syntax error, expected path after token '>'" $prompt  
# performTest "prompt \">\"" "" "> "
# performTest "prompt \\>" "" "> "
# performTest "prompt \"orchid $\"" "" "orchid $ "
# performTest "prompt myshell" "" "myshell" 
# performTest "prompt %" "" "% "

endTestSuite

### Pipes ###

startTestSuite "Pipes"
endTestSuite

### Redirection ###

### Builtins
startTestSuite "Builtins"

# performTest "pwd" "*/test" $prompt
# performTest "pwd | cat | grep \"test\"" "*/test" $prompt
# performTest "pwd | grep \"test\"" "*/test" $prompt
# performTest "cd test ; pwd" "cd: No such file or directory" $prompt
# # * Bash ignores other arguments to cd - expect no error
# performTest "cd test folderThatDoesNotExist ; pwd" "*/test" $prompt
# performTest "cd test folderThatDoesNotExist1 folderThatDoesNotExist2 ; pwd" "*/test" $prompt
# # performTest "cd ; pwd" "$env(HOME)" $prompt

endTestSuite

### Wildcards ###
startTestSuite "Wildcards"

# performTest "ls folder2/foo?.c" "folder2/foo1.c\tfolder2/foo2.c" $prompt
# performTest "ls folder2/foo*.c" "folder2/foo1.c\tfolder2/foo2.c\tfolder2/foo33.c" $prompt
# performTest "ls folder2/foo*" "folder2/foo1.c\tfolder2/foo2.c\tfolder2/foo33.c\tfolder2/foo4" $prompt
# performTest "ls folder2/abc.?" "folder2/abc.c\tfolder2/abc.x" $prompt
# performTest "ls folder2/abc*.?" "folder2/abc.c\tfolder2/abc.x\tfolder2/abc33.c" $prompt
# performTest "ls folder2/*" "folder2/abc.c[ ]*folder2/abc33.c[ ]*folder2/afoo[ ]*folder2/foo2.c[ ]*folder2/foo4[ ]*folder2/abc.x[ ]*folder2/abc33.cc[ ]*folder2/foo1.c[ ]*folder2/foo33.c[ ]*folder2/hfoo" $prompt
# # * Make sure wildcard doesn't expand when inside string
# performTest "ls \"folder2/*\"" "ls: folder2/*: No such file or directory" $prompt
# performTest "cat folder3/names.txt | grep \"Hello\"" "Hello World" $prompt 
# performTest "cat folder3/names.txt | grep 'Hello'" "Hello World" $prompt 

endTestSuite

### Marking Guide ###
# startTestSuite "Marking Guide"

# performTest "ls folder1" "foo1\tfoo2\tfoo3" $prompt
# performTest "exit" "" $prompt
# # performTest "" "" $prompt 

# endTestSuite

# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt
# performTest "" "" $prompt

### Miscellaneous ###
startTestSuite "Misc"

performTest\
    "|"\
    "sane: first token is command separator"\
    $prompt\
    "Check that shell throws an error if first token is a '|' command separator"
performTest\
    "&"\
    "sane: first token is command separator"\
    $prompt\
    "Check that shell throws an error if first token is a '&' command separator"
performTest\
    ";"\
    "sane: first token is command separator"\
    $prompt\
    "Check that shell throws an error if first token is a ';' command separator"
performTest\
    "echo Hello |"\
    "sane: last command followed by command separator '|'"\
    $prompt\
    "Check that an error is thrown if user tries to pipe last command."
performTest\
    "echo Hello | | cat"\
    "sane: at least two successive commands are separated by more than one command separator"\
    $prompt\
    "Check that the two successive command error works as expected for the '|' separator."
performTest\
    "echo Hello & & cat"\
    "sane: at least two successive commands are separated by more than one command separator"\
    $prompt\
    "Check that the two successive command error works as expected for the '&' separator."
performTest\
    "echo Hello ; ; cat"\
    "sane: at least two successive commands are separated by more than one command separator"\
    $prompt\
    "Check that the two successive command error works as expected for the ';' separator."

endTestSuite
