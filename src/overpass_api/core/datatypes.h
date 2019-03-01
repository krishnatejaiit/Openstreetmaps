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

#ifndef DE__OSM3S___OVERPASS_API__CORE__DATATYPES_H
#define DE__OSM3S___OVERPASS_API__CORE__DATATYPES_H

#include <cstring>
#include <map>
#include <set>
#include <vector>

#include "basic_types.h"
#include "type_node.h"
#include "type_way.h"
#include "type_relation.h"
#include "type_tags.h"
#include "type_area.h"


struct String_Object
{
  String_Object(string s) : value(s) {}
  String_Object(void* data) : value()
  {
    value = string(((int8*)data + 2), *(uint16*)data);
  }
  
  uint32 size_of() const
  {
    return value.length() + 2;
  }
  
  static uint32 size_of(void* data)
  {
    return *(uint16*)data + 2;
  }
  
  void to_data(void* data) const
  {
    *(uint16*)data = value.length();
    memcpy(((uint8*)data + 2), value.data(), value.length());
  }
  
  bool operator<(const String_Object& index) const
  {
    return this->value < index.value;
  }
  
  bool operator==(const String_Object& index) const
  {
    return this->value == index.value;
  }
  
  string val() const
  {
    return value;
  }
  
  protected:
    string value;
};


template< typename First, typename Second >
struct Pair_Comparator_By_Id {
  bool operator() (const pair< First, Second >& a, const pair< First, Second >& b)
  {
    return (a.first < b.first);
  }
};


template< typename First, typename Second >
struct Pair_Equal_Id {
  bool operator() (const pair< First, Second >& a, const pair< First, Second >& b)
  {
    return (a.first == b.first);
  }
};


template < class T >
T* binary_search_for_id(vector< T >& vect, typename T::Id_Type id)
{
  uint32 lower(0);
  uint32 upper(vect.size());
  
  while (upper > lower)
  {
    uint32 pos((upper + lower)/2);
    if (id < vect[pos].id)
      upper = pos;
    else if (vect[pos].id == id)
      return &(vect[pos]);
    else
      lower = pos + 1;
  }
  return 0;
}


template < class TObject >
TObject* binary_ptr_search_for_id(vector< TObject* >& vect, typename TObject::Id_Type id)
{
  uint32 lower(0);
  uint32 upper(vect.size());
  
  while (upper > lower)
  {
    uint32 pos((upper + lower)/2);
    if (id < vect[pos]->id)
      upper = pos;
    else if (vect[pos]->id == id)
      return vect[pos];
    else
      lower = pos + 1;
  }
  return 0;
}


template < class Id_Type, class TObject >
const TObject* binary_pair_search(const vector< pair< Id_Type, TObject> >& vect, Id_Type id)
{
  uint32 lower(0);
  uint32 upper(vect.size());
  
  while (upper > lower)
  {
    uint32 pos((upper + lower)/2);
    if (id < vect[pos].first)
      upper = pos;
    else if (vect[pos].first == id)
      return &vect[pos].second;
    else
      lower = pos + 1;
  }
  return 0;
}


/**
  * A dataset that is referred in the scripts by a variable.
  */
struct Set
{
  map< Uint32_Index, vector< Node_Skeleton > > nodes;
  map< Uint31_Index, vector< Way_Skeleton > > ways;
  map< Uint31_Index, vector< Relation_Skeleton > > relations;
  map< Uint31_Index, vector< Area_Skeleton > > areas;
};


struct Error_Output
{
  virtual void add_encoding_error(const string& error) = 0;
  virtual void add_parse_error(const string& error, int line_number) = 0;
  virtual void add_static_error(const string& error, int line_number) = 0;
  // void add_sanity_error(const string& error);
  
  virtual void add_encoding_remark(const string& error) = 0;
  virtual void add_parse_remark(const string& error, int line_number) = 0;
  virtual void add_static_remark(const string& error, int line_number) = 0;
  // void add_sanity_remark(const string& error);
  
  virtual void runtime_error(const string& error) = 0;
  virtual void runtime_remark(const string& error) = 0;
  
  virtual void display_statement_progress
    (uint timer, const string& name, int progress, int line_number,
     const vector< pair< uint, uint > >& stack) = 0;
  
  virtual bool display_encoding_errors() = 0;
  virtual bool display_parse_errors() = 0;
  virtual bool display_static_errors() = 0;
  
  virtual void add_padding(const string& padding) = 0;
  
