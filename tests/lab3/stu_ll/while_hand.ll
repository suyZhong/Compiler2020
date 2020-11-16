
;ModuleID = 'while.c'
source_filename = "while.c"

define dso_local i32 @main() #0{
;注释：第一个基本块的开始
%addra = alloca i32, align 4
%addri = alloca i32, align 4
store i32 10, i32* %addra,align 4
store i32 0,  i32* %addri,align 4
br label %while
;注释；第一个基本块的结束


;
while:
    %i = load i32, i32* %addri
    %compare = icmp slt i32 %i, 10
    br i1 %compare, label %true, label %false
;

;注释：第二个基本块的开始
true:
    %a = load i32, i32* %addra, align 4
    %i1 = add i32 %i, 1
    %a1 = add i32 %a, %i1
    store i32 %i1, i32* %addri
    store i32 %a1, i32* %addra
    br label %while
;注释：第二个基本块的结束

;注释：第三个基本块的开始
false:
    %a1 = load i32, i32* %addra
    ret i32 %a1
;注释：第三个基本块的结束

}