//
//  siphash.cpp
//  cuckoo_xcode
//
//  Created by pebble8888 on 2019/03/23.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "siphash.hpp"

// set siphash keys from 32 byte char array
void Siphash_keys::setkeys(const char *keybuf) {
    k0 = htole64(((uint64_t *)keybuf)[0]);
    k1 = htole64(((uint64_t *)keybuf)[1]);
    k2 = htole64(((uint64_t *)keybuf)[2]);
    k3 = htole64(((uint64_t *)keybuf)[3]);
}

uint64_t Siphash_keys::siphash24(const uint64_t nonce) const {
    Siphash_state v(*this);
    v.hash24(nonce);
    return v.xor_lanes();
}

