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

#ifndef DE__OSM3S___OVERPASS_API__STATEMENTS__AREA_QUERY_H
#define DE__OSM3S___OVERPASS_API__STATEMENTS__AREA_QUERY_H

#include <map>
#include <string>
#include <vector>
#include "statement.h"

using namespace std;

class Area_Query_Statement : public Statement
{
  public:
    Area_Query_Statement(int line_number_, const map< string, string >& attributes);
    virtual string get_name() const { return "area-query"; }
    virtual string get_result_name() const { return output; }
    virtual void forecast();
    virtual void execute(Resource_Manager& rman);
    virtual ~Area_Query_Statement();
    
    static Generic_Statement_Maker< Area_Query_Statement > statement_maker;
    
    virtual Query_Constraint* get_query_constraint();
    
    void get_ranges
      (set< pair< Uint32_Index, Uint32_Index > >& nodes_req,
       set< Uint31_Index >& area_block_req,
       Resource_Manager& rman);

    void get_ranges
      (const map< Uint31_Index, vector< Area_Skeleton > >& input_areas,
       set< pair< Uint32_Index, Uint32_Index > >& nodes_req,
       set< Uint31_Index >& area_block_req,
       Resource_Manager& rman);

    void collect_nodes
      (const set< pair< Uint32_Index, Uint32_Index > >& nodes_req,
       const set< Uint31_Index >& req,
       vector< Node::Id_Type >* ids,
       map< Uint32_Index, vector< Node_Skeleton > >& nodes,
       Resource_Manager& rman);
       
    void collect_nodes
      (map< Uint32_Index, vector< Node_Skeleton > >& nodes,
       const set< Uint31_Index >& req,
       Resource_Manager& rman);

    bool areas_from_input() const { return area_id.empty(); }
    string get_input() const { return input; }
    
    static bool is_used() { return is_used_; }
  
  private:
    string input;
    string output;
    vector< Area_Skeleton::Id_Type > area_id;    
    static bool is_used_;
    vector< Query_Constraint* > constraints;
};

#endif
