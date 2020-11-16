;ModuleID = 'fun.c'
source_filename = "fun.c"

;callee函数定义
define dso_local i32 @callee(i32 %0) #0{
;注释：第一个基本块开始
 
 %temp = mul i32 %0, 2
 ret i32 %temp
;注释：第一个基本块结束
}

;全局main函数定义
define dso_local i32 @main() #0{
;注释：第二个基本块开始
  %result = call i32 @callee(i32 110)
  ret i32 %result
;注释：第二个基本块结束
}