// Forward declaration
struct command_t;

////////////////////////////////////////////////////////////////////////////////
/// Execute the commands found in the command array.
///
/// @param   numCommands   int, number of commands in the command array.
/// @param   commands      command_t, command array.
////////////////////////////////////////////////////////////////////////////////
void sane_execute(int numCommands, struct command_t *commands);

