
;ModuleID = 'if.c'
source_filename = "if.c"

;注释：全局函数main的定义
define dso_local i32 @main() #0 {
;注释：第一个基本块的开始
 %a = alloca float, align 4

 ;浮点数如何表示？
 store float 0x40163851E0000000, float* %a, align 4
 %avalue = load float, float* %a
 %compare = fcmp olt float 1.0, %avalue
 br i1 %compare, label %true, label %false
;注释：第一个基本块的结束

;注释：第二个基本块的开始
true:
    ret i32 233
;注释：第二个基本块的结束

;注释：第三个基本块的开始
false:
    ret i32 0
;注释：第三个基本块的结束
}