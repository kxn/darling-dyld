/*
 * Copyright (c) 2021-2026 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

// Extracted from Apple dyld libdyld/utils.cpp; see vendor/apple-dyld-runnable/reference.json.
// Only this standalone API is built by the legacy Darling libdyld.
#include <TargetConditionals.h>
#include <mach-o/utils.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <string.h>

#if !TARGET_OS_SIMULATOR
static bool hasFile(const char* path)
{
    struct stat statBuf;
    if ( stat(path, &statBuf) == 0 )
        return true;
    char devPath[PATH_MAX];
    strlcpy(devPath, path, sizeof(devPath));
    strlcat(devPath, ".development", sizeof(devPath));
    return ( stat(devPath, &statBuf) == 0 );
}
#endif


void macho_for_each_runnable_arch_name(void (^ _Nonnull callback)(const char* _Nonnull archName, bool* _Nonnull stop))
{
    bool stop = false;
#if TARGET_OS_SIMULATOR
    // FIXME:  Should $SIMULATOR_ARCHS be used? Or is this API only about what the current simulator instance can run?
    // fallthrough to arch specific handling
#elif TARGET_OS_OSX
    if ( hasFile("/System/Cryptexes/OS/System/Library/dyld/dyld_shared_cache_arm64e") ) {
        // Apple Silicon mac
        callback("arm64e", &stop);
        if ( stop )
            return;
        callback("arm64", &stop);
        if ( stop )
            return;
        if ( hasFile("/System/Cryptexes/OS/System/Library/dyld/dyld_shared_cache_x86_64") ) {
            // has Rosetta support
            callback("x86_64", &stop);
        }
        return;
    }
    else if ( hasFile("/System/Cryptexes/OS/System/Library/dyld/dyld_shared_cache_x86_64h") ) {
        // Intel mac
        callback("x86_64h", &stop);
        if ( stop )
            return;
        callback("x86_64", &stop);
        return;
    }
    else if ( hasFile("/System/Cryptexes/OS/System/Library/dyld/dyld_shared_cache_x86_64") ) {
        // Old Intel mac
        callback("x86_64", &stop);
        return;
    }
#elif TARGET_OS_IOS
    if ( hasFile("/System/Cryptexes/OS/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64e") ) {
        // iPhone or iPad
        callback("arm64e", &stop);
        if ( stop )
            return;
        callback("arm64", &stop);
        return;
    }
    else if ( hasFile("/System/Cryptexes/OS/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64") ) {
        // old iPhone or ipad
        callback("arm64", &stop);
        return;
    }
#elif TARGET_OS_WATCH
    // gather grading for architectures
    struct NameAndGrade { const char* name; int grade=0; };
    NameAndGrade gn[3] = { { "arm64e"}, { "arm64"}, { "arm64_32"} };
    if ( hasFile("/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64e") ) {
        // M11 or later watch that supports 64-bit userland
        gn[0].grade = Architecture::arm64e.kernelGrade();
        gn[1].grade = Architecture::arm64.kernelGrade();
    }
    if ( hasFile("/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64_32") ) {
        gn[2].grade = Architecture::arm64_32.kernelGrade();
    }
    // sort three entries by grade (unrolled bubble sort)
    if ( gn[0].grade < gn[1].grade )
        std::swap(gn[0], gn[1]);
    if ( gn[1].grade < gn[2].grade )
        std::swap(gn[1], gn[2]);
    if ( gn[0].grade < gn[1].grade )
        std::swap(gn[0], gn[1]);
    // return them in grading order
    int usableCount = 0;
    for ( int i=0; i < 3; ++i ) {
        // skip NameAndGrade elements with no grade value
        if ( gn[i].grade != 0 ) {
            ++usableCount;
            callback(gn[i].name, &stop);
            if ( stop )
                return;
        }
    }
    if ( usableCount > 0 )
        return;
    // otherwise fall into no-dyld-cache case below
#else
    if ( hasFile("/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64e") ) {
        // Apple TV or other device with arm64e support
        callback("arm64e", &stop);
        if ( stop )
            return;
        callback("arm64", &stop);
        return;
    }
    else if ( hasFile("/System/Library/Caches/com.apple.dyld/dyld_shared_cache_arm64") ) {
        callback("arm64", &stop);
        return;
    }
#endif
    if ( stop )
        return;

    // no dyld cache, must be RAMDisk
#if __arm64e__
    callback("arm64e", &stop);
    if ( stop )
        return;
    callback("arm64", &stop);
#elif __arm64__
  #if __LP64__
    callback("arm64", &stop);
  #else
    callback("arm64_32", &stop);
  #endif
#elif __x86_64__
    callback("x86_64", &stop);
#else
#error "Unknown arch"
#endif
}
