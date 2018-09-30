# CISC361-Shell
C Shell

TODO

-CD needs cleaning up, double setting dirMem for no reason
	-Shouldn't reset prev if it's equal to current dir?
	-Just rewrite using setenv() and protected env variables (rewrite some of newEnvVar)
  
-Executing non-builtins needs to display message saying what you're running (executing /bin/ls -l for example)

-Absolute path handling improvements (./ls should work inside /bin for example)

-Implement wildcard handling

-Add some extra error checks

-Timeout feature

-Greatly improve documentation

-Handle backspace ?

-Maybe get rid of args? Have to rewrite most of older functions a bit to make sure logic is correct with argsEx
