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

performTest\
    "echo Wo'r'ld"\
    "World"\
    $prompt\
    "Test that the ' character is ignored in an argument."
performTest\
    "echo Wo\"r\"ld"\
    "World"\
    $prompt\
    "Test that the \" character is ignored in an argument."
performTest\
    "echo \\\"Hello"\
    "\"Hello"\
    $prompt\
    "Test that the \" character is not ignored if escaped."
performTest\
    "echo \"Hell'o\""\
    "Hell'o"\
    $prompt\
    "Test that using a single ' character within a string doesn't cause an error."
performTest\
    "echo \\\"Hell\"o\\\""\
    "sane: string not closed"\
    $prompt\
    "Test that the shell complains if all opened strings are not closed."
performTest\
    "echo Hell'o"\
    "sane: string not closed"\
    $prompt\
    "Test that the shell complains if all opened strings are not closed."
performTest\
    "echo \\\"Hell'o\\\""\
    "sane: string not closed"\
    $prompt\
    "Test that the shell complains if all opened strings are not closed."
performTest\
    "echo \"Hell\"o\""\
    "sane: string not closed"\
    $prompt\
    "Test that the shell complains if all opened strings are not closed."
performTest\
    "echo \"Hello"\
    "sane: string not closed"\
    $prompt\
    "Test that the shell complains if all opened strings are not closed."
# performTest\
#     "echo Hello\\ World"\
#     "Hello World"\
#     $prompt\
#     "Test that spaces can be used if escaped."
performTest\
    "echo Hello\\"\
    "Hello"\
    $prompt\
    "Test that a trailing \\ character does not cause an error."
performTest\
    "echo Hello\\ World\\"\
    "Hello World"\
    $prompt\
    "Test that a trailing \\ character does not cause an error."
performTest\
    "echo \"Hello\"o\"p\""\
    "Helloop"\
    $prompt\
    "Test that the \" character is ignored if inside a set of \" characters."
performTest\
    "echo \"Hello'o'p\""\
    "Hello'o'p"\
    $prompt\
    "Test that the ' character is not ignored if inside a set of \" characters."
performTest\
    "echo \\\"Hello\"o\"p\\\""\
    "\"Helloop\""\
    $prompt\
    "Test that the \" character is ignored if inside a set of \\\" characters."
performTest\
    "echo \\\"Hello'o'p\\\""\
    "\"Helloop\""\
    $prompt\
    "Test that the ' character is not ignored if inside a set of \\\" characters."
performTest\
    "echo \"Hello\\\"o\\\"p\""\
    "Hello\"o\"p"\
    $prompt\
    "Test that the \" character is not ignored if escaped and inside a set of \" characters."
performTest\
    "echo \\\"Hello\\\"o\\\"p\\\""\
    "\"Hello\"o\"p\""\
    $prompt\
    "Test that escaped strings work when placed within other escaped strings."
performTest\
    "echo 'Hell'o'p'"\
    "Hellop"\
    $prompt\
    "Test that a ' string is ignored when placed inside another ' string."
performTest\
    "echo 'Hell\"o\"p'"\
    "Hell\"o\"p"\
    $prompt\
    "Test that \" string is not ignored when placed inside a ' string."
performTest\
    "echo \\'Hello'o'p\\'"\
    "'Helloop'"\
    $prompt\
    "Test escaping of \' string."
performTest\
    "echo \\'Hello\"o\"p\\'"\
    "'Helloop'"\
    $prompt\
    "Test ignoring of \" string."
performTest\
    "echo \"Hello\"o\"p\"hole\" Wo\"r\"ld\""\
    "Helloophole World"\
    $prompt\
    "Test a more complex example of ignoring \" character."
performTest\
    "echo \"Hello'o'p'hole' Wo'r'ld\""\
    "Hello'o'p'hole' Wo'r'ld"\
    $prompt\
    "Test a more complex example of ' string within \" string."
performTest\
    "echo \\\"Hello\"o\"p\"hole\" Wo\"r\"ld\\\""\
    "\"Helloophole World\""\
    $prompt\
    "Test a more complex example of ignoring \" character inside \\\" string."
