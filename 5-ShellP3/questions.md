1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation ensures that all child processes are completed before the shell continues to accept user input because I used waitpid on every child process created by fork. That basically waits until all piped commands are finished before new inputs. If I have forgotten to call waitpid, it might move on too soon. Also, might create zombies as they might stick around in the child processes. 

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

It is necessary to close unused pipe ends after calling dup2 because the original ones stay open. If you leave a pipe open, it can cause you to run out of file descriptors because the pipe might not signal that there is no more data.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The command cd is implemented as a built-in rather than an external command because it needs to change the working directory of the shell process itself. The challenges that would arise if cd were implemented as an external process are that the dictionary of the parent shell wouldn't change, and it would run in a child process.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To modify my implementation to allow that, I would have to allocate memory dynamically, like using realloc. The pros to using this are that it makes it more flexible, but adds challenges because memory will have to be managed more carefully. 