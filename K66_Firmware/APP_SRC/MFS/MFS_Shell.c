// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2015.10.30
// 10:19:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "App.h"
#include "shell.h"



const SHELL_COMMAND_STRUCT Shell_commands[] = {
  { "cd",        Shell_cd     },
  { "copy",      Shell_copy   },
  { "create",    Shell_create },
  { "del",       Shell_del },
  { "di",        Shell_di },
  { "disect",    Shell_disect},
  { "dir",       Shell_dir },
  { "df",        Shell_df },
  { "exit",      Shell_exit },
  { "format",    Shell_format },
  { "help",      Shell_help },
  { "mkdir",     Shell_mkdir },
  { "pwd",       Shell_pwd },
  { "read",      Shell_read },
  { "ren",       Shell_rename },
  { "rmdir",     Shell_rmdir },
  { "sh",        Shell_sh },
  { "type",      Shell_type },
  { "write",     Shell_write },
  { "rdtst",     Shell_read_test},
  { "wrtst",     Shell_write_test},
  { "?",         Shell_command_list },
  { NULL,        NULL }
};

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
void Task_shell(uint32_t temp)
{
  (void)temp; /* suppress 'unused variable' warning */

  /* Run the shell on the serial port */
  printf("MFS Shell\n");
  for (;;)
  {
    Shell(Shell_commands, NULL);
    printf("Shell exited, restarting...\n");
  }
}
