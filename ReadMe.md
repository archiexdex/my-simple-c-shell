Environment
-----------
gcc (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609

Install
-------

Please put the project in linux base environment.
It is very important thing!!!!!!!!!

Just put command `make` then you will get `hw3`

How to run
----------
command `./hw3`
then you will enter my shell 

Example 
-------
If you want to leave the shell, you can use Ctrl-\

Execute a single command: 
`ls -la`

Properly block or unblock signals: Sending SIGINT or SIGQUIT should not terminate the current running shell.
`sleep 10`
then you can use ^Z or ^C to suspend or close the process

Replace standard output of a process:
`ls -la > tmp`

Replace standard input of a process:
`cat < /etc/passwd`

Setup foreground process group and background process groups:
`less /etc/passwd`

Create pipelines:
`cat /etc/passwd | cat | less`

Put processes into the same process group:
`ps -o pid,sid,pgid,ppid,cmd | cat | cat | tr A-Z a-z`

Manipulate environment variables: 
`export a=b`
`unset a`

Expand * and ? characters:
`ls *`

Job control: 
`jobs`
then you will see how many background jobs you have now
`bg`
`bg #1`
then you can run the suspend background process from suspend to continue state
`fg`
`fg #1`
then you can get background process to foreground
