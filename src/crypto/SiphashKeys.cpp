//
//  SiphashKeys.cpp
//
//  Created by pebble8888 on 2019/03/23.
//  Copyright © 2019 pebble8888. All rights reserved.
//

#include "SiphashKeys.hpp"
#include "SiphashState.hpp"

// 64bit * 4 (=32バイト)
void Siphash::setkeys(const char *keybuf)
{
    k0 = htole64(((uint64_t *)keybuf)[0]);
    k1 = htole64(((uint64_t *)keybuf)[1]);
    k2 = htole64(((uint64_t *)keybuf)[2]);
    k3 = htole64(((uint64_t *)keybuf)[3]);
}

// SipHash-2-4
uint64_t Siphash::siphash24(const uint64_t nonce) const
{
    SiphashState v(k0, k1, k2, k3);
    v.hash24(nonce);
    return v.xorLanes();
}

std::string Siphash::description(void) const
{
    char buf[256];
    sprintf(buf, "%llx %llx %llx %llx", k0, k1, k2, k3);
    return std::string(buf);
}
