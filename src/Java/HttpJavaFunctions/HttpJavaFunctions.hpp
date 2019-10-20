/* 
 * This file is part of the UnixCommons distribution (https://github.com/yoori/unixcommons).
 * UnixCommons contains help classes and functions for Unix Server application writing
 *
 * Copyright (c) 2012 Yuri Kuznecov <yuri.kuznecov@gmail.com>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef JAVA_HTTPJAVAFUNCTIONS_HPP
#define JAVA_HTTPJAVAFUNCTIONS_HPP

#include <jni.h>

extern "C"
{
  JNIEXPORT jstring JNICALL
  Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_initialize(
    JNIEnv* env, jclass cls);

  JNIEXPORT jstring JNICALL
  Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeURL(
    JNIEnv* env, jobject object, jstring url);

  JNIEXPORT jstring JNICALL
  Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeKeyword(
    JNIEnv* env, jobject object, jstring keyword);

  JNIEXPORT jstring JNICALL
  Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeChineseKeyword(
    JNIEnv* env, jobject object, jstring keyword);
}

#endif
