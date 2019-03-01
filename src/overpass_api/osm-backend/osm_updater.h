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

#ifndef DE__OSM3S___OVERPASS_API__OSM_BACKEND__OSM_UPDATER_H
#define DE__OSM3S___OVERPASS_API__OSM_BACKEND__OSM_UPDATER_H

#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "node_updater.h"
#include "relation_updater.h"
#include "way_updater.h"

using namespace std;

class Dispatcher_Client;

class Osm_Updater
{
  public:
    Osm_Updater(Osm_Backend_Callback* callback_, const string& data_version,
		bool meta, bool produce_augmented_diffs);
    Osm_Updater(Osm_Backend_Callback* callback_, string db_dir, const string& data_version,
		bool meta, bool produce_augmented_diffs);
    ~Osm_Updater();

    void finish_updater();
    void parse_file_completely(FILE* in);
    
  private:
    Nonsynced_Transaction* transaction;
    Dispatcher_Client* dispatcher_client;
    Node_Updater* node_updater_;
    Update_Node_Logger* update_node_logger_;
    Way_Updater* way_updater_;
    Update_Way_Logger* update_way_logger_;
    Relation_Updater* relation_updater_;
    Update_Relation_Logger* update_relation_logger_;
    string db_dir_;
    bool meta;

    void flush();
};

void parse_nodes_only(FILE* in);
void parse_ways_only(FILE* in);
void parse_relations_only(FILE* in);

#endif
