# osh
osh is a simple shell in C. I created it by following [this tutorial](https://brennan.io/2015/01/16/write-a-shell-in-c/) for learning how a shell works.

## Limitations
Since it's simple there are limitations that I've just copied and pasted from the original's [github repo](https://github.com/brenns10/lsh/tree/407938170e8b40d231781576e05282a41634848c).
* Commands must be on a single line.
* Arguments must be separated by whitespace.
* No quoting arguments or escaping whitespace.
* No piping or redirection.
* Only builtins are: cd, help, exit.