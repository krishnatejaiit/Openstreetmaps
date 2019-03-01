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

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <vector>

#include "../../template_db/block_backend.h"
#include "../../template_db/random_file.h"
#include "../osm-backend/area_updater.h"
#include "make_area.h"
#include "print.h"

using namespace std;

bool Make_Area_Statement::is_used_ = false;

Generic_Statement_Maker< Make_Area_Statement > Make_Area_Statement::statement_maker("make-area");

Make_Area_Statement::Make_Area_Statement
    (int line_number_, const map< string, string >& input_attributes)
    : Statement(line_number_)
{ 
  is_used_ = true;
  
  map< string, string > attributes;
  
  attributes["from"] = "_";
  attributes["into"] = "_";
  attributes["pivot"] = "";
  
  eval_attributes_array(get_name(), attributes, input_attributes);
  
  input = attributes["from"];
  output = attributes["into"];
  pivot = attributes["pivot"];
}

void Make_Area_Statement::forecast()
{
/*  Set_Forecast sf_in(declare_read_set(input));
  declare_read_set(tags);
  Set_Forecast& sf_out(declare_write_set(output));
    
  sf_out.area_count = 1;
  declare_used_time(100 + sf_in.node_count + sf_in.way_count);
  finish_statement_forecast();
    
  display_full();
  display_state();*/
}

pair< uint32, Uint32_Index > Make_Area_Statement::detect_pivot(const Set& pivot)
{
  uint32 pivot_type(0);
  Uint32_Index pivot_id(0u);
  map< Uint32_Index, vector< Node_Skeleton > >::const_iterator
      nit(pivot.nodes.begin());
  while ((pivot_id.val() == 0) && (nit != pivot.nodes.end()))
  {
    if (nit->second.size() > 0)
    {
      pivot_id = nit->second.front().id;
      pivot_type = NODE;
    }
    ++nit;
  }
  map< Uint31_Index, vector< Way_Skeleton > >::const_iterator
      wit(pivot.ways.begin());
  while ((pivot_id.val() == 0) && (wit != pivot.ways.end()))
  {
    if (wit->second.size() > 0)
    {
      pivot_id = wit->second.front().id;
      pivot_type = WAY;
    }
    ++wit;
  }
  map< Uint31_Index, vector< Relation_Skeleton > >::const_iterator
      rit(pivot.relations.begin());
  while ((pivot_id.val() == 0) && (rit != pivot.relations.end()))
  {
    if (rit->second.size() > 0)
    {
      pivot_id = rit->second.front().id;
      pivot_type = RELATION;
    }
    ++rit;
  }
  
  return make_pair< uint32, Uint32_Index >(pivot_type, pivot_id);
}

Node::Id_Type Make_Area_Statement::check_node_parity(const Set& pivot)
{
  set< Node::Id_Type > node_parity_control;
  for (map< Uint31_Index, vector< Way_Skeleton > >::const_iterator
    it(pivot.ways.begin()); it != pivot.ways.end(); ++it)
  {
    for (vector< Way_Skeleton >::const_iterator it2(it->second.begin());
    it2 != it->second.end(); ++it2)
    {
      if (it2->nds.size() < 2)
	continue;
      pair< set< Node::Id_Type >::iterator, bool > npp(node_parity_control.insert
          (it2->nds.front()));
      if (!npp.second)
	node_parity_control.erase(npp.first);
      npp = node_parity_control.insert
          (it2->nds.back());
      if (!npp.second)
	node_parity_control.erase(npp.first);
    }
  }
  if (node_parity_control.size() > 0)
    return *(node_parity_control.begin());
  return 0u;
}

