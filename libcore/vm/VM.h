// VM.h: the Virtual Machine class, for Gnash
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

#ifndef GNASH_VM_H
#define GNASH_VM_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "GC.h"
#include "string_table.h"
#include "SafeStack.h"
#include "CallStack.h"
#include "smart_ptr.h"

#include <map>
#include <vector>
#include <memory> 
#include <locale>
#include <boost/cstdint.hpp> 
#include <boost/random.hpp>
#include <boost/noncopyable.hpp>
#include <boost/intrusive_ptr.hpp>

// Forward declarations
namespace gnash {
    namespace abc {
        class Machine;
    }
	class Global_as;
	class VM;
	class fn_call;
	class movie_root;
	class NativeFunction;
    class SharedObjectLibrary;
	class as_value;
	class as_object;
	class VirtualClock;
}

namespace gnash {

/// A GC root used to mark all reachable collectable pointers
class VmGcRoot : public GcRoot 
{
public:
	VmGcRoot(VM& vm) : _vm(vm) {}
	virtual void markReachableResources() const;

private:
    VM& _vm;
};

/// The virtual machine
//
/// This is the machine that executes all actions in the 
/// main movie, including the actions in movies loaded by
/// it.
///
/// Note that the target SWF version of the "main" movie
/// (the first movie loaded, the 'root' movie) drives
/// the operation, as depending on that version the Virtual
/// Machine acts differently, for backward compatibility.
/// 
/// The VM is initialized once for each "stage" (main movie).
/// Gnash currently only supports a *single* VM as it uses
/// gloabls a lot. Definition of this class is aimed at
/// grouping the globals into a specific VM instance that
/// we might pass around in the future to allow multiple
/// movies runs.
/// For the moment, it will be a singleton, providing one-time
/// initialization.
///
class DSOEXPORT VM : boost::noncopyable
{

public:

	typedef as_value (*as_c_function_ptr)(const fn_call& fn);

    enum AVMVersion
    {
        AVM1,
        AVM2
    };
	
    /// \brief
	/// Initialize the virtual machine singleton with the given
	/// movie definition and return a reference to it.
	//
	/// The given movie will be only used to fetch SWF version from.
	///
	/// Don't call this function twice, and make sure you have
	/// called this *before* you call VM::get()
	///
	/// @param movie
	///	The definition for the root movie, only
	///	used to fetch SWF version from.
	///	TODO: take SWF version directly ?
	///
	/// @param clock
	///	Virtual clock used as system time.
	///
	static VM& init(int version, movie_root& root, VirtualClock& clock);

	SafeStack<as_value>& getStack() {
		return _stack;
	}

	CallStack& getCallStack() {
		return _callStack;
	}

    /// Get the VM clock
    //
    /// NOTE: this clock should drive all internal operations
    /// but maybe accessing it trough VM isn't the best idea.
    /// TODO: consider making this accessible trough RunResources
    /// instead.
    ///
    VirtualClock& getClock() {
        return _clock;
    }

	/// Return true if the singleton VM has been initialized
	static bool isInitialized();

    /// Resets any VM members that must be cleared before the GC cleans up
    //
    /// At present, this is:
    /// - SharedObjectLibrary
    ///
    /// Ideally, this would be left to the VM's dtor, but we have no control
    /// over destruction order at present.
    /// It is assumed that this is the last VM function called before the
    /// dtor.
    void clear();

	/// Get the singleton instance of the virtual machine
	//
	/// Make sure you called VM::init() before trying to
	/// get the singleton (an assertion would fail otherwise)
	///
	/// Use isInitialized() if you're unsure.
	///
	static VM& get();

	/// Get SWF version context for the currently running actions.
	//
	/// This information will drive operations of the virtual machine
	///
	int getSWFVersion() const;

	/// Set SWF version of the currently executing code
	void setSWFVersion(int v);

    /// Get the version of the currently executing VM
    //
    /// Note: this value changes according to the current execution context;
    /// do not use it to determine what VM version a particular SWF uses.
    AVMVersion getAVMVersion() const {
        return _avmVersion;
    }

    /// Set the version of the currently executing VM.
    void setAVMVersion(AVMVersion v) {
        _avmVersion = v;
    }

	/// Get the number of milliseconds since VM was started
	unsigned long int getTime() const;

	/// Get a reference to the string table used by the VM.
	string_table& getStringTable() const { return _stringTable; }

#ifdef ENABLE_AVM2
	/// Get a pointer to the machine, if it exists.
    abc::Machine* getMachine() const { return _machine; }
#endif

	/// Get version of the player, in a compatible representation
	//
	/// This information will be used for the System.capabilities.version
	/// and $version ActionScript variables.
	///
	const std::string& getPlayerVersion() const;
	
	/// Get current OS name. This is used for System.capabilites.os. If
	/// defined in gnashrc, that takes precedence. For Linux, the string
	/// includes the kernel version (unname -sr). Only works for systems
	/// with sys/utsname.h (POSIX 4.4).
	const std::string getOSName();
	
	/// Return the current language of the system. This is used for
	/// System.capabilities.language. Only works for systems with 
	/// a language environment variable.
	const std::string getSystemLanguage();
	
