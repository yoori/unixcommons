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



#include <HTTP/UrlAddress.hpp>

#include <JavaCommons/JavaCommons.hpp>

#include <HttpJavaFunctions/HttpJavaFunctions.hpp>

#include <Language/GenericSegmentor/Polyglot.hpp>
#include <Language/ChineeseSegmentor/NLPIR.hpp>

#include <Language/BLogic/NormalizeTrigger.hpp>


namespace
{
  Language::Segmentor::SegmentorInterface_var polyglot, nlpir;

  jstring
  normalize_keyword(JNIEnv* env, jstring keyword,
    const Language::Segmentor::SegmentorInterface* segmentor) throw ()
  {
    try
    {
      std::string normalized;
      {
        JavaCommons::StrPtr original(env, keyword);
        Language::Trigger::normalize(String::SubString(original.c_str()),
          normalized, segmentor);
      }
      return env->NewStringUTF(normalized.c_str());
    }
    catch (...)
    {
    }
    return env->NewStringUTF("");
  }
}

JNIEXPORT jstring JNICALL
Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_initialize(
  JNIEnv* env, jclass /*cls*/)
{
  try
  {
    polyglot = new Language::Segmentor::NormalizePolyglotSegmentor(
      "/opt/oix/polyglot/dict/");
    nlpir = new Language::Segmentor::Chineese::NlpirSegmentor;
  }
  catch (const eh::Exception& ex)
  {
    return env->NewStringUTF(ex.what());
  }
  catch (...)
  {
    return env->NewStringUTF("Unknown exception");
  }
  return env->NewStringUTF("INIT_SUCCESS");
}

JNIEXPORT jstring JNICALL
Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeURL(
  JNIEnv* env, jobject /*object*/, jstring url)
{
  try
  {
    JavaCommons::StrPtr real_url(env, url);
    return env->NewStringUTF(HTTP::normalize_http_address(
      String::SubString(real_url.c_str())).c_str());
  }
  catch (...)
  {
  }

  return env->NewStringUTF("");
}

JNIEXPORT jstring JNICALL
Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeKeyword(
  JNIEnv* env, jobject /*object*/, jstring keyword)
{
  return normalize_keyword(env, keyword, polyglot);
}

JNIEXPORT jstring JNICALL
Java_com_phorm_oix_util_normalization_UnixCommonsNormalizer_normalizeChineseKeyword(
    JNIEnv* env, jobject /*object*/, jstring keyword)
{
  return normalize_keyword(env, keyword, nlpir);
}
