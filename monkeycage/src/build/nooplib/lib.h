#pragma once

#include <stddef.h>
#include <stdlib.h>

extern "C" void monkeycage_init();

extern "C" void* monkeycage_register_cb(void* app_cb, void* key, size_t* cb_index);

extern "C" void* monkeycage_unregister_cb(void* key);

extern "C" void* monkeycage_retrieve_cb(void* sbx_cb, size_t* cb_index);

extern "C" size_t monkeycage_invoked_cb();
