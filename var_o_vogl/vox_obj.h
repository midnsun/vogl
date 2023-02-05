#pragma once
#include <cstdint>
#include <cstdlib>
//#include <crtdbg.h>
#include "var.h"

namespace some_const_variables{ const uint32_t b = sizeof(uint32_t) * 3 + sizeof(uint32_t) + sizeof(float) * 3 + sizeof(bool) + sizeof(uint32_t); }

template <typename T>
char* toBytes(T* var, size_t bytes);
template <typename T>
T fromBytes(char* s, size_t bytes);

typedef struct web_voxel_object {
	char vox_queue[some_const_variables::b];
	static const uint32_t my_unique_id = 2;
	uint32_t recieved_id;
} Wobj;

typedef struct voxel_object {
	static const uint32_t max_obj_amount = 512; //65536
	static const uint32_t obj_volume = 16;
	static const uint32_t bytes_size = some_const_variables::b;
	static uint32_t decode_id(char* s);
	bool visible = 0;
	uint32_t color;
	float x = 0, y = 0, z = 0;
	uint32_t w, h, d;
	var* v;
	void fill_vox_data();
	void bind_vox_data(var* vox);
	char* vox_to_bytes();
	bool bytes_to_vox(char *s, Wobj* web);
} Vobj;