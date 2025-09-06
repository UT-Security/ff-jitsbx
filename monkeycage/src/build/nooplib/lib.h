#pragma once

#include <stddef.h>

extern "C" void monkeycage_init();

extern "C" _Thread_local size_t monkeycage_last_callback_invoked;

extern "C" void* monkeycage_register_cb(void* app_cb, size_t* cb_index);

extern "C" void* monkeycage_unregister_cb(void* cb);

extern "C" void* monkeycage_retrieve_cb(void* sbx_cb, size_t* cb_index);
