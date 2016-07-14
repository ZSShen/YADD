[![Build Status](https://travis-ci.org/ZSShen/YADD.svg?branch=master)](https://travis-ci.org/ZSShen/YADD)


## **Objective**
Currently, this project is for self-studying purpose. But in essence, YADD is planed to be a complex disassembler for the Android Dex bytecode.  That is, a hybrid tool to support pure binary/signature dumping and to provide an interface for reversing analysis.  

As a reversing toolkit, YADD will basically support:  
+ Code block differentiation and control flow visualization.  
+ Symbolic level def-use chain to highlight the data dependency between instructions.  

## **Current Progress**
YADD is now `relying on the Dex file parsing and the instruction decoding algorithm provided by Android Open Source Project`.  It can now be built as a `independent executable for Dex code disassembling and signature dumping`. More features about control and data flow analysis will be updated in the near future.  

## **Installation**
Clone the project to your working directory.  
In the working directory, type the following commands.  
```
$ ./clean.py --rebuild
$ cd build
$ cmake ..
$ make
```
Done! And the executable should locate at:
`/PATH/TO/YOUR/WORKING/DIRECTORY/bin/dumper`

## **Usage**
```
Usage: dumper [options]
    Example: dumper --granularity=instruction --input=/PATH/TO/MY/DEX --output=PATH/TO/MY/LOG

  --granularity=(class|method|instruction): For data granularity
    class      : List class names only
    method     : List method signatures only
    instruction: Full dump

  --input=<classes.dex>: Specify the input dex pathname

  --output=<dump.txt>: Specify the output dump pathname

```

## **Contact**
Any problems? please contact me via the mail: andy.zsshen@gmail.com  
