/******************************************************************************
 * Memember functions for data structure ModuleManager
 ******************************************************************************/
#include <string>
#include <algorithm>
#include "vtr_assert.h"

#include "circuit_library.h"
#include "module_manager.h"

/******************************************************************************
 * Public Constructors
 ******************************************************************************/

/******************************************************************************
 * Public Accessors
 ******************************************************************************/
/* Find the name of a module */
std::string ModuleManager::module_name(const ModuleId& module_id) const {
  /* Validate the module_id */
  VTR_ASSERT(valid_module_id(module_id));
  return names_[module_id];
}

/* Get the string of a module port type */
std::string ModuleManager::module_port_type_str(const enum e_module_port_type& port_type) const {
  std::array<const char*, NUM_MODULE_PORT_TYPES> MODULE_PORT_TYPE_STRING = {{"GLOBAL PORTS", "INOUT PORTS", "INPUT PORTS", "OUTPUT PORTS", "CLOCK PORTS"}};
  return MODULE_PORT_TYPE_STRING[port_type];
}

/* Find a list of ports of a module by a given types */
std::vector<BasicPort> ModuleManager::module_ports_by_type(const ModuleId& module_id, const enum e_module_port_type& port_type) const {
  /* Validate the module_id */
  VTR_ASSERT(valid_module_id(module_id));

  std::vector<BasicPort> ports;
  for (const auto& port : port_ids_[module_id]) {
    /* Skip unmatched ports */
    if (port_type != port_types_[module_id][port]) {
      continue;
    }
    ports.push_back(ports_[module_id][port]);
  }

  return ports;
}

/******************************************************************************
 * Public Mutators
 ******************************************************************************/
/* Add a module */
ModuleId ModuleManager::add_module(const std::string& name) {
  /* Find if the name has been used. If used, return an invalid Id and report error! */
  std::map<std::string, ModuleId>::iterator it = name_id_map_.find(name);
  if (it != name_id_map_.end()) {
    return ModuleId::INVALID();
  }

  /* Create an new id */
  ModuleId module = ModuleId(ids_.size());
  ids_.push_back(module);

  /* Allocate other attributes */
  names_.push_back(name);
  parents_.emplace_back();
  children_.emplace_back();

  port_ids_.emplace_back();
  ports_.emplace_back();
  port_types_.emplace_back();

  /* Register in the name-to-id map */
  name_id_map_[name] = module;

  /* Build port lookup */
  port_lookup_.emplace_back();
  port_lookup_[module].resize(NUM_MODULE_PORT_TYPES);

  /* Return the new id */
  return module;
}

/* Add a port to a module */
ModulePortId ModuleManager::add_port(const ModuleId& module, 
                                     const BasicPort& port_info, const enum e_module_port_type& port_type) {
  /* Validate the id of module */
  VTR_ASSERT( valid_module_id(module) );

  /* Add port and fill port attributes */
  ModulePortId port = ModulePortId(port_ids_[module].size());
  port_ids_[module].push_back(port);
  ports_[module].push_back(port_info);
  port_types_[module].push_back(port_type);

  /* Update fast look-up for port */
  port_lookup_[module][port_type].push_back(port);

  return port;
}

/* Add a child module to a parent module */
void ModuleManager::add_child_module(const ModuleId& parent_module, const ModuleId& child_module) {
  /* Validate the id of both parent and child modules */
  VTR_ASSERT ( valid_module_id(parent_module) );
  VTR_ASSERT ( valid_module_id(child_module) );

  /* Try to find if the parent module is already in the list */
  std::vector<ModuleId>::iterator parent_it = std::find(parents_[child_module].begin(), parents_[child_module].end(), parent_module);
  if (parent_it == parents_[child_module].end()) {
    /* Update the parent module of child module */
    parents_[child_module].push_back(parent_module);
  }

  std::vector<ModuleId>::iterator child_it = std::find(children_[child_module].begin(), children_[child_module].end(), child_module);
  if (child_it == children_[parent_module].end()) {
    /* Update the child module of parent module */
    children_[parent_module].push_back(child_module);
  }
}

/******************************************************************************
 * Private validators/invalidators
 ******************************************************************************/
bool ModuleManager::valid_module_id(const ModuleId& module) const {
  return ( size_t(module) < ids_.size() ) && ( module == ids_[module] ); 
}

bool ModuleManager::valid_module_port_id(const ModuleId& module, const ModulePortId& port) const {
  if (false == valid_module_id(module)) {
    return false;
  }
  return ( size_t(port) < port_ids_[module].size() ) && ( port == port_ids_[module][port] ); 
}

void ModuleManager::invalidate_name2id_map() {
  name_id_map_.clear();
}

void ModuleManager::invalidate_port_lookup() {
  port_lookup_.clear();
}