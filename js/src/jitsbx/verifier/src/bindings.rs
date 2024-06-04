use libc::size_t;
use crate::*;

#[repr(C)]
pub struct JitCodeInfo {
    pub code: *const u8,
    pub length: size_t,

    pub double_pool_offset: u32,
    pub float_pool_offset: u32,
    pub simd_pool_offset: u32,

    pub extended_jump_table_offset: u32,
}

#[no_mangle]
pub extern "C" fn verify(info: *const JitCodeInfo) -> bool {
    let mut verifier = Verifier::new(info.into());
    verifier.verify()
}
