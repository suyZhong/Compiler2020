;ModuleID = 'assign.c'
source_filename = "assign.c"
;全局main函数定义
define dso_local i32 @main() #0{
;注释：第一个基本块开始
 %a = alloca [10 x i32], align 4
 %addr0 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 0 
 store i32 10, i32* %addr0, align 4
 %addr1 = getelementptr [10 x i32], [10 x i32]* %a, i32 0, i32 1
 %a0 = load i32, i32* %addr0
 %temp = mul i32 %a0, 2 ;用临时变量存放做乘法的结果，在下一步赋值给a1
 store i32 %temp, i32* %addr1, align 4
 
;注释：第一个基本块结束
;注释：第二个基本块开始
 %a1 = load i32, i32* %addr1
 ret i32 %a1
;注释：第二个基本块结束
}