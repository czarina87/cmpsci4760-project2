https://github.com/czarina87/cmpsci4760-project2
 
OSS & Worker Program - 

This project contains two C programs: oss (Operating System Simulator) and worker. The `oss program forks multiple `worker` processes that run concurrently,
 simulating how an operating system schedules it's processes. The `worker` processes report back to `oss` once they finish execution.

Compilation - 

Use the make to compile the programs. 
Navigate to the directory containing the source code, then compile the programs by running 'make'.

 Running the Program - 

After compiling, you run the program by using the following command - example:

bash
./oss -n 5 -s 2 -t 5 -i 1000


By default, the program uses predefined values for various parameters"
number of children to run simultaneously(`-s`)
number of child processes to launch (`-n`)
maximum time a child process will run (`-t`)
and interval for launching child processes (`-i`).


(`-h`) This will display more detailed usage instructions.

