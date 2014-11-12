
# Light Writer Font Tool

Simple font editor for Light Writer project with Pipslab.

<img src="http://i.imgur.com/fWpZEJF.png" alt="Light Writer">


# Getting the source

````sh
$ mkdir fontapp
$ cd fontapp
$ git clone git@github.com:roxlu/pipslab.git .
````

## Compiling and running

The installation works the same on Mac, Linux and Windows. On 
windows make sure you have GIT installed and that you used the 
git bash windows. 

Note: the first time you run `release_x86.sh` it takes a bit 
longer because it will compile and download all necessary 
dependencies.

````sh
$ cd fontapp
$ cd build
$ ./release_x86.sh
````

## Todo

````sh
[x] Socket::send(), when sending fails we exit(), need to handle this.
[ ] Make sure to validate the min_x, max_x, min_y, min_y
````