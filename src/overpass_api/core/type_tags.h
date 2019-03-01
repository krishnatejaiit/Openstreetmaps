/** Copyright 2008, 2009, 2010, 2011, 2012 Roland Olbricht
*
* This file is part of Overpass_API.
*
* Overpass_API is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Overpass_API is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Overpass_API.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DE__OSM3S___OVERPASS_API__CORE__TYPE_TAGS_H
#define DE__OSM3S___OVERPASS_API__CORE__TYPE_TAGS_H

#include <map>
#include <set>
#include <vector>

#include "basic_types.h"


struct Unsupported_Error
{
  Unsupported_Error(const string& method_name_) : method_name(method_name_) {}
  string method_name;
};


template< class Id_Type >
struct Tag_Entry
{
  uint32 index;
  string key;
  string value;
  vector< Id_Type > ids;
};


struct Tag_Index_Local
{
  uint32 index;
  string key;
  string value;
  
  Tag_Index_Local() {}
  
  Tag_Index_Local(void* data)
  {
    index = (*((uint32*)data + 1))<<8;
    key = string(((int8*)data + 7), *(uint16*)data);
    value = string(((int8*)data + 7 + key.length()),
		   *((uint16*)data + 1));
  }
  
  uint32 size_of() const
  {
    return 7 + key.length() + value.length();
  }
  
  static uint32 size_of(void* data)
  {
    return (*((uint16*)data) + *((uint16*)data + 1) + 7);
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = key.length();
    *((uint16*)data + 1) = value.length();
    *((uint32*)data + 1) = index>>8;
    memcpy(((uint8*)data + 7), key.data(), key.length());
    memcpy(((uint8*)data + 7 + key.length()), value.data(),
	   value.length());
  }
  
  bool operator<(const Tag_Index_Local& a) const
  {
    if ((index & 0x7fffffff) != (a.index & 0x7fffffff))
      return ((index & 0x7fffffff) < (a.index & 0x7fffffff));
    if (index != a.index)
      return (index < a.index);
    if (key != a.key)
      return (key < a.key);
    return (value < a.value);
  }
  
  bool operator==(const Tag_Index_Local& a) const
  {
    if (index != a.index)
      return false;
    if (key != a.key)
      return false;
    return (value == a.value);
  }
  
  static uint32 max_size_of()
  {
    throw Unsupported_Error("static uint32 Tag_Index_Global::max_size_of()");
    return 0;
  }
};


template< class TIndex >
void formulate_range_query
    (set< pair< Tag_Index_Local, Tag_Index_Local > >& range_set,
     const set< TIndex >& coarse_indices)
{
  for (typename set< TIndex >::const_iterator
    it(coarse_indices.begin()); it != coarse_indices.end(); ++it)
  {
    Tag_Index_Local lower, upper;
    lower.index = it->val();
    lower.key = "";
    lower.value = "";
    upper.index = it->val() + 1;
    upper.key = "";
    upper.value = "";
    range_set.insert(make_pair(lower, upper));
  }
}


template< class TIndex, class TObject >
void generate_ids_by_coarse
  (set< TIndex >& coarse_indices,
   map< uint32, vector< typename TObject::Id_Type > >& ids_by_coarse,
   const map< TIndex, vector< TObject > >& items)
{
  for (typename map< TIndex, vector< TObject > >::const_iterator
    it(items.begin()); it != items.end(); ++it)
  {
    coarse_indices.insert(TIndex(it->first.val() & 0x7fffff00));
    for (typename vector< TObject >::const_iterator it2(it->second.begin());
        it2 != it->second.end(); ++it2)
      ids_by_coarse[it->first.val() & 0x7fffff00].push_back(it2->id);
  }
}


struct Tag_Index_Global
{
  string key;
  string value;
  
  Tag_Index_Global() {}
  
  Tag_Index_Global(void* data)
  {
    key = string(((int8*)data + 4), *(uint16*)data);
    value = string(((int8*)data + 4 + key.length()),
		   *((uint16*)data + 1));
  }
  
  uint32 size_of() const
  {
    return 4 + key.length() + value.length();
  }
  
  static uint32 size_of(void* data)
  {
    return (*((uint16*)data) + *((uint16*)data + 1) + 4);
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = key.length();
    *((uint16*)data + 1) = value.length();
    memcpy(((uint8*)data + 4), key.data(), key.length());
    memcpy(((uint8*)data + 4 + key.length()), value.data(),
	   value.length());
  }
  
  bool operator<(const Tag_Index_Global& a) const
  {
    if (key != a.key)
      return (key < a.key);
    return (value < a.value);
  }
  
  bool operator==(const Tag_Index_Global& a) const
  {
    if (key != a.key)
      return false;
    return (value == a.value);
  }
  
  static uint32 max_size_of()
  {
    throw Unsupported_Error("static uint32 Tag_Index_Global::max_size_of()");
    return 0;
  }
};


#endif
