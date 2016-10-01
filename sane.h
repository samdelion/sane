struct command_t;

////////////////////////////////////////////////////////////////////////////////
// Execute the commands found in the command array.
//
// @param   numCommands   int, number of commands in the command array.
// @param   commands      command_t, command array.
// @return                int, 1 for success, 0 in case of error or quit.
////////////////////////////////////////////////////////////////////////////////
int sane_execute(int numCommands, command_t *commands);

// void sane_loop(void);

