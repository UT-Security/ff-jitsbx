#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>
#include <sys/mman.h>

_Thread_local size_t monkeycage_last_callback_invoked;

static bool cbinit();

extern void monkeycage_cbtrampoline();

void monkeycage_init() {
  bool ok = cbinit();
  if (!ok) {
    fprintf(stderr, "monkeycage: failed to initialize callback entries\n");
    exit(1);
  }
}

#define MAXCALLBACKS 40960

static void* callbacks[MAXCALLBACKS];

static size_t cbfreeslot() {
  for (size_t i = 0; i < MAXCALLBACKS; i++) {
    if (!callbacks[i]) {
      return i;
    }
  }

  return MAXCALLBACKS;
}

static size_t cbfind(void* fn) {
  for(size_t i = 0; i < MAXCALLBACKS; i++) {
    if (callbacks[i] == fn)
      return i;
  }

  return MAXCALLBACKS;
}

struct CallbackEntry {
  uint8_t code[40];
  uint64_t target;
  size_t index;
  uint64_t trampoline;
};

// Code for a callback trampoline.
static uint8_t cbtrampoline[40] = {
  0x4c, 0x8b, 0x15, 0x21, 0x00, 0x00, 0x00, // mov    33(%rip), %r10
  0x48, 0x8b, 0x05, 0x22, 0x00, 0x00, 0x00, // mov    34(%rip), %rax
  0xff, 0x25, 0x24, 0x00, 0x00, 0x00,       // jmp    *36(%rip)
  0x66, 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, // nops
  0x00, 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, // nops
};

static struct CallbackEntry* cbentries;

static bool cbinit() {
  size_t size = MAXCALLBACKS * sizeof(struct CallbackEntry);
  void* cbmap = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (cbmap == (void*)-1) {
    return false;
  }

  cbentries = (struct CallbackEntry*) cbmap;

  for (size_t i = 0; i < MAXCALLBACKS; i++) {
    memcpy(&cbentries[i].code, &cbtrampoline[0], sizeof(cbentries[i].code));
    cbentries[i].index = i;
  }

  return true;
}

void* monkeycage_register_cb(void* fn, size_t* index) {
  assert(fn);
  assert(index);
  assert(cbfind(fn) == MAXCALLBACKS && "fn is already registered as a callback");

  size_t slot = cbfreeslot();
  if (slot == MAXCALLBACKS) {
    *index = MAXCALLBACKS;
    return NULL;
  }

  // write 'fn' into the 'target' field for the chosen slot.
  __atomic_store_n(&cbentries[slot].target, (uint64_t) fn, __ATOMIC_SEQ_CST);
  
  // write the trampoline into the 'trampoline' field for the chosen slot
  __atomic_store_n(&cbentries[slot].trampoline, (uint64_t) monkeycage_cbtrampoline, __ATOMIC_SEQ_CST);

  // Mark the slot as allocated.
  callbacks[slot] = fn;
  *index = slot;

  return &cbentries[slot].code[0];
}

void monkeycage_unregister_cb(void* cb) {
  size_t slot = cbfind(cb);
  if (slot == MAXCALLBACKS)
    return;

  callbacks[slot] = NULL;
  __atomic_store_n(&cbentries[slot].target, 0, __ATOMIC_SEQ_CST);
  __atomic_store_n(&cbentries[slot].trampoline, 0, __ATOMIC_SEQ_CST);
}

void* monkeycage_retrieve_cb(void* cb, size_t* index) {
  assert(cb);
  assert(index);

  size_t size = MAXCALLBACKS * sizeof(struct CallbackEntry);
  if (cb < (void*)cbentries || cb > (void*)((uint8_t*)cbentries + size)) {
    *index = MAXCALLBACKS;
    return NULL;
  }

  size_t slot = ((uint8_t*)cb - (uint8_t*)cbentries) / sizeof(struct CallbackEntry);
  assert(slot < MAXCALLBACKS);
  assert(&cbentries[slot].code[0] == cb);

  *index = slot;
  return (void*)cbentries[slot].target;
}
