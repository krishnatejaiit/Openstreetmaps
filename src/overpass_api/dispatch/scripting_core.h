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

#ifndef DE__OSM3S___OVERPASS_API__DISPATCH__SCRIPTING_CORE_H
#define DE__OSM3S___OVERPASS_API__DISPATCH__SCRIPTING_CORE_H

#include "resource_manager.h"
#include "../frontend/console_output.h"
#include "../frontend/user_interface.h"
#include "../statements/statement.h"
#include "../../expat/expat_justparse_interface.h"
#include "../../template_db/dispatcher.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Exit_Error {};

class Dispatcher_Stub : public Watchdog_Callback
{
  public:
    // Opens the connection to the database, sets db_dir accordingly
    // and registers the process. error_output_ must remain valid over the
    // entire lifetime of this object.
    Dispatcher_Stub(string db_dir_, Error_Output* error_output_, string xml_raw,
		    int& area_level,
		    uint32 max_allowed_time, uint64 max_allowed_space);
    
    // Called once per minute from the resource manager
    virtual void ping() const;

    ~Dispatcher_Stub();
    
    string get_timestamp() { return timestamp; }
    string get_area_timestamp() { return area_timestamp; }
    Resource_Manager& resource_manager() { return *rman; }
    
  private:
    string db_dir, timestamp, area_timestamp;
    
    Error_Output* error_output;
    Dispatcher_Client* dispatcher_client;
    Dispatcher_Client* area_dispatcher_client;
    Nonsynced_Transaction* transaction;
    Nonsynced_Transaction* area_transaction;
    Resource_Manager* rman;
    bool meta;
};

typedef enum
    { parser_execute, parser_dump_xml, parser_dump_pretty_map_ql, parser_dump_compact_map_ql }
    Debug_Level;
    
bool parse_and_validate
    (Statement::Factory& stmt_factory,
     const string& xml_raw, Error_Output* error_output, Debug_Level debug_type);
    
vector< Statement* >* get_statement_stack();

bool get_uses_meta_data();

#endif