	// The boost Random Number Generator to use.
	//
	// http://www.boost.org/libs/random/random-generators.html
	//
	// TODO: boost/nondet_random.hpp provides access to a random device,
	// which can be used in preference to a pseudo-RNG. It is only
	// presently available on some platforms.
	// http://www.boost.org/libs/random/nondet_random.html
	//
	// Generators have different limits on the size of the seed. Please
	// check if replacing the generator.
	//
	// The mt11213b provides a pseudo-random number cycle
	// of length 2^11213-1 and requires approx 352*sizeof(uint32_t) memory
	// once initialized. It is more than adequate for most purposes.
	typedef boost::mt11213b RNG;	

	// Get a pointer to the random number generator for
	// use by Math.random() and random().
	//
	// The seed is the system time in milliseconds at the first call
	// to a random function. This allows a potentially variable amount
	// of time to elapse between starting gnash and initialization of
	// the generator, so decreasing predictability.
	RNG& randomNumberGenerator() const;

	/// Get a pointer to this VM's Root movie (stage)
	movie_root& getRoot() const;

    /// Return the Shared Object Library
    //
    /// The Shared Object Library is assumed to exist until VM::clear()
    /// is called.
    SharedObjectLibrary& getSharedObjectLibrary() const {
        assert(_shLib.get());
        return *_shLib;
    }

	/// Get a pointer to this VM's _global Object
	Global_as* getGlobal() const;

	/// Mark all reachable resources (for GC)
	//
	/// - root movie / stage (_rootMovie)
	/// - Global object (_global)
	/// - registered static GcResources (_statics)
	/// - Class Hierarchy object
	///
	///
	void markReachableResources() const;

	void registerNative(as_c_function_ptr fun, unsigned int x, unsigned int y);

	/// Return a native function or null
	NativeFunction* getNative(unsigned int x, unsigned int y) const;

#ifdef GNASH_USE_GC
	void addStatic(GcResource* res)
	{
		_statics.push_back(res);
	}
#else  // ndef GNASH_USE_GC
	// placeholder to avoid adding lots of
	// compile-time switches in callers
	void addStatic(as_object*) {}
#endif

private:

	friend class VmGcRoot;

	/// Use VM::get() to access the singleton
	//
	/// Initializes the GC singleton
	///
	VM(int version, movie_root& root, VirtualClock& clock);

	/// Should deinitialize the GC singleton
	/// If it doesn't is just because it corrupts memory :)
	~VM();

	// We use an auto_ptr here to allow constructing
	// the singleton when the init() function is called.
	friend class std::auto_ptr<VM>;
	static std::auto_ptr<VM> _singleton;

	/// Stage associated with this VM
	movie_root& _rootMovie;

	/// The _global ActionScript object for AVM1
	Global_as* _global;

	/// Target SWF version
	int _swfversion;

	/// Set the _global Object for actions run by Virtual Machine
	//
	/// Will be called by the init() function
	/// 
	void setGlobal(Global_as*);

#ifdef GNASH_USE_GC
	/// A vector of static GcResources (typically used for built-in
    /// class constructors)
	//
	/// The resources in this list will always be marked as reachable
	///
	typedef std::vector< boost::intrusive_ptr<GcResource> > ResVect;
	ResVect _statics;
#endif

	typedef std::map<unsigned int, as_c_function_ptr> FuncMap;
	typedef std::map<unsigned int, FuncMap> AsNativeTable;
	AsNativeTable _asNativeTable;

	/// Mutable since it should not affect how the VM runs.
	mutable string_table _stringTable;

#ifdef ENABLE_AVM2
	/// A running execution thread.
    abc::Machine *_machine;
#endif

	VirtualClock& _clock;

	SafeStack<as_value>	_stack;

	CallStack _callStack;

	/// Library of SharedObjects. Owned by the VM.
    std::auto_ptr<SharedObjectLibrary> _shLib;

    /// The currently executing machine
    //
    /// This is a hack like switching the SWF version, but without this
    /// there is no way for AS functions to know which version of the
    /// virtual machine called them. This is necessary e.g. for initializing
    /// the correct object prototypes etc.
    AVMVersion _avmVersion;

};

/////////////////////////////////////////////////////////////////////////////
///
/// VM ops on as_value.
///
/// These are currently used in:
/// 1. VM (AVM1)
/// 2. Machine (AVM2)
/// 3. Gnash's C++ implementation of ActionScript.
///
/// Usage 3 replicates VM behaviour, but does not use the VM's stack or other
/// resources. This can lead to different behaviour, for instance when there
/// are limits on the stack size. Such cases are probably rare.
///
/////////////////////////////////////////////////////////////////////////////

/// Carry out ActionNewAdd
//
/// @param op1      The as_value to add to.
/// @param op2      The as_value to add.
/// @param vm       The VM executing the operation.
//
/// TODO:           Consider whether it would be better to pass something
///                 other than the VM. But it is a VM operation, so it
///                 is logically sound.
void newAdd(as_value& op1, const as_value& op2, VM& vm);

/// Carry out ActionSubtract
//
/// @param op1      The as_value to subtract from.
/// @param op2      The as_value to subtract.
/// @param vm       The VM executing the operation.
void subtract(as_value& op1, const as_value& op2, VM& vm);

/// Carry out ActionSubtract
//
/// @param op1      The first comparand.
/// @param op2      The second comparand.
/// @param vm       The VM executing the operation.
as_value newLessThan(const as_value& op1, const as_value& op2, VM& vm);

/// Return true if the VM is executing AS3 (ABC bytecode).
inline bool
isAS3(VM& vm)
{
    return vm.getAVMVersion() == VM::AVM2;
}

} // namespace gnash

#endif // GNASH_VM_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
