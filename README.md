
	     Judy - C library creating and accessing dynamic arrays
	 ==============================================================


WHAT IS JUDY?

Judy is a C library that implements a dynamic array.  
Read original README for more information



WHY clone from https://sourceforge.net/projects/judy/?

To fix 64bit compilation under MinGW
Judy project is very old. In that time nobody can imagine thet UL on 64bit *nix sistims would be 8 bytes long, but on Windows 4 bytes long.
In original Judy source code there are a lot of L and UL postfix for constants. (In C default type for constants is signed Int)
It is not a problem to compille Judy under Cigwin (or MSYS2) but all programs would depends on cygwin1.dll (or msys-2.0.dll).
Some time i need native Win64 binarys to deploy as one EXE, without additional DLLs.
But MinGW understands L and UL as 32 integer. 
I replace all L and UL with system depend variable type intptr_t and uintptr_t. ON 32bit system it is 4bytes and 8bytes size.






Story in two words:

I'm new to C. I need dictionaries, but C have no native dictionaries.
I was searching for ready to use implementation.
Sudenly  I found JUDY.
What am I using if for?

1. Lineup several log files by time: microsecond timestamps, 20-30 Gb text logs with minimum memory overhead.
2. Dictionry for ID <=> timestamp for 6-9 million records
3. Not so offen, but it is unique to Trie - searching for neighbors. Prev dictionary record, next dictionary record. JUDY is natively sorted.