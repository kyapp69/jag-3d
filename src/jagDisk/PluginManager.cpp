/*************** <auto-copyright.pl BEGIN do not edit this line> **************
*
* jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*
*************** <auto-copyright.pl END do not edit this line> ***************/

#include <jagDisk/PluginManager.h>
#include <jagDisk/ReaderWriter.h>
#include <jagBase/LogMacros.h>

#include <Poco/Glob.h>
#include <Poco/File.h>
#include <Poco/SharedLibrary.h>
#include <Poco/ClassLoader.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/Environment.h>
#include <Poco/AutoPtr.h>
#include <boost/foreach.hpp>

#include <sstream>
#include <iostream>
#include <set>


using Poco::Util::IniFileConfiguration;


namespace jagDisk {


ReaderWriterInfo::ReaderWriterInfo( ReaderWriterPtr instance, const std::string& className,
            const std::string& baseClassName, const std::string& description )
  : _rwInstance( instance ),
    _className( className ),
    _baseClassName( baseClassName ),
    _description( description )
{
    PluginManager::PluginInfo* pi( PluginManager::instance()->getActivelyLoadingPlugin() );
    _pluginName = pi->_name;
    pi->_readerWriters.push_back( instance );

    PluginManager::instance()->addReaderWriter( *this );
}

bool operator<( const ReaderWriterInfo& lhs, const ReaderWriterInfo& rhs )
{
    if( lhs._pluginName < rhs._pluginName )
        return( true );
    else if( lhs._pluginName > rhs._pluginName )
        return( false );

    if( lhs._className < rhs._className )
        return( true );
    else if( lhs._className > rhs._className )
        return( false );

    if( lhs._baseClassName < rhs._baseClassName )
        return( true );
    else if( lhs._baseClassName > rhs._baseClassName )
        return( false );

    if( lhs._description < rhs._description )
        return( true );
    else
        return( false );
}




PluginManager* PluginManager::instance( const int initFlags )
{
    static PluginManager* s_instance( new PluginManager( initFlags ) );
    return( s_instance );
}

PluginManager::PluginManager( const int initFlags )
  : LogBase( "jag.disk.plugmgr" ),
    _activelyLoadingPlugin( NULL )
{
    if( ( initFlags & USE_CURRENT_DIRECTORY ) != 0 )
    {
        addPath( Poco::Path::current(), false );
    }
    if( ( initFlags & USE_JAG3D_PLUGIN_PATN_ENV_VAR ) != 0 )
    {
        std::string paths;
        try {
            paths = Poco::Environment::get( "JAG3D_PLUGIN_PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }
    if( ( initFlags & USE_SYSTEM_PATH ) != 0 )
    {
        std::string paths;
        try {
            paths = Poco::Environment::get( "PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }
    if( ( initFlags & USE_LD_LIBRARY_PATH ) != 0 )
    {
        std::string paths;
        try {
            paths = Poco::Environment::get( "LD_LIBRARY_PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }

    if( !_paths.empty() )
        loadConfigFiles();
}
PluginManager::~PluginManager()
{
}

void PluginManager::addPath( const std::string& path, const bool loadConfigs )
{
    _paths.push_back( path );
    if( loadConfigs )
        loadConfigFiles();
}
void PluginManager::addPaths( const std::string& paths, const bool loadConfigs )
{
    if( paths.empty() )
        return;

    const char sep( Poco::Path::pathSeparator() );
    std::string::size_type pos, lastPos( 0 );
    do {
        pos = paths.find( sep, lastPos );
        const std::string::size_type len( pos - lastPos );
        if( JAG3D_LOG_TRACE )
        {
            std::ostringstream ostr;
            ostr << paths.substr( lastPos, len ) << " " << pos << " " << lastPos;
            JAG3D_TRACE( ostr.str() );
        }
        if( len > 0 )
            _paths.push_back( paths.substr( lastPos, len ) );
        lastPos = pos+1;
    } while( pos != paths.npos );

    if( loadConfigs )
        loadConfigFiles();
}
void PluginManager::clearPaths()
{
    _paths.clear();
}


bool PluginManager::loadPlugins( PluginInfoPtrVec& plugins )
{
    BOOST_FOREACH( PluginInfo* pi, plugins )
    {
        if( pi->_loaded )
            continue;

        if( loadPlugin( pi ) )
            pi->_loaded = true;
        else
            // Abort.
            return( false );
    }

    return( true );
}

bool PluginManager::loadPlugin( PluginInfo* pi )
{
    _activelyLoadingPlugin = pi;

    const Poco::Path& lib( pi->_path );

    typedef Poco::ClassLoader< ReaderWriter > LibLoader;
    LibLoader loader;
    try {
        loader.loadLibrary( lib.toString() );
    } catch( Poco::LibraryLoadException lle ) {
        JAG3D_ERROR( "Can't load \"" + lib.toString() + "\":" );
        JAG3D_ERROR( "\tCaught Poco::LibraryLoadException." );
        JAG3D_ERROR( "\tMessage: " + lle.message() );
        return( false );
    } catch( ... ) {
        JAG3D_ERROR( "Can't load \"" + lib.toString() + "\": unknown exception." );
        return( false );
    }

    return( true );
}

PluginManager::PluginInfoPtrVec PluginManager::getPluginsForExtension( const std::string& extension )
{
    PluginInfoPtrVec plugins;

    BOOST_FOREACH( PluginInfo& pi, _pluginInfo )
    {
        if( ( pi._extensions.find( extension ) != pi._extensions.end() ) ||
            ( pi._extensions.find( "*" ) != pi._extensions.end() ) )
            // The PluginInfo either supports this extension explicitly,
            // or claims to support all extensions ("*").
            plugins.push_back( &pi );
    }

    return( plugins );
}


const ReaderWriterInfoVec& PluginManager::getLoadedReaderWriters() const
{
    return( _rwInfo );
}


void PluginManager::addReaderWriter( const ReaderWriterInfo& rwInfo )
{
    _rwInfo.push_back( rwInfo );
}


void PluginManager::loadConfigFiles()
{
    typedef std::set< std::string > StringSet;

    _pluginInfo.clear();

    BOOST_FOREACH( Poco::Path::StringVec::value_type s, _paths )
    {
        Poco::Path path, libPath;
        if( !( path.tryParse( s.c_str() ) ) )
            continue;
        path.makeDirectory();
        path.setFileName( "*.jagpi" );

        StringSet stringSet;
        Poco::Glob::glob( path, stringSet, Poco::Glob::GLOB_DOT_SPECIAL );
        BOOST_FOREACH( StringSet::value_type jagpiFileName, stringSet )
        {
            JAG3D_TRACE( jagpiFileName );

            std::string extensions;
            Poco::AutoPtr< IniFileConfiguration > conf( new IniFileConfiguration( jagpiFileName ) );
            PluginInfo pi;
            try {
                extensions = conf->getString( "Jag3DPlugin.Extensions" );
                pi._name = conf->getString( "Jag3DPlugin.Name" );
                pi._description = conf->getString( "Jag3DPlugin.Description" );
            }
            catch( ... )
            {
                // Not one of our .jagpi files, or badly formed .jagpi file.
                continue;
            }

            // Parse the extensions string into a vector of individual strings.
            while( !( extensions.empty() ) )
            {
                size_t pos( extensions.find( ";" ) );
                if( pos != std::string::npos )
                {
                    pi._extensions.insert( extensions.substr( 0, pos ) );
                    extensions = extensions.substr( pos+1 );
                }
                else
                {
                    pi._extensions.insert( extensions );
                    extensions.clear();
                }
            }

            pi._path = path;
            pi._path.setFileName( Poco::Path( jagpiFileName ).getBaseName()
                + Poco::SharedLibrary::suffix() );
            if( !( Poco::File( pi._path ).exists() ) )
                continue;

            _pluginInfo.push_back( pi );

            JAG3D_TRACE( pi._path.toString() );
            JAG3D_TRACE( pi._name );
            JAG3D_TRACE( pi._description );
        }
    }
}


bool operator<( const PluginManager::PluginInfo& lhs, const PluginManager::PluginInfo& rhs )
{
    if( lhs._name < rhs._name )
        return( true );
    else
        return( false );
}


// jagDisk
}
