// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cstring>
#include <cstdlib>
#include "npapi.h"
#include "external.h"
#include "npruntime.h"
#include "plugin.h" 
#include "pluginScriptObject.h"

#if 0

// http://www.adobe.com/support/flash/publishexport/scriptingwithflash/scriptingwithflash_04.html
// Tell Target Methods. Use TGetProperty, TGetPropertyAsNumber, TSetProperty
TCallLabel
TCurrentFrame
TCurrentLabel
TGetProperty
TGetPropertyAsNumber
TGotoFrame
TGotoLabel
TPlay
TSetProperty
TStopPlay

// Target Properties
X_POS
Y_POS
X_SCALE
Y_SCALE
CURRENT_FRAME                   // read-only
TOTAL_FRAMES                    // read-only
ALPHA
VISIBLE
WIDTH                           // read-only
HEIGHT                          // read-only
ROTATE
TARGET                          // read-only
FRAMES_LOADED                   // read-only
NAME
DROP_TARGET                     // read-only
URL                             // read-only
HIGH_QUALITY
FOCUS_RECT
SOUND_BUF_TIME

// Standard Events
OnProgress
OnReadyStateChange
FSCommand
#endif

namespace gnash {

// Callbacks for the default methods

// As these callbacks use a generalized typedef for the signature, often some
// of the parameters can be ignored. These are commented out to elimnate the
// volumnes of bogus warnings about not using them in the method.

// SetVariable( Name, Value )
//
// Sends something like this:
// <invoke name="SetVariable" returntype="xml">
//        <arguments>
//              <string>var1</string>
//              <string>value1</string>
//        </arguments>
// </invoke>
//
// Receives:
//      nothing
bool
SetVariableCallback (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    std::string varname;
    if (argCount == 2) {
        varname = NPStringToString(NPVARIANT_TO_STRING(args[0]));
        const NPVariant& value = args[1];
        // log_debug("Setting Variable \"%s\"", varname);
        gpso->SetVariable(varname, value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// GetVariable( Name )
//
// Sends something like this:
// <invoke name="GetVariable" returntype="xml">
//      <arguments>
//              <string>var1</string>
//      </arguments>
// </invoke>
//
// Receives something like this:
//      <number>123</number>
bool
GetVariableCallback (NPObject *npobj, NPIdentifier /* name */,
                     const NPVariant *args,
                     uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    std::string varname;
    // This method only takes one argument
    if (argCount == 1) {
        varname = NPStringToString(NPVARIANT_TO_STRING(args[0]));

        GnashNPVariant value = gpso->GetVariable(varname);
        value.copy(*result);

        return true;
    }
    
    NULL_TO_NPVARIANT(*result);
    return false;
}

// GotoFrame( frameNumber )
//
// Sends something like this:
// <invoke name="GotoFrame" returntype="xml">
//      <arguments>
//              <number>123</number>
//      </arguments>
// </invoke>
//
//    Receives:
// 	nothing
bool
GotoFrame (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    ExternalInterface ei;

    std::string varname;
    if (argCount == 1) {
        std::string str = ei.convertNPVariant(&args[0]);
        std::vector<std::string> iargs;
        iargs.push_back(str);
        str = ei.makeInvoke("GotoFrame", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't goto the specified frame, network problems.");
            return false;
        }        
        // gpso->GotoFrame(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// IsPlaying()
//
// Sends this:
// <invoke name="IsPlaying" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives something like this:
//      </true/>
bool
IsPlaying (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("IsPlaying", iargs);
        
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't check if the movie is playing, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        std::string data = gpso->readPlayer(gpso->getControlFD());
        if (data.empty()) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }

        GnashNPVariant value = ei.parseXML(data);
        if (NPVARIANT_TO_BOOLEAN(value.get()) == true) {
            BOOLEAN_TO_NPVARIANT(true, *result);
        } else {
            BOOLEAN_TO_NPVARIANT(false, *result);
        }

        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// LoadMovie( Layer, Url )
//
// Sends something like this:
// <invoke name="LoadMovie" returntype="xml">
//      <arguments>
//              <number>2</number>
//              <string>bogus</string>
//      </arguments>
// </invoke>
//
//    Receives:
// 	nothing
bool
LoadMovie (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;
    
    if (argCount == 2) {
        ExternalInterface ei;
        // int layer = NPVARIANT_TO_INT32(args[0]);
        // std::string url = NPStringToString(NPVARIANT_TO_STRING(args[1]));
        std::string str = ei.convertNPVariant(&args[0]);
        std::vector<std::string> iargs;
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[1]);
        iargs.push_back(str);
        str = ei.makeInvoke("LoadMovie", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't load the movie, network problems.");
            return false;
        }        
        // gpso->LoadMovie();
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Pan ( x, y, mode )
//
// Sends something like this:
// <invoke name="Pan" returntype="xml">
//      <arguments>
//              <number>1</number>
//              <number>2</number>
//              <number>0</number>
//      </arguments>
// </invoke>
//
// Receives:
//    	nothing
bool
Pan (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 3) {
        ExternalInterface ei;
        std::string str = ei.convertNPVariant(&args[0]);
        std::vector<std::string> iargs;
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[1]);
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[2]);
        iargs.push_back(str);
        str = ei.makeInvoke("Pan", iargs);
        
        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't pan the movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// PercentLoaded()
//
// Sends this:
// <invoke name="PercentLoaded" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives something like this:
//      <number>33</number>
bool
PercentLoaded (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
//    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

#if 1
    static int counter = 0;
//    log_error("%s: %d ; %d\n", __FUNCTION__, gpso->getControlFD(), counter);
    INT32_TO_NPVARIANT(counter, *result);
    if (counter >= 100) {
        counter = 0;
    } else {
        counter += 20;
    }
    return true;
#else
    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("PercentLoaded", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't check percent loaded, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        std::string data = gpso->readPlayer(gpso->getControlFD());
        if (data.empty()) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }
        
        NPVariant *value = ei.parseXML(data);
        if (NPVARIANT_IS_INT32(*value)) {
            INT32_TO_NPVARIANT(NPVARIANT_TO_INT32(*value), *result);
        } else {
            INT32_TO_NPVARIANT(0, *result);
        }

        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
#endif
}

// Play();
//
// Sends this:
// <invoke name="Play" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives:
// 	nothing
bool
Play (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("Play", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't play movie, network problems.");
            return false;
        }        
        // gpso->IsPlaying(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Rewind();
//
// Sends this:
// <invoke name="Rewind" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives:
// 	nothing
bool
Rewind (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("Rewind", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't rewind movie, network problems.");
            return false;
        }        
        // gpso->Rewind(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// SetZoomRect ( left, top, right, bottom )
// Sends something like this:
// <invoke name="SetZoomRect" returntype="xml">
//      <arguments>
//              <number>1</number>
//              <number>2</number>
//              <number>3</number>
//              <number>4</number>
//      </arguments>
// </invoke>
//
// Receives:
// 	nothing
bool
SetZoomRect (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 4) {
        ExternalInterface ei;
        std::string str = ei.convertNPVariant(&args[0]);
        std::vector<std::string> iargs;
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[1]);
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[2]);
        iargs.push_back(str);
        str = ei.convertNPVariant(&args[3]);
        iargs.push_back(str);
        str = ei.makeInvoke("SetZoomRect", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't Set the Zoom Rect the movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// StopPlay()
//
// Sends this:
// <invoke name="StopPlay" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives:
// 	nothing
bool
StopPlay (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("StopPlay", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't stop-play movie, network problems.");
            return false;
        }        
        // gpso->IsPlaying(value);
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// Zoom( percent )
//
// Sends something like this:
// <invoke name="Zoom" returntype="xml">
//      <arguments>
//              <number>12</number>
//      </arguments>
// </invoke>
//
// Receives:
// 	nothing
bool
Zoom (NPObject *npobj, NPIdentifier /* name */, const NPVariant *args,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);

    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 1) {
        ExternalInterface ei;
        std::string str = ei.convertNPVariant(&args[0]);
        std::vector<std::string> iargs;
        iargs.push_back(str);
        str = ei.makeInvoke("Zoom", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't zoom movie, network problems.");
            return false;
        }        
        BOOLEAN_TO_NPVARIANT(true, *result);
        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

// TotalFrames()
//
// Sends something like this:
// <invoke name="TotalFrames" returntype="xml">
//      <arguments></arguments>
// </invoke>
//
// Receives:
//      <number>66</number>
bool
TotalFrames (NPObject *npobj, NPIdentifier /* name */, const NPVariant */*args */,
          uint32_t argCount, NPVariant *result)
{   
    log_debug(__PRETTY_FUNCTION__);
    
    GnashPluginScriptObject *gpso = (GnashPluginScriptObject *)npobj;

    if (argCount == 0) {
        ExternalInterface ei;
        std::vector<std::string> iargs;
        std::string str = ei.makeInvoke("TotalFrames", iargs);

        // Write the message to the Control FD.
        size_t ret = gpso->writePlayer(gpso->getControlFD(), str);
        // Unless we wrote the same amount of data as the message contained,
        // something went wrong.
        if (ret != str.size()) {
            log_error("Couldn't check percent loaded, network problems.");
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }        
        std::string data = gpso->readPlayer(gpso->getControlFD());
        if (data.empty()) {
            BOOLEAN_TO_NPVARIANT(false, *result);
            return false;
        }

        GnashNPVariant value = ei.parseXML(data);
        if (NPVARIANT_IS_INT32(value.get())) {
            value.copy(*result);
        } else {
            INT32_TO_NPVARIANT(0, *result);
        }

        return true;
    }
    
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
