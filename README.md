# Pointer Class Language++
> A simple Work in Progress Just-in-Time Compiler designed for ARM Processors (Specifically tested on a Nintendo 3DS using devkitPro), written entirely in C++.
> It compiles directly to opcodes, without linking or anything external.

# Usage
---
### Compiling PCL++ Code
To compile some code, simply create an instance of the PCLPP class.
`PCLPP pclpp;` <br>
And call the compile function.
`pclpp.compile("my code as std::string");`

---
### Running PCL++ Code <br>
The assemblinizer.hpp and assemblinizer_jit.h file work together to run the code. <br>
This repo for example uses Kynex7510's [CTRL](https://github.com/kynex7510/CTRL) library. <br>
<br>
Heres an example on how to run it using the CTRL library:
`int out = Assemblinizer::Run(plcpp.GetMainAssembly());`

# Syntax
---
### Simple Example
Heres a simple example of the PCL++ language.
```cpp
class int4 byte 4;

class PBC {
  int4 test = 12;
}

main {
  new PBC pbc;
  returnset pbc.test;
}
```
To return the address of a variable instead, put an Asterix (*) after the full usage.<br>
`returnset pbc.test*;` <br>
> Sets r0 to the address of pbc.test

# How the Language works
---
Basically, everything in this Language is a pointer. Its specifically designed for ARM32 processors.
A Script has a limit of 65536 local variables. Once a block finishes, it frees all the slots the block used on the local variables, and the actual memory at that address.
A local variable ALWAYS stores an Address to memory. All this wouldnt be possible without the standard library it uses. It uses the standard library to:
- Read Memory
- Change Memory
- Manage Local Variables
- Allocate
- Free

Im still working on adding custom functions, which will work like this:
```cpp
int MyCPPFunction()
{
  return 45;
}
PCLPP pclpp;
PCLPP_Library& pclpplib;
pclpp.Link("MyCPPFunction", (uint32_t)MyCPPFunction);
pclpp.AddLink(pclpplib);
```
```cpp
main {
  call returnedVal MyCPPFunction();
  returnset returnedVal;
}
```
> The above code will return 45 (once its actually implemented)
Class functions also are a big goal, but will be very difficult.
