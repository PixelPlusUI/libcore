/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "IoUtils"

#include "JNIHelp.h"
#include "JniConstants.h"
#include "NetworkUtilities.h"
#include "ScopedPrimitiveArray.h"

#include <errno.h>
#include <unistd.h>

static void IoUtils_close(JNIEnv* env, jclass, jobject fileDescriptor) {
    // Get the FileDescriptor's 'fd' field and clear it.
    // We need to do this before we can throw an IOException (http://b/3222087).
    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    jniSetFileDescriptorOfFD(env, fileDescriptor, -1);

    // Even if close(2) fails with EINTR, the fd will have been closed.
    // Using TEMP_FAILURE_RETRY will either lead to EBADF or closing someone else's fd.
    // http://lkml.indiana.edu/hypermail/linux/kernel/0509.1/0877.html
    jint rc = close(fd);
    if (rc == -1) {
        jniThrowIOException(env, errno);
    }
}

static jint IoUtils_getFd(JNIEnv* env, jclass, jobject fileDescriptor) {
    return jniGetFDFromFileDescriptor(env, fileDescriptor);
}

static void IoUtils_setBlocking(JNIEnv* env, jclass, jobject fileDescriptor, jboolean blocking) {
    int fd = jniGetFDFromFileDescriptor(env, fileDescriptor);
    if (fd == -1) {
        return;
    }
    if (!setBlocking(fd, blocking)) {
        jniThrowIOException(env, errno);
    }
}

static JNINativeMethod gMethods[] = {
    NATIVE_METHOD(IoUtils, close, "(Ljava/io/FileDescriptor;)V"),
    NATIVE_METHOD(IoUtils, getFd, "(Ljava/io/FileDescriptor;)I"),
    NATIVE_METHOD(IoUtils, setBlocking, "(Ljava/io/FileDescriptor;Z)V"),
};
int register_libcore_io_IoUtils(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "libcore/io/IoUtils", gMethods, NELEM(gMethods));
}
