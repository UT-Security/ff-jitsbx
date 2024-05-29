use libc::{size_t, uint8_t};

#[no_mangle]
pub extern "C" fn jitsbx_verify(code: *mut uint8_t, length: size_t) -> bool {
    true
}
