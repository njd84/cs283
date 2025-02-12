1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice for this application because it reads an entire line at a time and matches the way users typically enter commands in a shell. Using fgets() also helps with errors like buffer overflows. 

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  You need to use malloc to allocate memory instead of allocating a fixed-size array because now it can allocate the buffer on the heap and not on the stack. As this assignment is dealing with large buffer sizes using malloc is better because it helps avoid things like stack overflow and it gives you the option to adjust the buffer. 


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**: This is necessary because it is important to correctly parse commands. If you were to leave the spaces and not trimming it could cause issues like misinterpreted part of the command or its arguments which can lead to misrecognized commands and empty arguments. 


4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**: The three redirection examples that we should implement in our custom shell are output redirection, input redirection, and appending output. The challenges we might face in implementing output redirection are ensuring that files open correctly with the right permissions, managing file descriptors effectively, and dealing with possible I/O problems are all important tasks to do properly. In implementing the input redirection, it's important to open the file for reading, check that it actually exists, and make sure that the command is reading from the right file descriptor. Lastly for the appending output implementation, opening the file in append mode, ensuring that the file pointer has the correct position at the end of the file, and handling multiple writes if the shell allows multiple processes.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**: The key difference between redirection and piping is that piping connects the output of one command to the input of another by using the | operator. It sets up a sequence of processes to operate in real time, sending data across in-memory buffers, which is good for processing data on the spot without previously saving it. Redirection sends data to or from files or devices, using > directs a command's output to a file, whereas < receives input from a file, mostly for data storage. It normally operates on an individual basis, connecting a command's output directly to a file.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  It is important to keep these separate in a shell because redirecting them on their own such as saving regular output to a file while errors are displayed onscreen. Another thing is that it avoids combining error signals with regular output, which is important for data cleanliness, particularly when chaining commands.

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**: In our custom shell should handle errors efficiently by tracking both STDOUT and STDERR, allowing users to tell the difference between regular output and error messages. For commands that output to both streams, having the ability to merge them might be useful. Users can achieve this using syntax such as " 2>&1 ", which switches STDERR to STDOUT, resulting in a simplified view when necessary.