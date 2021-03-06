// PropertyList.cpp:  ActionScript property lists, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "PropertyList.h"
#include "Property.h" 
#include "as_environment.h"
#include "log.h"
#include "as_function.h"
#include "as_value.h" // for enumerateValues
#include "VM.h" // For string_table
#include "string_table.h"

#include <utility> // for std::make_pair
#include <boost/bind.hpp> 

// Define the following to enable printing address of each property added
//#define DEBUG_PROPERTY_ALLOC

// Define this to get verbosity of properties insertion and flags setting
//#define GNASH_DEBUG_PROPERTY 1

namespace gnash {

namespace {

inline
PropertyList::container::iterator
iterator_find(const PropertyList::container &p, const ObjectURI& uri)
{
    return p.find(uri);
}

}

typedef PropertyList::container::index<PropertyList::OrderTag>::type::iterator
    order_iterator;

order_iterator
iterator_find(PropertyList::container &p, int order)
{
	return p.get<1>().find(order);
}

const Property*
PropertyList::getPropertyByOrder(int order)
{
    order_iterator i = iterator_find(_props, order);
	if (i == _props.get<1>().end()) return 0;

	return &(*i);
}

const Property*
PropertyList::getOrderAfter(int order)
{
    order_iterator i = iterator_find(_props, order);

	if (i == _props.get<1>().end()) return 0;

	do {
		++i;
		if (i == _props.get<1>().end()) return 0;
	} while (i->getFlags().get_dont_enum());

	return &(*i);
}

bool
PropertyList::reserveSlot(const ObjectURI& uri, boost::uint16_t slotId)
{
    order_iterator found = iterator_find(_props, slotId + 1);
	if (found != _props.get<1>().end()) return false;

	Property a(uri, as_value());
	a.setOrder(slotId + 1);
	_props.insert(a);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
	log_debug("Slot for AS property %s inserted with flags %s", l(uri)
            a.getFlags());
#endif

	return true;
}

bool
PropertyList::setValue(const ObjectURI& uri, const as_value& val,
        const PropFlags& flagsIfMissing)
{
	container::iterator found = iterator_find(_props, uri);
	
	if (found == _props.end())
	{
		// create a new member
		Property a(uri, val, flagsIfMissing);
		// Non slot properties are negative ordering in insertion order
		a.setOrder(- ++_defaultOrder - 1);
		_props.insert(a);
#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
		log_debug("Simple AS property %s inserted with flags %s",
			l(uri), a.getFlags());
#endif
		return true;
	}

	const Property& prop = *found;
	if (prop.isReadOnly() && ! prop.isDestructive())
	{
        ObjectURI::Logger l(getStringTable(_owner));
		log_error(_("Property %s is read-only %s, not setting it to %s"), 
			l(uri), prop.getFlags(), val);
		return false;
	}

	// Property is const because the container uses its members
	// for indexing. We don't use value (only name and namespace)
	// so this const_cast is safe
	const_cast<Property&>(prop).setValue(_owner, val);

	return true;
}

bool
PropertyList::setFlags(const ObjectURI& uri, int setFlags, int clearFlags)
{
	container::iterator found = iterator_find(_props, uri);
	if ( found == _props.end() ) return false;

	PropFlags oldFlags = found->getFlags();

	PropFlags& f = const_cast<PropFlags&>(found->getFlags());
	return f.set_flags(setFlags, clearFlags);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
	log_debug("Flags of property %s changed from %s to  %s",
		l(uri), oldFlags, found->getFlags());
#endif
}

void
PropertyList::setFlagsAll(int setFlags, int clearFlags)
{
    PropertyList::container::iterator it;
    for (it=_props.begin(); it != _props.end(); ++it) {
		PropFlags& f = const_cast<PropFlags&>(it->getFlags());
		f.set_flags(setFlags, clearFlags);
    }
}

Property*
PropertyList::getProperty(const ObjectURI& uri) const
{
	container::iterator found = iterator_find(_props, uri);
	if (found == _props.end()) return 0;
	return const_cast<Property*>(&(*found));
}

std::pair<bool,bool>
PropertyList::delProperty(const ObjectURI& uri)
{
	//GNASH_REPORT_FUNCTION;
	container::iterator found = iterator_find(_props, uri);
	if (found == _props.end()) {
		return std::make_pair(false, false);
	}

	// check if member is protected from deletion
	if (found->getFlags().get_dont_delete()) {
		return std::make_pair(true, false);
	}

	_props.erase(found);
	return std::make_pair(true, true);
}


/// This does not reflect the normal enumeration order. It is sorted
/// lexicographically by property.
void
PropertyList::dump(std::map<std::string, as_value>& to) 
{
    ObjectURI::Logger l(getStringTable(_owner));

	for (container::const_iterator i=_props.begin(), ie=_props.end();
            i != ie; ++i)
	{
		to.insert(std::make_pair(l(i->uri()), i->getValue(_owner)));
	}
}

void
PropertyList::enumerateKeys(as_environment& env, PropertyTracker& donelist)
    const
{
	string_table& st = getStringTable(_owner);

    // We should enumerate in order of creation, not lexicographically.
    typedef container::nth_index<1>::type ContainerByOrder;

	for (ContainerByOrder::const_reverse_iterator i=_props.get<1>().rbegin(),
            ie=_props.get<1>().rend(); i != ie; ++i) {

		if (i->getFlags().get_dont_enum()) continue;

        const ObjectURI& uri = i->uri();

		if (donelist.insert(uri).second) {

            const std::string& qname = getNamespace(uri) ?
                st.value(getName(uri)) + "." + st.value(getNamespace(uri)) :
                st.value(getName(uri));

			env.push(qname);
		}
	}
}

void
PropertyList::dump()
{
    ObjectURI::Logger l(getStringTable(_owner));
	for (container::const_iterator it=_props.begin(), itEnd=_props.end();
            it != itEnd; ++it) {
		log_debug("  %s: %s", l(it->uri()), it->getValue(_owner));
	}
}

bool
PropertyList::addGetterSetter(const ObjectURI& uri, as_function& getter,
	as_function* setter, const as_value& cacheVal,
	const PropFlags& flagsIfMissing)
{
	Property a(uri, &getter, setter, flagsIfMissing);
	a.setOrder(- ++_defaultOrder - 1);

	container::iterator found = iterator_find(_props, uri);
	if (found != _props.end())
	{
		// copy flags from previous member (even if it's a normal member ?)
		PropFlags& f = a.getFlags();
		f = found->getFlags();
		a.setCache(found->getCache());
		_props.replace(found, a);

#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
		log_debug("AS GetterSetter %s replaced copying flags %s", l(uri),
                a.getFlags());
#endif

	}
	else
	{
		a.setCache(cacheVal);
		_props.insert(a);
#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
		log_debug("AS GetterSetter %s inserted with flags %s", l(uri),
                a.getFlags());
#endif
	}

	return true;
}

bool
PropertyList::addGetterSetter(const ObjectURI& uri, as_c_function_ptr getter,
	as_c_function_ptr setter, const PropFlags& flagsIfMissing)
{
	Property a(uri, getter, setter, flagsIfMissing);
	a.setOrder(- ++_defaultOrder - 1);

	container::iterator found = iterator_find(_props, uri);
	if (found != _props.end())
	{
		// copy flags from previous member (even if it's a normal member ?)
		PropFlags& f = a.getFlags();
		f = found->getFlags();
		_props.replace(found, a);

#ifdef GNASH_DEBUG_PROPERTY
        ObjectURI::Logger l(getStringTable(_owner));
		log_debug("Native GetterSetter %s replaced copying flags %s", l(uri),
                a.getFlags());
#endif

	}
	else
	{
		_props.insert(a);
#ifdef GNASH_DEBUG_PROPERTY
		string_table& st = getStringTable(_owner);
		log_debug("Native GetterSetter %s in namespace %s inserted with "
                "flags %s", st.value(key), st.value(nsId), a.getFlags());
#endif
	}

	return true;
}

bool
PropertyList::addDestructiveGetter(const ObjectURI& uri, as_function& getter, 
	const PropFlags& flagsIfMissing)
{
	container::iterator found = iterator_find(_props, uri);
	if (found != _props.end())
	{
        ObjectURI::Logger l(getStringTable(_owner));
		log_error("Property %s already exists, can't addDestructiveGetter",
                l(uri));
		return false; // Already exists.
	}

	// destructive getter don't need a setter
	Property a(uri, &getter, (as_function*)0, flagsIfMissing, true);
	a.setOrder(- ++_defaultOrder - 1);
	_props.insert(a);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
	log_debug("Destructive AS property %s inserted with flags %s",
            l(uri), a.getFlags());
#endif

	return true;
}

bool
PropertyList::addDestructiveGetter(const ObjectURI& uri,
	as_c_function_ptr getter, const PropFlags& flagsIfMissing)
{
	container::iterator found = iterator_find(_props, uri);
	if (found != _props.end()) return false; 

	// destructive getter don't need a setter
	Property a(uri, getter, (as_c_function_ptr)0, flagsIfMissing, true);
	a.setOrder(- ++_defaultOrder - 1);
	_props.insert(a);

#ifdef GNASH_DEBUG_PROPERTY
    ObjectURI::Logger l(getStringTable(_owner));
	log_debug("Destructive native property %s with flags %s", l(uri),
            a.getFlags());
#endif
	return true;
}

void
PropertyList::clear()
{
	_props.clear();
}

void
PropertyList::setReachable() const
{
    std::for_each(_props.begin(), _props.end(),
            boost::mem_fn(&Property::setReachable));
}

} // namespace gnash

