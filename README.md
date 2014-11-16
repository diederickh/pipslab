
# Light Writer Font Tool

Simple font editor for Light Writer project with Pipslab for the "KWF Kankerbestrijding". This font
editor was used to generate a hand written font that we used to draw light strokes using a ABB 
IRB 140 robot. 

<img src="https://farm9.staticflickr.com/8575/15618078729_fb04d06529_o.png" alt="Light Writer">

A list stroke created with the IRB:
<img src="https://farm8.staticflickr.com/7467/15619109040_52719c6280_o.png" alt="ABB stroke">

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
$ ./release_x86.sh 32    # for a 32 bit build
$ ./release_x86.sh 64    # for a 64 bit build
````

## Todo

````sh
[x] Socket::send(), when sending fails we exit(), need to handle this.
[x] Make sure to validate the min_x, max_x, min_y, min_y
````