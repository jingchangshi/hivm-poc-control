module attributes {hacc.target = #hacc.target<"Ascend950PR_9579">} {
  func.func @plain_cache4(%src: memref<64xf32, #hivm.address_space<ub>>,
                          %dst: memref<64xf32, #hivm.address_space<gm>>) {
    hivm.hir.store ins(%src : memref<64xf32, #hivm.address_space<ub>>)
                   outs(%dst : memref<64xf32, #hivm.address_space<gm>>)
                   {l2_cache_mode = 4 : i32}
    return
  }
}
