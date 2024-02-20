# OSS & Worker Programs

This project contains two C-based programs: `oss` (Operating System Simulator) and `worker`. The `oss` program forks multiple `worker` processes that run concurrently, simulating how an operating system schedules processes. The `worker` processes report back to `oss` once they finish execution.

## Compilation

Use the make to compile the programs. 
Navigate to the directory containing the source code, then compile the programs by running 'make'

## Running the Program

After compilation, run the program by using the following command:
bash
./oss

This will start the `oss` program, which will subsequently spawn `worker` processes according to its scheduling logic.

By default, the program uses predefined values for various parameters"
number of children to run simultaneously(`-s`)
number of child processes to launch (`-n`)
maximum time a child process will run (`-t`)
and interval for launching child processes (`-i`).


(`-h`) This will display more detailed usage instructions.