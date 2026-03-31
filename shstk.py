import gdb
from pwn import asm, context

class ShstkRet(gdb.Command):
    def __init__(self):
        super(ShstkRet, self).__init__("shstk_ret", gdb.COMMAND_RUNNING)
        self.dont_repeat()

    def read_shstk_ptr(self):
        # Save rip
        f = gdb.newest_frame()
        saved_ip = int(f.read_register("rip"))
        # Read shstk into r11
        context.arch = "amd64"
        sc = asm(f"rdsspq r11")
        # Save memory at rip
        inferior = gdb.selected_inferior()
        saved_mem = inferior.read_memory(saved_ip, len(sc))
        # Patch in and execute shellcode
        inferior.write_memory(saved_ip, sc)
        gdb.execute(f"si")
        # Restore saved memory
        inferior.write_memory(saved_ip, saved_mem)
        gdb.execute(f"set $rip = {saved_ip}")
        
    def do_ret(self):
        # Read rsp
        f = gdb.newest_frame()
        sp = int(f.read_register("rsp"))
        # Load into rip
        inferior = gdb.selected_inferior()
        mem = inferior.read_memory(sp, 8)
        new_rip = int.from_bytes(bytes(mem), byteorder="little", signed=False)
        gdb.execute(f"set $rip = {new_rip}")
        # Update rsp
        new_rsp = sp + 8
        gdb.execute(f"set $rsp = {new_rsp}")

    def invoke(self, arg, from_tty):
        self.do_ret()
        self.read_shstk_ptr()
        
class ReadShstk(gdb.Command):
    def __init__(self):
        super(ReadShstk, self).__init__("read_shstk", gdb.COMMAND_RUNNING)
        self.dont_repeat()

    def read_shstk_ptr(self):
        # Save rip
        f = gdb.newest_frame()
        saved_ip = int(f.read_register("rip"))
        # Read shstk into r11
        context.arch = "amd64"
        sc = asm(f"rdsspq r11")
        # Save memory at rip
        inferior = gdb.selected_inferior()
        saved_mem = inferior.read_memory(saved_ip, len(sc))
        # Patch in and execute shellcode
        inferior.write_memory(saved_ip, sc)
        gdb.execute(f"si")
        # Restore saved memory
        inferior.write_memory(saved_ip, saved_mem)
        gdb.execute(f"set $rip = {saved_ip}")

    def invoke(self, arg, from_tty):
        self.read_shstk_ptr()
        
class IncShstk(gdb.Command):
    def __init__(self):
        super(IncShstk, self).__init__("inc_shstk", gdb.COMMAND_RUNNING)
        self.dont_repeat()

    def inc_shstk(self):
        # Save rip
        f = gdb.newest_frame()
        saved_ip = int(f.read_register("rip"))
        # Read shstk into r11
        context.arch = "amd64"
        sc = asm(f"push r11; mov r11, 1; incsspq r11; pop r11;")
        # Save memory at rip
        inferior = gdb.selected_inferior()
        saved_mem = inferior.read_memory(saved_ip, len(sc))
        # Patch in and execute shellcode
        inferior.write_memory(saved_ip, sc)
        gdb.execute(f"si 4")
        # Restore saved memory
        inferior.write_memory(saved_ip, saved_mem)
        gdb.execute(f"set $rip = {saved_ip}")

    def invoke(self, arg, from_tty):
        self.inc_shstk()

# Register to the gdb runtime
ShstkRet()
ReadShstk()
IncShstk()
