# CISC361-Shell
C Shell

          A simple shell written in C and designed to be run in a terminal on a Linux system. Created for two   
          
          projects as a part of CISC361-Operating Systems at the University of Delaware.
          
          Written between September 14th and October 14th, 2018.

TODO


          Assignment 2 leftovers

          -Add more error checks, make sure no system calls go without a check

          -Absolute path handling improvements (./ls should work inside /bin for example)

          -Wildcard commands cause memory leaks
          
          -Where uses commandFind() so commandSet() needs to be changed, remove bool for break checking
	  
          -Check plumber() for redudant bool checking one of the arrays for freeing

          -Does envSet return a bunch of NULL values that get stored in **returnPtr?

          -headRef() comment doc needs rewording

          -refreshPath() comment doc should mention that pathlist needs to be already allocated with spaces for new members

          -Shorten hasWildcards comment brief

          -Improve expand() to handle wildcards anywhere on the commandline. Need a function to check the index of the wildcard arg, move the other args around and then refill the end of the commandline with wild args.

          -Comment changeDirectory() further to explain setenv() statements and how it works now


          Assignment 3

          -Backgrounding not implemented properly, error in waitpid shouldnt be thrown on normal commands

          -Test watchuser, fix associated leaks
	  
	-Should userList member 'watch' be a bool? Do we even need user and watch?

          -Implement watchmail command

          -Implement redirection operators

          -Implement noclobber

          -Implement IPC

          -Document assignment 3 code
