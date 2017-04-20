# sane

![sane_black](https://www.dropbox.com/s/zqvccdjgq22xhm7/Screen%20Shot%202016-10-15%20at%204.28.27%20pm.png?dl=0)

The **S**amuel Evans-Powell and Nathan G**ane** shell.

Current Features:
- Standard input and output redirection
- Pipelining
- Background job execution
- Sequential job execution
- Shell builtin command support
- Zombie process reaping
- Proper handling of slow system calls

## User Guide
### Tests
- Test suite and all test cases are found in test/
- Assuming you've built the shell and the 'sane' executable is found in bin/,
you can execute the test suite by running:
 ```
 cd test
 ./test.sh
 ```
- By default, clean output is shown, to see more verbose output, execute the
test suite like so:
```
cd test
./test.sh v
```
