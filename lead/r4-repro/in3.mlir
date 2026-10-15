

module attributes {hacc.target = #hacc.target<"Ascend310B3">} {
  func.func @absent_ascend310b3(%src: memref<64xf32, #hivm.address_space<ub>>,
                                %dst: memref<64xf32, #hivm.address_space<gm>>) {
    hivm.hir.store ins(%src : memref<64xf32, #hivm.address_space<ub>>)
                   outs(%dst : memref<64xf32, #hivm.address_space<gm>>)
    return
  }
}
