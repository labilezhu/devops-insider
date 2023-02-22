# GNU Make

## Skills

### Making make print commands before executing

> [https://stackoverflow.com/questions/11004695/making-make-print-commands-before-executing-when-not-using-cmake](https://stackoverflow.com/questions/11004695/making-make-print-commands-before-executing-when-not-using-cmake)



By default, `make` does print every command before executing it. This printing can be suppressed by one of the following mechanisms:

- on a case-by-case basis, by adding `@` at the beginning of the command
- globally, by adding the .SILENT built-in target.
- somewhere along the make process, by invoking sub-make(s) with one of the flags `-s`, `--silent` or `--quiet`, as in `$(MAKE) --silent -C someDir`, for example. From that moment on, command echoing is suppressed in the sub-make.

If your makefile does not print the commands, then it is probably using one of these three mechanisms, and you have to actually inspect the makefile(s) to figure out which.

As a workaround to avoid these echo-suppressing mechanisms, you could re-define the shell to be used to use a debug mode, for example like `make SHELL="/bin/bash -x" target`. Other shells have similar options. With that approach, it is not `make` printing the commands, but the shell itself.

If you use the flag `-n` or `--just-print`, the echo-suppressing mechanisms will be ignored and you will always see all commands that `make` thinks should be executed -- but they are not actually executed, just printed. That might be a good way to figure out what you can actually expect to see.

The `VERBOSE` variable has no standard meaning for `make`, but only if your makefile interprets it.



