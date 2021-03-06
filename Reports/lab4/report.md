# Lab4 实验报告

小组组长 蒲明昱 PB18111733 
小组成员 钟溯飏 PB18111764


## 实验要求

- 了解lightIR设计，阅读文档，了解相关接口的使用
- 阅读cminus-f语义规则与ast.cpp，了解语法树是如何生成的
- 根据相关的语义规则，实现cminus_builder，可以自动生成IR
- 完成类型检查，必要时需要实现转化，同时尽量减少冗余

## 实验难点

- 如何存取各个变量
- 细心考虑各个地方的类型转换问题
- if/else 中出现return时，对基本块的处理
- 如何正确处理函数调用，函数定义，取值等各个地方的数组（指针）

## 实验设计

1. 如何设计全局变量
   - 本次实验对变量的存取基本没有使用全局变量，而是在每次访问时，创建一个新的scope，结束后用exit()，使用"@"存放可能的返回值，使用"&"存放var的返回地址。（因为变量名不可能有这种字符出现）
   - 定义并赋值了tmpValue和tmpFloat，但实际上并没有使用。
   - 使用tmpInt存放可能要用的int值，这个仅在varDeclaration创建数组中使用，在对num结点遍历后，使用Arraytype创建数组类型时采用更新的tmpInt。（因为varDec时仅可能为integer类型）
   - 使用assignFlag对assign-stmt做标记，以减少无用的冗余，具体见下。
   - 使用retFlag标记某个stmt里有return-stmt，防止selection-stmt产生多余的bb和跳转。具体细节为，在每次if、else、while执行完stmt访问后，进行判断，若retFlag为假，才进行跳转bb及指令的设置。
   - 使用brDepth对if/else/while中出现的分支进行维护，去除可能存在的冗余BB，具体见下。
2. 遇到的难点以及解决方案
   - 在if/else中出现了return则容易导致各种报错，通过使用retFlag和brDepth解决。具体见下。
   - 函数调用时，数组参数通过指针来传入函数，需要注意使用gep来获取数组首元素地址；如果该参数是个指针，即call所需的参数是该函数接受参数中的参数，是需要直接传入。
   - 访问var时，对数组的取值遇到困难。解决：对于某个变量，类型为数组和指针的取值方法是不一样的。如果是数组，即通过局部变量定义或全局变量定义的，在使用gep时idx需要两个偏移量，一个是数组元素自身的偏移量，一个是在这个元素中的偏移量；而如果这个var是个指针，即通过函数传入的形参(int \* 或 float \*)去取值，则只需要一个偏移量。
   - 在assign，return，call，expr中出现传入类型与所需要类型不一致时，进行类型转换，具体是首先判断是否是i1，如果是则直接转化为i32（因为cminusf不接受i1运算，也没有发现i1直接转换成float的方法）；然后在assign, return, call中，若所需类型与传入类型不一致，则强制将传入类型转化为所需类型；在expr中，只要某一个操作数为float，则将另一个数转化为float。在下标运算中，则结果无论如何都强制转化为i32。注意在if和while的cond_br中，考虑到有可能出现expr值不为i1，则加一个判断，若不为i1则和0比较，比零大则为真，反之为假。
3. 如何降低生成 IR 中的冗余
   - 降低访问var时的冗余：在本程序设计中，访问var时需要对数据进行load并返回，但在assign表达式里，对左值的操作并不需要load，因此在访问这个结点时，使用assignFlag对其进行标记。在var中，若assignFlag为假，才进行create_load操作；同时考虑到在数组赋值时，具有特殊性，下标并不是一个左值，这里需要load，于是又采用一个标记assignIndexFlag对其进行缓存，在此时访问expr时，先将assignFlag置为false，访问完以后，再取回原来的assignFlag值。
   - 降低return后多余的return及代码的冗余：因为如果出现了if/else里都有return的情况，实际上接下来的所有stmt都是不需要的。这里维护一个brDepth进行判断，并在compound-stmt迭代访问stmt时，若brDepth为比迭代之前小，则直接跳出迭代过程，这样就降低了冗余与错误。具体解释如下
     - 每当访问一个selection-stmt或iteration-stmt时，brDepth++，每遇到一个return-stmt时brDepth--，若一个if-stmt没有返回值也没有else-stmt，brDepth--；若一个if-stmt没有完全return，且else-stmt没有return-stmt时，brDepth--。
     - 这么做是为了保证在一个正常if/else（即至多只有一个分支里有return）或while结束后，brDepth回到进入stmt之前的状态。
   - if/else 里，若缺少else，则会做出判断，只生成两个BB而非默认的三个。
4. 对scope的使用
   - 先前提到了本代码并未使用助教推荐的全局变量来存取变量，而是使用了scope的方法（本质上一样，应该只要全部替换掉就可以） 
   - 即，在需要获取返回值的访问结点操作前后加上scope.enter()和scope.exit()，并在访问结点时使用scope.push("@", value)将可能要用的值，如某个expr的值推入作用域，在离开结点回到原来结点时，使用scope.find("@")将其取出。
   - 其余的（如全局变量，函数等）则用scope.push(node.id, value)实现。


### 实验总结

- 学习了各种cpp语法，对访问者模式有了深刻的认识。
- 也在人脑遍历语法树的同时，对cminusf语法掌握得更加深入。
- 需要注意严格根据语义进行类型检查与转换。
- 需要学会仔细阅读开发者文档的能力，去很快的了解头文件以及demo是很有助于快速上手的
- 学会使用logging及gdb进行调试，能在segmentation fault时很快定位问题。
- 对于符合要求的各种情况都要有细致的考虑，同时在尽可能简单的方法中减少代码的冗余。
- <img src="./figs/success.png" alt="suc" style="zoom:50%;" />

### 实验反馈 （可选 不会评分）

- 感觉上手时有些困难，看着那么多文档以及头文件，容易找不准切入点
- 给的测试样例好像有点简单？

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
