1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

The remote client determines that by searching for a unique end-of-message indicator that the server incorporates, in our case the EOF character 0x04. Among the methods include looping on recv() until the entire message is received and employing things like length prefixes. 

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

The shell protocol should add its own EOF marker for responses to prompt message boundaries. There is a high chance of misinterpretation because partial reads or multiple messages might merge.

3. Describe the general differences between stateful and stateless protocols.

The difference between those two is stateful protocols keep session context between requests, they are more useful for continuous sessions. On the other hand, stateless protocols make each request independently, they are scaled easily and are simpler to manage.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

We use this because low latency is critical. In other words, this is used for things like gaming when it has minimal overhead. If needed, it can tolerate some loss or implement its own reliability.

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

The interface/abstraction provided by the operating system is socket API, which enables applications to create, connect, send, and receive data over networks.