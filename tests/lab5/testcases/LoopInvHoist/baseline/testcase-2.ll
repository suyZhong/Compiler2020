; ModuleID = 'cminus'
source_filename = "testcase-2.cminus"

declare i32 @input()

declare void @output(i32)

declare void @outputFloat(float)

declare void @neg_idx_except()

define void @main() {
label_entry:
  %op10 = sitofp i32 0 to float
  %op11 = fmul float %op10, 0x4002666660000000
  %op12 = fmul float %op11, 0x3ff6666660000000
  %op13 = fptosi float %op12 to i32
  %op14 = sitofp i32 3 to float
  %op15 = fmul float %op14, 0x3ff0000000000000
  %op16 = fptosi float %op15 to i32
  %op19 = add i32 %op13, %op16
  %op20 = add i32 %op19, 123
  br label %label2
label2:                                                ; preds = %label_entry, %label7
  %op25 = phi i32 [ %op16, %label7 ], [ undef, %label_entry ]
  %op26 = phi i32 [ %op13, %label7 ], [ undef, %label_entry ]
  %op27 = phi i32 [ %op22, %label7 ], [ undef, %label_entry ]
  %op28 = phi i32 [ %op20, %label7 ], [ undef, %label_entry ]
  %op4 = icmp slt i32 %op27, 100000000
  %op5 = zext i1 %op4 to i32
  %op6 = icmp ne i32 %op5, 0
  br i1 %op6, label %label7, label %label23
label7:                                                ; preds = %label2
  %op22 = add i32 %op27, 1
  br label %label2
label23:                                                ; preds = %label2
  call void @output(i32 %op28)
  ret void
}
