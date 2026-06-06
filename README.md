# unibug

`unibug` is a tiny PE test case for a Unicorn x86 limitation.

## Purpose

This demonstrates the limitations of Unicorn and how it can fail on specific AVX instructions, this being one I've accidentally discovered, on such simple things such as copying data from one memory location to another.

## Explaination

The program itself is normal and prints `hello world`, and if AVX is available on the host it then builds a 32 byte packet containing `helloworld`, and
passes that packet through `foobar`.

The important part is that `foobar` does the pass with raw AVX bytes:

```asm
C5 FC 10 02 C5 FC 11 01
```

Or as raw ASM instructions would look like

```asm
vmovups ymm0, ymmword ptr [rdx]
vmovups ymmword ptr [rcx], ymm0
```

On Windows x64, `rdx` is the source pointer and `rcx` is the destination pointer.
So the function is just moving one 32 byte block from source to destination.

Native output on an AVX-capable machine (no emulation)

```text
hello world
passed data: helloworld
```

However, on unicorn (tested on unicorn v2.1.4) 

Starting emulation at `foobar` fails before any data is copied:

```text
Bytes=c5 fc 10 02 c5 fc 11 01 c3
EndResult=failed
Err=Invalid instruction (UC_ERR_INSN_INVALID)
RIPDelta=0x0
DstBytes=00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ...
```

The instructions are valid AVX
instructions. The problem is that this is the 256 bit YMM form and 
Unicorn 2.1.4 does not emulate or traditionally handle that path.

The `C5 FC` VEX prefix has the `L` bit set, which selects the 256 bit AVX
encoding. In Unicorn's x86 translator, that case is rejected on purpose:

```c
/* VEX.L (256 bit) encodings are not supported */
if (s->vex_l != 0) {
    goto illegal_op;
}
```

That then becomes `UC_ERR_INSN_INVALID`

## How this could be fixed:
- (Obviously) handle those instructions natively
- Add a code hook for mov