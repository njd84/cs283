1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  The shell will turn into the command it is attempting to execute if we simply use execvp directly. This is why it will not function properly. As a result, the shell will disappear as it will no longer exist. The value that the fork provides is that it has the ability to sustain the shell by making a duplicate of itself.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If the fork() system call fails it will return -1. In this scenario we skip that command and print an error message. This was we doing run into major issues like crashing. 

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**: In the event that an absolute path is not provided, PATH becomes important since it informs OS systems where to search for the executable file. Execvp() searches the directories specified in the PATH variable until it locates the corresponding file.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**: The purpose of calling wait() after forking is because it waits for the child's class to be finished. In the case that we don't call it we have a higher possibility of it being a zombie process. 

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**: 
    The information that this provides is that it gets the exit code from the status that is returned by wait(). This is important because this makes it clear if there was an error or not. 

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**: My implementation of build_cmd_buff() looks that the test inside the quotes as a single argument. An example like "hello world" is a single argument and it will not split up in the trust file. 


7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**: The changes that were made in the parsing logic compared to before is that we took off extra spaces and now we are handling quotes correctly. One of the unexpected challenges in refactoring my old code was getting the space and quote to work how I wanted it to work.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**: The purpose of the signals in a Linux system is to inform the processes about the events. They differ because they only send notifications and not data, unlike the rest. 
   

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: The three common signals are SIGKILL, it forces a process to stop right away, SIGTERM requests a shutdown, and SIGINT, this happenes when you press Ctrl-C. 

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**: When a process receives SIGSTOP, it stops the process which cannot be caught or ignored like SIGINT as that is different from what that can handle
