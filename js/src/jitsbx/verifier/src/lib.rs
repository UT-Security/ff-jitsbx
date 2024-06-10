mod bindings;

pub use bindings::*;

use std::{slice, collections::HashSet};
use yaxpeax_x86::amd64::{InstDecoder, Instruction, Opcode, Operand, RegSpec};
use yaxpeax_arch::{LengthedInstruction, AddressDiff, AddressBase};

const BUNDLE_ALIGNMENT: usize = 32;
const MAX_CODE_BYTES_PER_PROCESS: usize = 512 * 1024 * 1024;
const INDIRECT_CODE_TARGET_MASM: i32 = ((MAX_CODE_BYTES_PER_PROCESS - 1) & !(BUNDLE_ALIGNMENT - 1)) as i32;  

pub(crate) struct JitCodeInfo<'a> {
    code: &'a[u8],
    code_addr: u64,
    double_pool_offset: u32,
    float_pool_offset: u32,
    simd_pool_offset: u32,
    extended_jump_table_offset: u32,
}

impl<'a> From<*const bindings::JitCodeInfo> for JitCodeInfo<'a> {
    fn from(info: *const bindings::JitCodeInfo) -> JitCodeInfo<'a> {
        unsafe {
            JitCodeInfo {
                code: slice::from_raw_parts((*info).code, (*info).length),
                code_addr: (*info).code as u64,
                double_pool_offset: (*info).double_pool_offset,
                float_pool_offset: (*info).float_pool_offset,
                simd_pool_offset: (*info).simd_pool_offset,
                extended_jump_table_offset: (*info).extended_jump_table_offset,
            }
        }
    }
}

pub(crate) struct Verifier<'a> {
    info: JitCodeInfo<'a>,
    decoder: InstDecoder,
    icount: usize,
    jump_targets: HashSet<usize>,
    start_addr: Vec<usize>,
    instrs: Vec<Instruction>,
}

impl<'a> Verifier<'a> {
    pub(crate) fn new(info: JitCodeInfo<'a>) -> Self {
        Verifier {
            info,
            decoder: InstDecoder::default(),
            icount: 0,
            jump_targets: HashSet::new(),
            start_addr: Vec::new(),
            instrs: Vec::new(),
        }
    }

    pub(crate) fn verify(&mut self) -> bool {
        let mut ip: usize = 0;

        while ip < self.info.double_pool_offset as usize {
            let instr = match self.decoder.decode_slice(&self.info.code[ip..]) {
                Ok(instr) => instr,
                Err(err) => {
                    return false;
                }
            };

            self.instrs.push(instr);

            if !self.instr_is_allowed(instr) {
                return false;
            }

            self.start_addr.push(ip);
            self.icount += 1;

            if self.instr_overlaps_block_size(instr, ip) {
                return false;
            }

            if self.instr_is_indirect_jump_or_call(instr) {
                if !self.instr_is_jitsbx_jump_or_call(instr, ip) {
                    return false;
                }
            } else {
                self.jump_targets.insert(ip);
            }

            ip += instr.len().to_const() as usize;
        }

        true       
    }

    fn instr_is_allowed(&self, instr: Instruction) -> bool {
        match instr.opcode() {
            Opcode::RETURN => false,
            Opcode::CALLF | Opcode::JMPF => false,
            Opcode::CALL | Opcode::JMP => {
                match instr.operand(0) {
                    Operand::RegIndexBaseScaleDisp(_, _, _, _)
                    | Operand::RegIndexBaseScale(_, _, _)
                    | Operand::RegScaleDisp(_, _, _) 
                    | Operand::RegIndexBaseDisp(_, _, _) 
                    | Operand::RegIndexBase(_, _) 
                    | Operand::RegScale(_, _)
                    | Operand::RegDisp(_, _) => false,
                    _ => true,
                }
            },
             _ => true
        }
    }

    fn instr_overlaps_block_size(&self, instr: Instruction, ip: usize) -> bool {
        let length = instr.len().to_const() as usize;
        if ip / BUNDLE_ALIGNMENT != (ip + length - 1) / BUNDLE_ALIGNMENT {
            true
        } else {
            false
        }
    }

    fn instr_is_indirect_jump_or_call(&self, instr: Instruction) -> bool {
        match instr.opcode() {
            Opcode::CALL | Opcode::JMP => {
                match instr.operand(0) {
                     Operand::Register(_)
                     | Operand::RegDeref(_) => true,
                    _ => false,
                }
            }
            _ => false,
        }
    }

    fn instr_is_jitsbx_jump_or_call(&self, instr: Instruction, ip: usize) -> bool {
        let mut verifier = IndirectJumpVerifier::new(&self.info, ip, instr);       
        
        verifier.verify(&self.decoder)
    }
} 

struct IndirectJumpVerifier<'a> {
    info: &'a JitCodeInfo<'a>,
    ip: usize,
    instr: Instruction,
    is_call: bool,
    register: RegSpec,
    state: IndirectJumpVerifierState,
}

enum IndirectJumpVerifierState {
    Initial,
    Masked(usize),
    Sandboxed(usize),
}

impl<'a> IndirectJumpVerifier<'a> {
    pub fn new(info: &'a JitCodeInfo<'a>, ip: usize, instr: Instruction) -> Self {    
        let is_call = match instr.opcode() {
            Opcode::CALL => true,
            Opcode::JMP => false,
            _ => unreachable!(),
        };

        let register = match instr.operand(0) {
            Operand::Register(register) => register,
            Operand::RegDeref(register) => register,
            _ => unreachable!(),
        };

        Self {
            info,
            ip,
            instr,
            is_call,
            register,
            state: IndirectJumpVerifierState::Initial,
        }
    }

    pub fn verify(&mut self, decoder: &InstDecoder) -> bool {
        // return address of call instruction should be bundle aligned.
        if self.is_call && !((self.ip + self.instr.len().to_const() as usize) % BUNDLE_ALIGNMENT == 0) {
            return false;
        }

        let current = (self.ip / BUNDLE_ALIGNMENT) * BUNDLE_ALIGNMENT;

        while current < self.ip {
            let instr = match decoder.decode_slice(&self.info.code[current..]) {
                Ok(instr) => instr,
                _ => unreachable!(),
            };

            match self.state {
                IndirectJumpVerifierState::Initial => {
                    current += instr.len().to_const() as usize;
                    if instr.opcode() == Opcode::AND && self.is_jitsbx_mask(instr, current, decoder) {
                        
                    }
                },
                IndirectJumpVerifierState::Masked(mask_ip) => {
                    
                },
                IndirectJumpVerifierState::Sandboxed(mask_ip) => {
                    
                }
            }
        }

        true
    }

    fn is_jitsbx_mask(&self, instr: Instruction, ip: usize, decoder: &InstDecoder) -> bool {
        /*match instr.operand(0) {
            Operand::Register(RegSpec::d(self.register.num())) => (),
            _ => return false,
        }*/

        /*match instr.operand(1) {
            Operand::ImmediateI32(gn)
        }*/

        true
    }
}
