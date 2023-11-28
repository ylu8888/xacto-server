# xacto-server

The goal of this assignment is to become familiar with low-level POSIX
threads, multi-threading safety, concurrency guarantees, and networking.
The overall objective is to implement a simple multi-threaded
transactional object store.  As you will probably find this somewhat
difficult, to grease the way we have provided you with a design for the server,
as well as binary object files for almost all the modules.  This means that you
can build a functioning server without initially facing too much
complexity.  In each step of the assignment, you will replace one of our
binary modules with one built from your own source code.  If you succeed
in replacing all of our modules, you will have completed your own
version of the server.
It is probably best if you work on the modules in roughly the order
indicated below.  Turn in as many modules as you have been able to finish
and have confidence in.  Don't submit incomplete modules or modules
that don't function at some level, as these will negatively impact
the ability of the code to be compiled or to pass tests.

Takeaways

After completing this homework, you should:

Have a basic understanding of socket programming

Understand thread execution, locks, and semaphores

Have an advanced understanding of POSIX threads

Have some insight into the design of concurrent data structures

Have enhanced your C programming abilities
