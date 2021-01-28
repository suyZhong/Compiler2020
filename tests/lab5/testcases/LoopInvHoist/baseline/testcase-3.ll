; ModuleID = 'cminus'
source_filename = "testcase-3.cminus"

@a = global i32 zeroinitializer
@b = global i32 zeroinitializer
@c = global i32 zeroinitializer
declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  store i32 1, i32* @a
  store i32 2, i32* @b
  store i32 3, i32* @c
  %op8 = load i32, i32* @a
  %op9 = load i32, i32* @b
  %op10 = mul i32 %op8, %op9
  %op11 = load i32, i32* @c
  %op12 = mul i32 %op10, %op11
  %op13 = mul i32 %op12, 123
  %op14 = sitofp i32 %op13 to float
  %op15 = fmul float %op14, 0x3ff3c0c200000000
  %op16 = fmul float %op15, 0x4016f06a20000000
  %op17 = fmul float %op16, 0x4002aa9940000000
  %op18 = fmul float %op17, 0x4011781d80000000
  %op19 = fmul float %op18, 0x401962ac40000000
  %op20 = fptosi float %op19 to i32
  br label %label2
label2:                                                ; preds = %label_entry, %label7
  %op25 = phi i32 [ %op22, %label7 ], [ undef, %label_entry ]
  %op26 = phi i32 [ %op20, %label7 ], [ undef, %label_entry ]
  %op4 = icmp slt i32 %op25, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label23
label7:                                                ; preds = %label2
  %op22 = add i32 %op25, 1
  br label %label2
label23:                                                ; preds = %label2
  call void @output(i32 %op26)
  ret void
}
