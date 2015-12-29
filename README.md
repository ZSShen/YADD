# YADD

[![Join the chat at https://gitter.im/ZSShen/YADD](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/ZSShen/YADD?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
Yet another Android Dex bytecode Disassembler


##Objective
Currently, this project is for self-studying purpose. But in essence, YADD is planed to be a complex disassembler for the Android Dex bytecode.  That is, a hybrid tool to support pure binary/signature dumping and to provide an interface for reversing analysis.  

As a reversing toolkit, YADD will basically support:  
  + Code block differentiation and control flow visualization.
  + Symbolic level def-use chain to highlight the data dependency between instructions.

##Current Progress
YADD is now `relying on the Dex file parsing and the instruction decoding algorithm of Android Open Source Project`.  It can now be built as a `pure executable for Dex code dumping`. More capabilities about control and data flow analysis will be updated in the near future.  

##Installation
Clone the project into your working folder.
In the working folder, type these instructions.
```
$ ./clean.py --rebuild
$ cd build
$ cmake ..
$ make
```
Done! And the executable should locate in:
`/path/to/your/working/folder/bin/dumper`

##Usage 
```
Usage: dumper [options]
    Example: dumper --granularity=instruction --input=/PATH/TO/MY/DEX --output=PATH/TO/MY/TXT

  --granularity=(class|method|instruction): For data granularity
    class      : List class names only
    method     : List method signatures only
    instruction: Full dump

  --input=<classes.dex>: Specify the input dex pathname

  --output=<dump.txt>: Specify the output dump pathname

```

##Contact
Any problems? please contact me via the mail: andy.zsshen@gmail.com  