  static const uint QUIET = 1;
  static const uint CONCISE = 2;
  static const uint PROGRESS = 3;
  static const uint ASSISTING = 4;
  static const uint VERBOSE = 5;
};


class Osm_Backend_Callback
{
  public:
    virtual void update_started() = 0;
    virtual void compute_indexes_finished() = 0;
    virtual void update_ids_finished() = 0;
    virtual void update_coords_finished() = 0;
    virtual void prepare_delete_tags_finished() = 0;
    virtual void tags_local_finished() = 0;
    virtual void tags_global_finished() = 0;
    virtual void flush_roles_finished() = 0;
    virtual void update_finished() = 0;
    virtual void partial_started() = 0;
    virtual void partial_finished() = 0;
    
    virtual void parser_started() = 0;
    virtual void node_elapsed(Node::Id_Type id) = 0;
    virtual void nodes_finished() = 0;
    virtual void way_elapsed(Way::Id_Type id) = 0;
    virtual void ways_finished() = 0;
    virtual void relation_elapsed(Relation::Id_Type id) = 0;
    virtual void relations_finished() = 0;
    virtual void parser_succeeded() = 0;
};


struct User_Data
{
  uint32 id;
  string name;
  
  User_Data() : id(0) {}
  
  User_Data(void* data)
  {
    id = *(uint32*)data;
    name = string(((int8*)data + 6), *(uint16*)((int8*)data + 4));
  }
  
  uint32 size_of() const
  {
    return 6 + name.length();
  }
  
  static uint32 size_of(void* data)
  {
    return 6 + *(uint16*)((int8*)data + 4);
  }
  
  void to_data(void* data) const
  {
    *(uint32*)data = id;
    *(uint16*)((int8*)data + 4) = name.length();
    memcpy(((int8*)data + 6), name.data(), name.length());
  }
  
  bool operator<(const User_Data& a) const
  {
    return (id < a.id);
  }
  
  bool operator==(const User_Data& a) const
  {
    return (id == a.id);
  }
};


struct OSM_Element_Metadata
{
  OSM_Element_Metadata() : user_id(0) {}
  
  uint32 version;
  uint64 timestamp;
  uint32 changeset;
  uint32 user_id;
  string user_name;
};


template< class Id_Type >
struct OSM_Element_Metadata_Skeleton
{
  Id_Type ref;
  uint32 version;
  uint64 timestamp;
  uint32 changeset;
  uint32 user_id;
  
  OSM_Element_Metadata_Skeleton() : ref(0u), version(0), timestamp(0), changeset(0), user_id(0) {}
  
  OSM_Element_Metadata_Skeleton(Id_Type ref_)
    : ref(ref_), version(0), timestamp(0), changeset(0), user_id(0) {}
  
  OSM_Element_Metadata_Skeleton(void* data)
    : ref(*(uint32*)data)
  {
    version = *(uint32*)((int8*)data + 4);
    timestamp = (*(uint64*)((int8*)data + 8) & 0xffffffffffull);
    changeset = *(uint32*)((int8*)data + 13);
    user_id = *(uint32*)((int8*)data + 17);
  }
  
  uint32 size_of() const
  {
    return 21;
  }
  
  static uint32 size_of(void* data)
  {
    return 21;
  }
  
  void to_data(void* data) const
  {
    *(uint32*)data = ref.val();
    *(uint32*)((int8*)data + 4) = version;
    *(uint64*)((int8*)data + 8) = timestamp;
    *(uint32*)((int8*)data + 13) = changeset;
    *(uint32*)((int8*)data + 17) = user_id;
  }
  
  bool operator<(const OSM_Element_Metadata_Skeleton& a) const
  {
    return (ref < a.ref);
  }
  
  bool operator==(const OSM_Element_Metadata_Skeleton& a) const
  {
    return (ref == a.ref);
  }
};


template< class TIndex, class TObject >
const pair< TIndex, const TObject* >* binary_search_for_pair_id
    (const vector< pair< TIndex, const TObject* > >& vect, typename TObject::Id_Type id)
{
  uint32 lower(0);
  uint32 upper(vect.size());
  
  while (upper > lower)
  {
    uint32 pos((upper + lower)/2);
    if (id < vect[pos].second->id)
      upper = pos;
    else if (vect[pos].second->id == id)
      return &(vect[pos]);
    else
      lower = pos + 1;
  }
  return 0;
}


#endif
