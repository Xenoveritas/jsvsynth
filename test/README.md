This directory contains test cases intended to be run via VirtualDub's
command line interface, which is used to verify if they work. You can either
install VirtualDub into your appropriate Program Files directory (on 64-bit
versions, that's usually `C:\Program Files (x86)`, on 32-bit, it's
`C:\Program Files`), or you can place it anywhere and set the `VDUB_HOME`
environment variable to point to it.

If you opt for the second, instead of setting the environment varible for your
account, you can set it in a special file named `setenv.bat` which will be
invoked before tests are run.

In any case, once you have the environment set up, you can use `runtests.bat`
to run through any AVS file with a file name that ends with "test". Results
of the tests will be stored in a TXT file called `results.txt`.