pair< Uint32_Index, Uint32_Index > Make_Area_Statement::create_area_blocks
    (map< Uint31_Index, vector< Area_Block > >& areas,
     uint32 id, const Set& pivot)
{
  vector< Node > nodes;
  for (map< Uint32_Index, vector< Node_Skeleton > >::const_iterator
      it(pivot.nodes.begin()); it != pivot.nodes.end(); ++it)
  {
    for (vector< Node_Skeleton >::const_iterator it2(it->second.begin());
        it2 != it->second.end(); ++it2)
      nodes.push_back(Node(it2->id.val(), it->first.val(), it2->ll_lower));
  }
  sort(nodes.begin(), nodes.end(), Node_Comparator_By_Id());
  
  for (map< Uint31_Index, vector< Way_Skeleton > >::const_iterator
      it(pivot.ways.begin()); it != pivot.ways.end(); ++it)
  {
    for (vector< Way_Skeleton >::const_iterator it2(it->second.begin());
        it2 != it->second.end(); ++it2)
    {
      if (it2->nds.size() < 2)
	continue;
      uint32 cur_idx(0);
      vector< uint64 > cur_polyline;
      for (vector< Node::Id_Type >::const_iterator it3(it2->nds.begin());
          it3 != it2->nds.end(); ++it3)
      {
	Node* node(binary_search_for_id(nodes, *it3));
	if (node == 0)
	  return make_pair(*it3, it2->id);
	if ((node->index & 0xffffff00) != cur_idx)
	{
	  if (cur_idx != 0)
	  {
	    if (cur_polyline.size() > 1)
	      areas[cur_idx].push_back(Area_Block(id, cur_polyline));
	    
	    vector< Aligned_Segment > aligned_segments;
	    Area::calc_aligned_segments
	        (aligned_segments, cur_polyline.back(),
		 ((uint64)node->index<<32) | node->ll_lower_);
	    cur_polyline.clear();
	    for (vector< Aligned_Segment >::const_iterator
	        it(aligned_segments.begin()); it != aligned_segments.end(); ++it)
	    {
	      cur_polyline.push_back((((uint64)it->ll_upper_)<<32)
	        | it->ll_lower_a);
	      cur_polyline.push_back((((uint64)it->ll_upper_)<<32)
	        | it->ll_lower_b);
	      areas[it->ll_upper_].push_back(Area_Block(id, cur_polyline));
	      cur_polyline.clear();
	    }
	  }
	  cur_idx = (node->index & 0xffffff00);
	}
	cur_polyline.push_back(((uint64)node->index<<32) | node->ll_lower_);
      }
      if ((cur_idx != 0) && (cur_polyline.size() > 1))
	areas[cur_idx].push_back(Area_Block(id, cur_polyline));
    }
  }
  return make_pair< uint32, uint32 >(0, 0);
}

uint32 Make_Area_Statement::shifted_lat(uint32 ll_index, uint64 coord)
{
  return ::ilat(ll_index | (coord>>32), coord & 0xffffffff);
}

int32 Make_Area_Statement::lon_(uint32 ll_index, uint64 coord)
{
  return ::ilon(ll_index | (coord>>32), coord & 0xffffffff);
}

void Make_Area_Statement::add_segment_blocks
    (map< Uint31_Index, vector< Area_Block > >& area_blocks, uint32 id)
{
  /* We use that more northern segments always have bigger indices.
    Thus we can collect each block's end points and add them, if they do not
    cancel out, to the next block further northern.*/
  for (map< Uint31_Index, vector< Area_Block > >::const_iterator
    it(area_blocks.begin()); it != area_blocks.end(); ++it)
  {
    set< int32 > lons;
    
    for (vector< Area_Block >::const_iterator it2(it->second.begin());
        it2 != it->second.end(); ++it2)
    {
      if (it2->coors.empty())
	continue;
      
      const uint64& ll_front(it2->coors.front());
      int32 lon_front(lon_(ll_front>>32, ll_front & 0xffffffffull));
      const uint64& ll_back(it2->coors.back());
      int32 lon_back(lon_(ll_back>>32, ll_back & 0xffffffffull));
      if (lons.find(lon_front) == lons.end())
	lons.insert(lon_front);
      else
	lons.erase(lon_front);
      if (lons.find(lon_back) == lons.end())
	lons.insert(lon_back);
      else
	lons.erase(lon_back);
    }
    
    if (lons.empty())
      continue;
    
    uint32 current_idx(it->first.val());
    
    // calc lat
    uint32 lat(shifted_lat(it->first.val(), 0) + 16*65536);
    int32 lon(lon_(it->first.val(), 0));
    uint32 northern_ll_upper(::ll_upper(lat, lon) ^ 0x40000000);
    // insert lons
    vector< Area_Block >& northern_block(area_blocks[northern_ll_upper]);
    for (set< int32 >::const_iterator it2(lons.begin()); it2 != lons.end(); ++it2)
    {
      int32 from(*it2);
      ++it2;
      int32 to(*it2);
      vector< uint64 > coors;
      coors.push_back
          ((((uint64)(::ll_upper(lat, from) ^ 0x40000000))<<32) | ::ll_lower(lat, from));
      coors.push_back
          ((((uint64)(::ll_upper(lat, to) ^ 0x40000000))<<32) | ::ll_lower(lat, to));
      Area_Block new_block(id, coors);
      northern_block.push_back(new_block);
    }
    
    it = area_blocks.find(Uint31_Index(current_idx));
  }
}

