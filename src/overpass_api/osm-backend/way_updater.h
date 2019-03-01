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

#ifndef DE__OSM3S___OVERPASS_API__OSM_BACKEND__WAY_UPDATER_H
#define DE__OSM3S___OVERPASS_API__OSM_BACKEND__WAY_UPDATER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "../../template_db/block_backend.h"
#include "../../template_db/random_file.h"
#include "../../template_db/transaction.h"
#include "../core/datatypes.h"
#include "../core/settings.h"

using namespace std;


struct Update_Way_Logger
{
public:
  void insertion(const Way& way)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = insert.find(way.id);
    if (it == insert.end())
      insert.insert(make_pair(way.id, make_pair< Way, OSM_Element_Metadata* >(way, 0)));
    else
      it->second.first = way;
    
    it = keep.find(way.id);
    if (it != keep.end())
      keep.erase(it);
  }
  
  void insertion(Way::Id_Type id, const OSM_Element_Metadata& meta)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = insert.find(id);
    if (it != insert.end())
    {
      if (it->second.second)
        delete it->second.second;
      it->second.second = new OSM_Element_Metadata(meta);
    }
  }
  
  void deletion(const Uint31_Index& index, const Way_Skeleton& skel)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = erase.find(skel.id);
    if (it == erase.end())
    {
      it = erase.insert(make_pair(skel.id, make_pair< Way, OSM_Element_Metadata* >
          (Way(skel.id.val()), 0))).first;
    }
    else
      it->second.first = Way(skel.id.val());
    it->second.first.index = index.val();
    it->second.first.nds = skel.nds;
    it->second.first.segment_idxs = skel.segment_idxs;

    it = keep.find(skel.id);
    if (it != keep.end())
      keep.erase(it);
  }
  
  void keeping(const Uint31_Index& index, const Way_Skeleton& skel)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = erase.find(skel.id);
    if (it != erase.end())
      return;
    
    it = insert.find(skel.id);
    if (it != insert.end())
      return;
    
    it = keep.find(skel.id);
    if (it == keep.end())
    {
      it = keep.insert(make_pair(skel.id, make_pair< Way, OSM_Element_Metadata* >
          (Way(skel.id.val()), 0))).first;
    }
    else
      it->second.first = Way(skel.id.val());
    it->second.first.index = index.val();
    it->second.first.nds = skel.nds;
    it->second.first.segment_idxs = skel.segment_idxs;
  }
  
  void deletion(const Tag_Index_Local& index, const Uint32_Index& ref)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = erase.find(ref.val());
    if (it != erase.end())
      it->second.first.tags.push_back(make_pair(index.key, index.value));
    it = keep.find(ref.val());
    if (it != keep.end())
      it->second.first.tags.push_back(make_pair(index.key, index.value));
  }
  
  void keeping(const Tag_Index_Local& index, const Uint32_Index& ref)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = keep.find(ref.val());
    if (it != keep.end())
      it->second.first.tags.push_back(make_pair(index.key, index.value));
  }
  
  void deletion(const Uint31_Index& index,
		const OSM_Element_Metadata_Skeleton< Way::Id_Type >& meta_skel)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = erase.find(meta_skel.ref);
    if (it != erase.end())
    {
      if (it->second.second)
        delete it->second.second;
      OSM_Element_Metadata* meta = new OSM_Element_Metadata();
      meta->version = meta_skel.version;
      meta->timestamp = meta_skel.timestamp;
      meta->changeset = meta_skel.changeset;
      meta->user_id = meta_skel.user_id;
      it->second.second = meta;
    }
    it = keep.find(meta_skel.ref);
    if (it != keep.end())
    {
      if (it->second.second)
        delete it->second.second;
      OSM_Element_Metadata* meta = new OSM_Element_Metadata();
      meta->version = meta_skel.version;
      meta->timestamp = meta_skel.timestamp;
      meta->changeset = meta_skel.changeset;
      meta->user_id = meta_skel.user_id;
      it->second.second = meta;
    }
  }
  
  void keeping(const Uint31_Index& index,
	       const OSM_Element_Metadata_Skeleton< Way::Id_Type >& meta_skel)
  {
    map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator it = keep.find(meta_skel.ref);
    if (it != keep.end())
    {
      if (it->second.second)
        delete it->second.second;
      OSM_Element_Metadata* meta = new OSM_Element_Metadata();
      meta->version = meta_skel.version;
      meta->timestamp = meta_skel.timestamp;
      meta->changeset = meta_skel.changeset;
      meta->user_id = meta_skel.user_id;
      it->second.second = meta;
    }
  }
  
  void set_delete_meta_data(const vector< OSM_Element_Metadata_Skeleton< Way::Id_Type > >& meta_to_delete_)
  {
    for (typename vector< OSM_Element_Metadata_Skeleton< Way::Id_Type > >::const_iterator
        it = meta_to_delete_.begin(); it != meta_to_delete_.end(); ++it)
    {
      OSM_Element_Metadata meta;
      meta.version = it->version;
      meta.timestamp = it->timestamp;
      meta.changeset = it->changeset;
      meta.user_id = it->user_id;
      meta_to_delete.push_back(make_pair(it->ref, meta));
    }
  }
  
  void request_user_names(map< uint32, string >& user_names)
  {
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator
        it = insert_begin(); it != insert_end(); ++it)
    {
      if (it->second.second)
        user_names[it->second.second->user_id];
    }
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator
        it = keep_begin(); it != keep_end(); ++it)
    {
      if (it->second.second)
        user_names[it->second.second->user_id];
    }
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator
        it = erase_begin(); it != erase_end(); ++it)
    {
      if (it->second.second)
        user_names[it->second.second->user_id];
    }
    
    for (vector< pair< Way::Id_Type, OSM_Element_Metadata > >::const_iterator
        it = meta_to_delete.begin(); it != meta_to_delete.end(); ++it)
      user_names[it->second.user_id];
  }
  
  void set_user_names(map< uint32, string >& user_names)
  {
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator
        it = insert_begin(); it != insert_end(); ++it)
    {
      if (it->second.second)
        it->second.second->user_name = user_names[it->second.second->user_id];
    }  
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator
        it = keep_begin(); it != keep_end(); ++it)
    {
      if (it->second.second)
        it->second.second->user_name = user_names[it->second.second->user_id];
    }  
    for (map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator
        it = erase_begin(); it != erase_end(); ++it)
    {
      if (it->second.second)
        it->second.second->user_name = user_names[it->second.second->user_id];
    }  
    
    for (vector< pair< Way::Id_Type, OSM_Element_Metadata > >::iterator
        it = meta_to_delete.begin(); it != meta_to_delete.end(); ++it)
      it->second.user_name = user_names[it->second.user_id];
  }

  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator insert_begin() const
  { return insert.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator insert_end() const
  { return insert.end(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator keep_begin() const
  { return keep.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator keep_end() const
  { return keep.end(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator erase_begin() const
  { return erase.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator erase_end() const
  { return erase.end(); }
  
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator insert_begin()
  { return insert.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator insert_end()
  { return insert.end(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator keep_begin()
  { return keep.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator keep_end()
  { return keep.end(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator erase_begin()
  { return erase.begin(); }
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::iterator erase_end()
  { return erase.end(); }

  const Way* get_erased(Way::Id_Type ref) const;
  const Way* get_inserted(Way::Id_Type ref) const;
  
  const OSM_Element_Metadata* get_erased_meta(Way::Id_Type ref) const
  { return binary_pair_search(meta_to_delete, ref); }
  
  ~Update_Way_Logger();
  
private:
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > > insert;
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > > keep;
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > > erase;
  vector< pair< Way::Id_Type, OSM_Element_Metadata > > meta_to_delete;
};


inline const Way* Update_Way_Logger::get_erased(Way::Id_Type ref) const
{
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator it = erase.find(ref);
  if (it != erase.end())
    return &it->second.first;
  
  it = keep.find(ref);
  if (it != keep.end())
    return &it->second.first;
  
  return 0;
}


inline const Way* Update_Way_Logger::get_inserted(Way::Id_Type ref) const
{
  map< Way::Id_Type, pair< Way, OSM_Element_Metadata* > >::const_iterator it = insert.find(ref);
  if (it != insert.end())
    return &it->second.first;
  
  it = keep.find(ref);
  if (it != keep.end())
    return &it->second.first;
  
  return 0;
}


struct Way_Updater
{
  Way_Updater(Transaction& transaction, bool meta);
  
  Way_Updater(string db_dir, bool meta);
  
  void set_id_deleted(Way::Id_Type id, const OSM_Element_Metadata* meta = 0)
  {
    ids_to_modify.push_back(make_pair(id, false));
    if (meta)
    {
      user_by_id[meta->user_id] = meta->user_name;
      OSM_Element_Metadata_Skeleton< Way::Id_Type > meta_skel;
      meta_skel.ref = id;
      meta_skel.version = meta->version;
      meta_skel.changeset = meta->changeset;
      meta_skel.timestamp = meta->timestamp;
      meta_skel.user_id = meta->user_id;
      ways_meta_to_delete.push_back(meta_skel);
    }
  }
  
  void set_way
      (uint32 id, uint32 lat, uint32 lon, const vector< pair< string, string > >& tags,
       const vector< Node::Id_Type > nds,
       const OSM_Element_Metadata* meta = 0)
  {
    ids_to_modify.push_back(make_pair(id, true));
    
    Way way;
    way.id = id;
    way.nds = nds;
    way.tags = tags;
    ways_to_insert.push_back(way);
    if (meta)
    {
      user_by_id[meta->user_id] = meta->user_name;
      OSM_Element_Metadata_Skeleton< Way::Id_Type > meta_skel;
      meta_skel.ref= way.id;
      meta_skel.version = meta->version;
      meta_skel.changeset = meta->changeset;
      meta_skel.timestamp = meta->timestamp;
      meta_skel.user_id = meta->user_id;
      ways_meta_to_insert.push_back(make_pair(meta_skel, 0));
    }
  }
  
  void set_way(const Way& way,
	       const OSM_Element_Metadata* meta = 0)
  {
    ids_to_modify.push_back(make_pair(way.id, true));
    ways_to_insert.push_back(way);
    if (meta)
    {
      user_by_id[meta->user_id] = meta->user_name;
      OSM_Element_Metadata_Skeleton< Way::Id_Type > meta_skel;
      meta_skel.ref= way.id;
      meta_skel.version = meta->version;
      meta_skel.changeset = meta->changeset;
      meta_skel.timestamp = meta->timestamp;
      meta_skel.user_id = meta->user_id;
      ways_meta_to_insert.push_back(make_pair(meta_skel, 0));
    }
  }
  
  void update(Osm_Backend_Callback* callback, bool partial,
	      Update_Way_Logger* update_logger);
  
  void update_moved_idxs
      (Osm_Backend_Callback* callback, const vector< pair< Node::Id_Type, Uint32_Index > >& moved_nodes,
       Update_Way_Logger* update_logger);
  
  const vector< pair< Way::Id_Type, Uint31_Index > >& get_moved_ways() const
  {
    return moved_ways;
  }
  
private:
  uint32 update_counter;
  Transaction* transaction;
  bool external_transaction;
  bool partial_possible;
  vector< pair< Way::Id_Type, bool > > ids_to_modify;
  vector< Way > ways_to_insert;
  vector< pair< Way::Id_Type, Uint31_Index > > moved_ways;
  string db_dir;
  
  bool meta;
  vector< pair< OSM_Element_Metadata_Skeleton< Way::Id_Type >, uint32 > > ways_meta_to_insert;
  vector< OSM_Element_Metadata_Skeleton< Way::Id_Type > > ways_meta_to_delete;
  map< uint32, string > user_by_id;
  
  void find_affected_ways(const vector< pair< Node::Id_Type, Uint32_Index > >& moved_nodes,
       Update_Way_Logger* update_logger);
  
  void compute_indexes(vector< Way* >& ways_ptr);

  void update_way_ids(const vector< Way* >& ways_ptr, map< uint32, vector< Way::Id_Type > >& to_delete,
		      bool record_minuscule_moves);
  
  void update_members
      (const vector< Way* >& ways_ptr, const map< uint32, vector< Way::Id_Type > >& to_delete,
       Update_Way_Logger* update_logger);
  
  void merge_files(const vector< string >& froms, string into);
};

#endif
