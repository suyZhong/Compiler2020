[TOC]

# lab1实验报告

学号-PB18111733

姓名-蒲明昱

## 实验要求

完成了根据`cminux-f`的词法补全[lexical_analyer.l](./src/lexer/lexical_analyzer.l)文件，完成词法分析器，能够输出识别出的`token`，`type` ,`line(刚出现的行数)`，`pos_start(该行开始位置)`，`pos_end(结束的位置,不包含)`的实验要求。

## 实验难点

1. 虚拟机环境使用不熟练：按照issue中worth-reading，配置vscode完成此次实验。（虽然优秀的同学已经在issue里写的很清楚啦，还是小小的花了比较长的时间）
2. `cminux-f`词法不熟悉：造成了在一开始做实验时，可以明白实验要求和大致要完成的方向，但是不知道按照什么样的语法来写。当查找很多网上的范例，调试通过对“int a；”的词法分析后，感觉思路才明朗了起来。

3. 再然后可能就是不停地调试注释相关的代码hhh，注释里的换行符能不能被识别，空格能不能被识别，等等。最后写了一个comment函数来处理注释这个东西。

## 实验设计

根据lexical_analyzer.h中给出的需要识别的字符，依次在lexical_analyzer.l中进行处理。

分别考虑不同情况下的换行，pos_start 和 pos_end

根据diff比较给出的结果，对不正确的地方进行重新考虑，debug之后可以通过助教的全部testcase

## 实验结果验证

##### 1.助教的测试样例：

进行cmake和make之后，采用diff进行测试：
![avatar](/home/cp/lab1.1/Reports/lab1/img/testcase.png)


##### 2.自己的测试样例：

##### test.cminus:

```
/*12√注释*/
/*🙂注释*/
/*注释*/
/*"*"注释*/
/*d=====(￣▽￣*)b*/
/*↕
*/
/*◍*/
/*(⊙o⊙)？*/
/*おㄞげㄚ*/
/*◒◑◓*/

a[1];
a[];

i=0;
while(/*试试能不能写到扩号中间*/i<3){
    i+1;
}

/*又是对注释的尝试*/
void main()
{
    int[] outputs =
    {
        /*I'm*/       -1;
        /*afraid*/    +1;
        /*this*/      -1;
        /*can't*/     +1;
        /*be*/        -1;
        /*completed*/ +1;
    }
}

/ * * /
```

使用如下命令得到out文件，判断是否满足

`./build/lexer ./test.cminus out`

out文件的内容：
![avatar](/home/cp/lab1.1/Reports/lab1/img/out.png)

## 实验反馈

1. 实验紧扣课程内容，文档详细
2. 虽然实验做完之后可以发现难度并不高，却仍花费了好几天的时间。在初始配置环境，cminus-f语法的使用方面存在的问题较多