void Make_Area_Statement::execute(Resource_Manager& rman)
{
  map< Uint32_Index, vector< Node_Skeleton > >& nodes
      (rman.sets()[output].nodes);
  map< Uint31_Index, vector< Way_Skeleton > >& ways
      (rman.sets()[output].ways);
  map< Uint31_Index, vector< Relation_Skeleton > >& relations
      (rman.sets()[output].relations);
  map< Uint31_Index, vector< Area_Skeleton > >& areas
      (rman.sets()[output].areas);
  
  // detect pivot element
  map< string, Set >::const_iterator mit(rman.sets().find(pivot));
  if (mit == rman.sets().end())
  {
    nodes.clear();
    ways.clear();
    relations.clear();
    areas.clear();
    
    return;
  }
  pair< uint32, Uint32_Index > pivot_pair(detect_pivot(mit->second));
  int pivot_type(pivot_pair.first);
  uint32 pivot_id(pivot_pair.second.val());
  
  if (pivot_type == 0)
  {
    nodes.clear();
    ways.clear();
    relations.clear();
    areas.clear();
    
    return;
  }
  
  //formulate range query to query tags of the pivot
  set< Uint31_Index > coarse_indices;
  set< pair< Tag_Index_Local, Tag_Index_Local > > range_set;
  if (pivot_type == NODE)
    coarse_indices.insert(mit->second.nodes.begin()->first.val() & 0xffffff00);
  else if (pivot_type == WAY)
    coarse_indices.insert(mit->second.ways.begin()->first.val() & 0xffffff00);
  else if (pivot_type == RELATION)
    coarse_indices.insert(mit->second.relations.begin()->first.val() & 0xffffff00);
  
  formulate_range_query(range_set, coarse_indices);
  
  // iterate over the result
  vector< pair< string, string > > new_tags;
  File_Properties* file_prop = 0;
  if (pivot_type == NODE)
    file_prop = osm_base_settings().NODE_TAGS_LOCAL;
  else if (pivot_type == WAY)
    file_prop = osm_base_settings().WAY_TAGS_LOCAL;
  else if (pivot_type == RELATION)
    file_prop = osm_base_settings().RELATION_TAGS_LOCAL;
  Block_Backend< Tag_Index_Local, Uint32_Index > items_db
      (rman.get_transaction()->data_index(file_prop));
  Block_Backend< Tag_Index_Local, Uint32_Index >::Range_Iterator
      tag_it(items_db.range_begin
        (Default_Range_Iterator< Tag_Index_Local >(range_set.begin()),
         Default_Range_Iterator< Tag_Index_Local >(range_set.end())));
  for (; !(tag_it == items_db.range_end()); ++tag_it)
  {
    if (tag_it.object().val() == pivot_id)
      new_tags.push_back(make_pair(tag_it.index().key, tag_it.index().value));
  }
  
  if (pivot_type == WAY)
    pivot_id += 2400000000u;
  else if (pivot_type == RELATION)
    pivot_id += 3600000000u;
  
  mit = rman.sets().find(input);
  if (mit == rman.sets().end())
  {
    nodes.clear();
    ways.clear();
    relations.clear();
    areas.clear();
    
    return;
  }
  
  // check node parity
  Node::Id_Type odd_id(check_node_parity(mit->second));
  if (!(odd_id == Node::Id_Type(0u)))
  {
    ostringstream temp;
    temp<<"make-area: Node "<<odd_id.val()
        <<" is contained in an odd number of ways.\n";
    runtime_remark(temp.str());
  }
  
  // create area blocks
  map< Uint31_Index, vector< Area_Block > > area_blocks;
  pair< Node::Id_Type, Way::Id_Type > odd_pair
    (create_area_blocks(area_blocks, pivot_id, mit->second));
  if (!(odd_pair.first == Node::Id_Type(0u)))
  {
    ostringstream temp;
    temp<<"make-area: Node "<<odd_pair.first.val()
        <<" referred by way "<<odd_pair.second.val()
        <<" is not contained in set \""<<input<<"\".\n";
    runtime_remark(temp.str());
  }
  
  if (!(odd_id == Node::Id_Type(0u)) || !(odd_pair.first == Node::Id_Type(0u)))
  {
    nodes.clear();
    ways.clear();
    relations.clear();
    areas.clear();
    
    return;
  }

  add_segment_blocks(area_blocks, pivot_id);
  
  set< uint32 > used_indices;
  for (map< Uint31_Index, vector< Area_Block > >::const_iterator
      it(area_blocks.begin()); it != area_blocks.end(); ++it)
    used_indices.insert(it->first.val());
  
  Area_Location new_location(pivot_id, used_indices);
  new_location.tags = new_tags;
  Uint31_Index new_index(new_location.calc_index());
  
  nodes.clear();
  ways.clear();
  relations.clear();
  areas.clear();

  if (new_index.val() == 0)
    return;
  
  if (rman.area_updater())
  {
    Area_Updater* area_updater(rman.area_updater());
    area_updater->set_area(new_index, new_location);
    area_updater->add_blocks(area_blocks);
    area_updater->commit();
  }
  
  areas[new_index].push_back(Area_Skeleton(new_location));
  
  rman.health_check(*this);
}