performTest\
    "echo \\\"Hello'o'p'hole' Wo'r'ld\\\""\
    "\"Helloophole World\""\
    $prompt\
    "Test a more complex example of ignoring \" character inside ' string."
performTest\
    "echo \"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\""\
    "Hello\"o\"p\"hole\" World"\
    $prompt\
    "Test more complex example of escaping string characters."
performTest\
    "echo \\\"Hello\\\"o\\\"p\\\"hole\\\" Wo\"r\"ld\\\""\
    "\"Hello\"o\"p\"hole\" World\""\
    $prompt\
    "Test more complex example of escaping string characters."
performTest\
    "echo 'Hell'o'p'hole' Wo'r'ld'"\
    "Hellophole World"\
    $prompt\
    "Test a more complex example of ignoring the ' character inside a ' string."
performTest\
    "echo 'Hell\"o\"p\"hole\" Wo\"r\"ld'"\
    "Hell\"o\"p\"hole\" Wo\"r\"ld"\
    $prompt\
    "Test a more complex example of not ignoring the \" character inside a ' string."
performTest\
    "echo \\'Hello'o'p'hole' Wo'r'ld\\'"\
    "'Helloophole World'"\
    $prompt\
    "Test a more complex example of ignoring the ' character inside an escaped \' string."
performTest\
    "echo \\'Hello\"o\"p\"hole\" Wo\"r\"ld\\'"\
    "'Helloophole World'"\
    $prompt\
    "Test a more complex example of ignoring the \" character inside an escaped \' string."

endTestSuite

### Prompt ###
startTestSuite "Prompt"

performTest\
    "prompt >"\
    "sane: syntax error, expected path after token '>'"\
    $prompt\
    "Test that a single '>' is interpreted as a redirection operator and hence throws error."
performTest\
    "prompt \">\""\
    ""\
    "> "\
    "Test that placing the '>' character in \" quotes allows a '>' prompt to be created."
performTest\
    "prompt '>'"\
    ""\
    "> "\
    "Test that placing the '>' character in ' quotes allows a '>' prompt to be created."
performTest\
    "prompt \\>"\
    "" "> "\
    "Test that escaping the '>' character allows a '>' prompt to be created."
performTest\
    "prompt \"orchid $\""\
    ""\
    "orchid $ "\
    "Test a multiple argument prompt."
performTest\
    "prompt %"\
    ""\
    "% "\
    "Restore prompt."

endTestSuite

### Builtins
startTestSuite "Builtins"

performTest\
    "pwd"\
    "*/test"\
    $prompt\
    "Test that pwd works."
performTest\
    "pwd | cat | grep \"test\""\
    "*/test"\
    $prompt\
    "Test that builtins work in a pipeline."
performTest\
    "pwd | grep \"test\""\
    "*/test"\
    $prompt\
    "Test that builtins work in a pipeline"
performTest\
    "cd testFolderThatDoesntExist ; pwd"\
    "cd: No such file or directory"\
    $prompt\
    "Test that cd complains if no directory to cd to."
performTest\
    "cd test folderThatDoesNotExist ; pwd"\
    "*/test"\
    $prompt\
    "Test that cd ignores arguments other than first (like bash)."
performTest\
    "cd test folderThatDoesNotExist1 folderThatDoesNotExist2 ; pwd"\
    "*/test"\
    $prompt\
    "Test that cd ignores multiple arguments other than first."
# performTest "cd ; pwd" "$env(HOME)" $prompt "Test that cd with no arguments work."

endTestSuite

### Wildcards ###
startTestSuite "Wildcards"

performTest\
    "ls folder2/foo?.c"\
    "folder2/foo1.c[ ]*folder2/foo2.c"\
    $prompt\
    "Test that '?' wildcard character works."
performTest\
    "ls folder2/foo*.c"\
    "folder2/foo1.c[ ]*folder2/foo2.c[ ]*folder2/foo33.c"\
    $prompt\
    "Test that '*' wildcard character works."
performTest\
    "ls folder2/foo*"\
    "folder2/foo1.c[ ]*folder2/foo2.c[ ]*folder2/foo33.c[ ]*folder2/foo4"\
    $prompt\
    "Test that '*' wildcard character works."
