// Copyright (c) 2020, University of Washington All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// Redistributions of source code must retain the above copyright notice, this list
// of conditions and the following disclaimer.
// 
// Redistributions in binary form must reproduce the above copyright notice, this
// list of conditions and the following disclaimer in the documentation and/or
// other materials provided with the distribution.
// 
// Neither the name of the copyright holder nor the names of its contributors may
// be used to endorse or promote products derived from this software without
// specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Note: Users should only use this file for compiling with NEWLIB

// In GLIBC, this header file is present in the default include path however
// in NEWLIB, the same header file is present in <default include path>/machine/
// therefore, this dummy file is used to actually call the correct header file in
// NEWLIB whilst still maintaining the same interface as GLIBC. This is done to prevent
// changes to the CUDA-lite library code.

#ifndef _ENDIAN_H
#define _ENDIAN_H

#include <machine/endian.h>

#if defined(_DEFAULT_SOURCE) && (!defined(__ASSEMBLER__))
// BlackParrot is Little Endian
#define le16toh(_x) ((__uint16_t)(_x))
#define htole16(_x) ((__uint16_t)(_x))
#define le32toh(_x) ((__uint32_t)(_x))
#define htole32(_x) ((__uint32_t)(_x))
#define le64toh(_x) ((__uint64_t)(_x))
#define htole64(_x) ((__uint64_t)(_x))
#endif

#endif/* _ENDIAN_H__ */
