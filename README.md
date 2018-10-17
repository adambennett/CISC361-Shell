# CISC361-Shell
C Shell

          A simple shell written in C and designed to be run in a terminal on a Linux system. Created for two   
          
          projects as a part of CISC361-Operating Systems at the University of Delaware.
          
          Written between September 14th and October 17th, 2018.

TODO


          Assignment 2 leftovers

          -Add more error checks, make sure no system calls go without a check

          -Absolute path handling improvements (./ls should work inside /bin for example)

          -Wildcard commands cause memory leaks
          
          -Where uses commandFind() so commandSet() needs to be changed, remove bool for break checking
	  
          -Check plumber() for redudant bool checking one of the arrays for freeing

          -Does envSet return a bunch of NULL values that get stored in **returnPtr?

          -Improve expand() to handle wildcards anywhere on the commandline. Need a function to check the index of the wildcard arg, move the other args around and then refill the end of the commandline with wild args.

          Assignment 3

          -Fix watchuser leaks. Add logoff alerts (keep track of usersIDs?)
	  
	  -Should userList member 'watch' be a bool? Do we even need user and watch?
	  
          -Fix watchmail leaks
	  
          -Fix redirection leaks

          -Implement IPC

          -Document assignment 3 code