performTest\
    "ls folder2/abc.?"\
    "folder2/abc.c[ ]*folder2/abc.x"\
    $prompt\
    "Test that '?' wildcard character works."
performTest\
    "ls folder2/abc*.?"\
    "folder2/abc.c[ ]*folder2/abc.x[ ]*folder2/abc33.c"\
    $prompt\
    "Test that the '?' and '*' wildcard characters work in combination."
performTest\
    "ls \"folder2/*\""\
    "ls: folder2/*: No such file or directory"\
    $prompt\
    "Make sure wildcard doesn't expand when inside string"
# performTest\
#     "ls folder2/*"\
#     "folder2/abc.c[ ]*folder2/abc33.c[ ]*folder2/foo2.c[ ]*folder2/hfoo[ ]*folder2/abc.x[ ]*folder2/afoo[ ]*folder2/foo33.c[ ]*folder2/abc33.c[ ]*folder2/foo1.c[ ]*folder2/foo4[ ]*"\
#     $prompt\
#     "Test that '*' wildcard character works with large number of files."

endTestSuite

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
set timeout 5
performTest\
    "sleep 1 ; echo hello"\
    "hello"\
    $prompt\
    "Ensure that sequential execution works as expected."
performTest\
    "sleep 1 ; ls -l folder1"\
    "[ ]*foo1\r\n[ ]*foo2\r\n[ ]*foo3"\
    $prompt\
    "Ensure that sequential execution works as expected."
performTest\
    "sleep 1 ; echo hello1 ; sleep 1 ; echo hello2"\
    "hello1\r\nhello2"\
    $prompt\
    "Ensure that sequential execution works as expected with multiple sleeps."
set timeout 2 
performTest\
    "sleep 10 & echo hello"\
    "hello"\
    $prompt\
    "Test that background execution works (shell shouldn't wait for sleep to execute before executing the echo command)"
performTest\
    "ls folder2/abc.c folder2/abc.x folder2/abc33.c folder2/abc33.cc"\
    "folder2/abc.c[ ]*folder2/abc.x[ ]*folder2/abc33.c[ ]*folder2/abc33.cc"\
    $prompt\
    "Test a command with a large number of parameters"
# performTest\
#     "exit"\
#     ""\
#     $prompt\
#     "Test that the exit command works."

endTestSuite

### Redirection ###
startTestSuite "Redirection"

performTest\
    "cat < folder3/names.txt"\
    "Betty\r\nAardvark\r\nHello World"\
    $prompt\
    "Test that standard input redirection works."
performTest\
    "grep Betty < folder3/names.txt"\
    "Betty"\
    $prompt\
    "Test that standard input redirection works."
performTest\
    "cat < folder3/names.txt > folder4/names.txt ; cat < folder4/names.txt"\
    "Betty\r\nAardvark\r\nHello World"\
    $prompt\
    "Test that standard output redirection works."
performTest\
    "grep Betty < folder3/names.txt > folder4/betty.txt ; cat < folder4/betty.txt"\
    "Betty"\
    $prompt\
    "Test that standard input redirection works."

endTestSuite

### Pipes ###

startTestSuite "Pipes"

performTest\
    "cat folder3/names.txt | cat"\
    "Betty\r\nAardvark\r\nHello World"\
    $prompt\
    "Test that a simple shell pipeline works."
performTest\
    "cat folder3/names.txt | grep Aardvark"\
    "Aardvark"\
    $prompt\
    "Test that a simple shell pipeline works."
performTest\
    "cat folder3/names.txt | sort"\
    "Aardvark\r\nBetty\r\nHello World"\
    $prompt\
    "Test that a simple shell pipeline works."
performTest\
    "cat folder3/names.txt | sort -r"\
    "Hello World\r\nBetty\r\nAardvark"\
    $prompt\
    "Test that a simple shell pipeline works."
performTest\
    "cat folder3/names.txt | sort | sort -r | grep Hello"\
    "Hello World"\
    $prompt\
    "Test that a longer shell pipeline works."

endTestSuite
