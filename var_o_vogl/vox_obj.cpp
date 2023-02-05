#include "vox_obj.h"

template <typename T>
char* toBytes(T* var) {
    static char* buf = (char*)malloc(sizeof(var));
    buf = reinterpret_cast<char*>(var);
    return &buf[0];
}

template <typename T>
T fromBytes(char* s) { return *reinterpret_cast<T*>(s); }


void Vobj::fill_vox_data() {
	if (this->w == 0 || this->h == 0 || this->d == 0) this->w = Vobj::obj_volume; this->h = Vobj::obj_volume; this->d = Vobj::obj_volume;
}

void Vobj::bind_vox_data(var* vox) {
    this->v = vox;
}

char* Vobj::vox_to_bytes() { // visible color x y z w h d id
    //_CrtSetDebugFillThreshold(sizeof(uint64_t));
    char s[Vobj::bytes_size];
    size_t i, k = 0;
    uint32_t id = Wobj::my_unique_id;
    char* temp;

    temp = toBytes(&this->visible);
    s[0] = temp[0];
    k++;
    temp = toBytes(&this->color);
    for (i = 0; i < sizeof(this->color); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->x);
    for (i = 0; i < sizeof(this->x); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->y);
    for (i = 0; i < sizeof(this->y); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->z);
    for (i = 0; i < sizeof(this->z); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->w);
    for (i = 0; i < sizeof(this->w); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->h);
    for (i = 0; i < sizeof(this->h); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&this->d);
    for (i = 0; i < sizeof(this->d); i++) s[i + k] = temp[i];
    k += i;
    temp = toBytes(&id);
    for (i = 0; i < sizeof(id); i++) s[i + k] = temp[i];

    return &s[0];
}

bool Vobj::bytes_to_vox(char* s, Wobj* web) { //id d h w z y x color visible
    size_t k = 0;
    k += sizeof(web->recieved_id);
    web->recieved_id = fromBytes<uint32_t>(&s[Vobj::bytes_size - k]);
    if (/*web->recieved_id != Wobj::my_unique_id*/1) {
        k += sizeof(this->d);
        this->d = fromBytes<uint32_t>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->h);
        this->h = fromBytes<uint32_t>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->w);
        this->w = fromBytes<uint32_t>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->z);
        this->z = fromBytes<float>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->y);
        this->y = fromBytes<float>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->x);
        this->x = fromBytes<float>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->color);
        this->color = fromBytes<uint32_t>(&s[Vobj::bytes_size - k]);
        k += sizeof(this->visible);
        this->visible = fromBytes<bool>(&s[Vobj::bytes_size - k]);
        return true;
    }
    else return false;
}

uint32_t Vobj::decode_id(char* s) { return fromBytes<uint32_t>(&s[Vobj::bytes_size - sizeof(Wobj::my_unique_id)]); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////