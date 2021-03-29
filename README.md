# 实验说明
请fork此`repo`到自己的仓库下，随后在自己的仓库中完成实验，请确保自己的`repo`为`Private`。

### 目前已布置的实验:
*   [lab1](./Documentations/lab1/README.md)
    *   DDL : 2020/10/13, 23:59:59
*   [lab2](./Documentations/lab2/README.md)
    *   DDL : 2020/10/30, 23:59:59
*   [lab3](./Documentations/lab3/README.md)
    *   DDL : 2020/11/16, 23:59:59
*   [lab4](./Documentations/lab4/README.md)
    *   DDL : 2020/12/07, 23:59:59
*   [lab5](./Documentations/lab5/README.md)
    *   DDL-Phase1 :2020/12/21 23:59:59 (北京标准时间，UTC+8)  
    *   DDL-Phase2 :2021/01/07 23:59:59 (北京标准时间，UTC+8) 
### FAQ: How to merge upstream remote branches
In brief, you need another alias for upstream repository (we assume you are now in your local copy of forked repository on Gitlab):
```
(shell) $ git remote add upstream http://222.195.68.197/staff/2020fall-compiler_cminus.git
```
Then try to merge remote commits to your local repository:
```
(shell) $ git pull upstream master
```
Then synchronize changes to your forked remote repository:
```
(shell) $ git push origin master
```

----

## 说明

The work I've done are in `src/`
- `cminusfc/cminusf_builder_stu.cpp`（auto gen IR）
- `optimization/ActiveVars.cpp`（对IR活跃变量的分析）
- `optimization/ConstPropagation.cpp`：常量传播的实现
- `optimization/GlobalSubExprDeletion.cpp`：块间公共子表达式删除不完全的实现
- `optimization/LoopInvHoist.cpp`：循环不变式外提的实现
- `optimization/MarkedCodeDeletion.cpp`：标记式死代码删除的实现

感谢助教提供了自己设计的，简化版llvm接口：lightIR。以及提供的各种示例程序：
- `src/cminusfc/cminusf_builder.cpp`
- `src/optimization/Mem2Reg.cpp`
- `src/optimization/LoopSearch.cpp`
- ...

感谢助教团队
- Zewen Jin
- Ping Gong
- Jiahao Li
- Yiming Zhu
- Qingyuan Chen
- Haiquan Wang
- Kai Ma
- Yuxin Ma

## License

Because the repo contains code from TAs, who have not licensed the code under an open source license, but only allowed us to public the code, the repo is only public for learning.

ALL RIGHTS ARE RESERVED.