// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_HAL_DMA_HH_
#define UTL_HAL_DMA_HH_

namespace utl::hal::dma {

template <typename Buffer_t, typename T>
volatile Buffer_t* dma_cast(volatile T& object) {
    return reinterpret_cast<volatile Buffer_t*>(const_cast<volatile std::remove_const_t<T>*>(&object));
}

template <typename T>
size_t dma_sizeof(T&& object) {
    return sizeof(object);
}

} //namespace utl::hal::dma

#endif //UTL_HAL_DMA_HH_
