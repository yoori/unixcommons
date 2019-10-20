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



#include <Language/SegmentorManager/SegmentorManager.hpp>
#include "DummySegmentor.hpp"
#include <vector>
#include <iostream>

typedef std::vector<Language::Segmentor::SegmentorInterface_var> CurHolder;

template <typename CompositeType>
void test_me(const std::string& name, bool strict);

template <typename CompositeType>
void put_spaces_scenario(std::ostream& err, bool strict);

template <typename CompositeType>
void segmentation_scenario(std::ostream& err, bool strict);

int main(int /*argc*/, char** /*argv*/)
{
  try
  {
    test_me<Language::Segmentor::CompositeSegmentor>(
      "CompositeSegmentor", true);

    return 0;
  }
  catch (const eh::Exception& e)
  {
    std::cerr << "main: eh::Exception caught: " << e.what() << std::endl;
  }

  return -1;
}

template <typename CompositeType>
void
test_me(const std::string& name, bool strict)
{
  std::ostringstream errors;
  put_spaces_scenario<CompositeType>(errors, strict);
  segmentation_scenario<CompositeType>(errors, strict);

  std::cout << name << " test completes ";
  if (errors.str().empty())
  {
    std::cout << "successfully";
  }
  else
  {
    std::cout << "with errors";
    std::cerr << "Errors of " << name << ":\n" << errors.str() << std::endl;
  }
  std::cout << std::endl;
}

template <typename CompositeType>
void
put_spaces_scenario(std::ostream& err, bool strict)
{
  const char TEST[] = "APRICOTMY";
  std::ostringstream out;
  CurHolder segms;
  const size_t SEGMS_COUNT = 8;

  for (size_t i = 0; i < SEGMS_COUNT; ++i)
  {
    segms.push_back(Language::Segmentor::SegmentorInterface_var(
      new DummySegmentor(i + 1, out)));
  }

   Language::Segmentor::SegmentorInterface_var segm(
     new CompositeType(segms.begin(), segms.end()));

  std::string res;
  segm->put_spaces(res, TEST, sizeof(TEST) - 1);

  if (res != "A P R I C O T M Y")
  {
    err << SEGMS_COUNT << " DummySegmentor-s should put "
        << SEGMS_COUNT << " spaces. "
           "Src: " << TEST
        << " Expected: A P R I C O T M Y "
           "Got " << res
        << std::endl;
  }
  if (strict && out.str() != "#1#2#3#4#5#6#7#8")
  {
    err << SEGMS_COUNT << " DummySegmentor-s should be invoked "
           " once (from put_spaces) in alphabetical order. "
        << "Expected: #1#2#3#4#5#6#7#8 "
           "Got " << out.str()
        << std::endl;
  }
}

template <typename CompositeType>
void
segmentation_scenario(std::ostream& err, bool strict)
{
  const char TEST[] = "APRICOTMY";
  std::ostringstream out;
  CurHolder segms;
  const size_t SEGMS_COUNT = 4;

  for (size_t i = 0; i < SEGMS_COUNT; ++i)
  {
    segms.push_back(Language::Segmentor::SegmentorInterface_var(
      new DummySegmentor(i + 1, out)));
  }

  Language::Segmentor::SegmentorInterface_var segm(
    new CompositeType(segms.begin(), segms.end()));

  Language::Segmentor::WordsList wlist;
  segm->segmentation(wlist, TEST, sizeof(TEST) - 1);

  std::string res;
  Language::Segmentor::WordsList::const_iterator it = wlist.begin();
  Language::Segmentor::WordsList::const_iterator end = wlist.end();
  while(it != end)
  {
    res += *it;
    ++it;
    if (it != end)
    {
      res += ' ';
    }
  }

  if (res != "A P R I C O T M Y")
  {
    err << SEGMS_COUNT << " DummySegmentor-s should segment as follows: "
           "Src: " << TEST
        << " Expected: A P R I C O T M Y "
           "Got " << res
        << std::endl;
  }
  if (strict && out.str() != "#1#2#2#3#3#3#3#4#4#4#4#4#4#4#4")
  {
    err << "Every of " << SEGMS_COUNT << " DummySegmentor-s should be "
           "invoked (from segmentation) double times than previous "
           "(begin from 1) in alphabetical order. "
        << "Expected: #1#2#2#3#3#3#3#4#4#4#4#4#4#4#4 "
           "Got " << out.str()
        << std::endl;
  }
}
