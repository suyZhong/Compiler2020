# Lab5 实验报告-阶段一

小组组长 蒲明昱 PB18111733 
小组成员 钟溯飏 PB18111764

## 实验要求

请按照自己的理解，写明本次实验需要干什么

## 思考题
### LoopSearch
1. 循环的入口如何确定？循环的入口的数量可能超过1嘛？
   - 使用`find_loop_base()`确定，逻辑为：对之前Tarjan算法获取的强联通分量进行操作，迭代每个scc的每个node，去判断他们的prev是否在这个scc里，若不在，则返回这个node.
   - 不可能超过1:
2. 简述一下算法怎么解决循环嵌套的情况。
   - 每次使用Tarjan算法后，只会获得大的循环体。在对其进`find_loop_base()`后，将base node加入reserved中，然后删除base node，（同时删除nodes,base后继的prev和前驱的succ中所有的basenode），这样做即将大的scc解体。
   - 接着对所有nodes中的结点初始化，然后回到while大循环，从头开始操作，直到所有的`result.size() == 0`即处理完所有嵌套。
   - 对reserved解释：因为之前删除了base后继的prev，若嵌套while紧随大的while，即这个入口的处于外层的prev刚好是被删除的base node，会出现所有的`find(prev)`都成功，则此时`base == nullptr`；去reserved中寻找，即大while的后继是否在这个scc中，若在，则代表是这个嵌套while的入口，返回即可。
### Mem2reg
1. ...
2. ...
3. ...

### 代码阅读总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
