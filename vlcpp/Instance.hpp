/*****************************************************************************
 * Instance.hpp: Instance API
 *****************************************************************************
 * Copyright © 2015 libvlcpp authors & VideoLAN
 *
 * Authors: Alexey Sokolov <alexey+vlc@asokolov.org>
 *          Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef LIBVLC_CXX_INSTANCE_H
#define LIBVLC_CXX_INSTANCE_H

#include "common.hpp"
#include "Internal.hpp"
#include "structures.hpp"

#include <string>
#include <vector>

namespace VLC
{

class Instance : public Internal<libvlc_instance_t>
{
public:
    /**
     * Create and initialize a libvlc instance. This functions accept a list
     * of "command line" arguments similar to the main(). These arguments
     * affect the LibVLC instance default configuration.
     *
     * \version Arguments are meant to be passed from the command line to
     * LibVLC, just like VLC media player does. The list of valid arguments
     * depends on the LibVLC version, the operating system and platform, and
     * set of available LibVLC plugins. Invalid or unsupported arguments will
     * cause the function to fail (i.e. return NULL). Also, some arguments
     * may alter the behaviour or otherwise interfere with other LibVLC
     * functions.
     *
     * \warning There is absolutely no warranty or promise of forward,
     * backward and cross-platform compatibility with regards to
     * Instance::Instance() arguments. We recommend that you do not use them,
     * other than when debugging.
     *
     * \param argc  the number of arguments (should be 0)
     *
     * \param argv  list of arguments (should be NULL)
     */
    Instance(int argc, const char *const * argv)
        : Internal{ libvlc_new( argc, argv ), libvlc_release }
    {
    }

    /**
     * Create an empty VLC instance.
     *
     * Calling any method on such an instance is undefined.
    */
    Instance() = default;

    /**
     * Check if 2 Instance objects contain the same libvlc_instance_t.
     * \param another another Instance
     * \return true if they contain the same libvlc_instance_t
     */
    bool operator==(const Instance& another) const
    {
        return m_obj == another.m_obj;
    }


    /**
     * Try to start a user interface for the libvlc instance.
     *
     * \param name  interface name, or NULL for default
     *
     * \return 0 on success, -1 on error.
     */
    int addIntf(const std::string& name)
    {
        return libvlc_add_intf( *this, name.c_str() );
    }

    /**
     * Registers a callback for the LibVLC exit event. This is mostly useful
     * if the VLC playlist and/or at least one interface are started with
     * libvlc_playlist_play() or Instance::addIntf() respectively. Typically,
     * this function will wake up your application main loop (from another
     * thread).
     *
     * \note This function should be called before the playlist or interface
     * are started. Otherwise, there is a small race condition: the exit
     * event could be raised before the handler is registered.
     *
     * \param cb  callback to invoke when LibVLC wants to exit, or NULL to
     * disable the exit handler (as by default)
     *
     * \param opaque  data pointer for the callback
     *
     * \warning This function and Instance::wait() cannot be used at the same
     * time.
     */
    void setExitHandler(void(*cb)(void *), void * opaque)
    {
        libvlc_set_exit_handler( *this, cb, opaque );
    }

    /**
     * Sets the application name. LibVLC passes this as the user agent string
     * when a protocol requires it.
     *
     * \param name  human-readable application name, e.g. "FooBar player
     * 1.2.3"
     *
     * \param http  HTTP User Agent, e.g. "FooBar/1.2.3 Python/2.6.0"
     *
     * \version LibVLC 1.1.1 or later
     */
    void setUserAgent(const std::string& name, const std::string& http)
    {
        libvlc_set_user_agent( *this, name.c_str(), http.c_str() );
    }

    /**
     * Sets some meta-information about the application. See also
     * Instance::setUserAgent() .
     *
     * \param id  Java-style application identifier, e.g. "com.acme.foobar"
     *
     * \param version  application version numbers, e.g. "1.2.3"
     *
     * \param icon  application icon name, e.g. "foobar"
     *
     * \version LibVLC 2.1.0 or later.
     */
    void setAppId(const std::string& id, const std::string& version, const std::string& icon)
    {
        libvlc_set_app_id( *this, id.c_str(), version.c_str(), icon.c_str() );
    }

    /**
     * Unsets the logging callback for a LibVLC instance. This is rarely
     * needed: the callback is implicitly unset when the instance is
     * destroyed. This function will wait for any pending callbacks
     * invocation to complete (causing a deadlock if called from within the
     * callback).
     *
     * \version LibVLC 2.1.0 or later
     */
    void logUnset()
    {
        libvlc_log_unset( *this );
    }

    /**
     * Sets the logging callback for a LibVLC instance. This function is
     * thread-safe: it will wait for any pending callbacks invocation to
     * complete.
     *
     * \param data  opaque data pointer for the callback function
     *
     * \note Some log messages (especially debug) are emitted by LibVLC while
     * is being initialized. These messages cannot be captured with this
     * interface.
     *
     * \warning A deadlock may occur if this function is called from the
     * callback.
     *
     * \param p_instance  libvlc instance
     *
     * \version LibVLC 2.1.0 or later
     */
    void logSet(libvlc_log_cb cb, void * data)
    {
        libvlc_log_set(*this, cb, data);
    }

    /**
     * Sets up logging to a file.
     *
     * \param stream  FILE pointer opened for writing (the FILE pointer must
     * remain valid until Instance::logUnset() )
     *
     * \version LibVLC 2.1.0 or later
     */
    void logSetFile(FILE * stream)
    {
        libvlc_log_set_file( *this, stream );
    }

    /**
     * Returns a list of audio filters that are available.
     *
     * \return a list of module descriptions. It should be freed with
     * ModuleDescription::moduleDescriptionListRelease() . In case of an
     * error, NULL is returned.
     *
     * \see ModuleDescription
     *
     * \see ModuleDescription::moduleDescriptionListRelease()
     */
    std::vector<ModuleDescription> audioFilterList()
    {
        auto releaser = [](libvlc_module_description_t* ptr) { libvlc_module_description_list_release(ptr); };
        auto ptr = std::unique_ptr<libvlc_module_description_t, decltype(releaser)>( libvlc_audio_filter_list_get(*this), releaser );
        if ( ptr == nullptr )
            return {};
        libvlc_module_description_t* p = ptr.get();
        std::vector<ModuleDescription> res;
        while ( p != NULL )
        {
            res.emplace_back( p );
            p = p->p_next;
        }
        return res;
    }


    /**
     * Returns a list of video filters that are available.
     *
     * \return a list of module descriptions. It should be freed with
     * ModuleDescription::moduleDescriptionListRelease() . In case of an
     * error, NULL is returned.
     *
     * \see ModuleDescription
     *
     * \see ModuleDescription::moduleDescriptionListRelease()
     */
    std::vector<ModuleDescription> videoFilterList()
    {
        auto releaser = [](libvlc_module_description_t* ptr) { libvlc_module_description_list_release(ptr); };
        auto ptr = std::unique_ptr<libvlc_module_description_t, decltype(releaser)>( libvlc_video_filter_list_get(*this), releaser );
        if ( ptr == nullptr )
            return {};
        libvlc_module_description_t* p = ptr.get();
        std::vector<ModuleDescription> res;
        while ( p != NULL )
        {
            res.emplace_back( p );
            p = p->p_next;
        }
        return res;
    }

    /**
     * Gets the list of available audio output modules.
     *
     * \return list of available audio outputs. It must be freed it with
     *
     * \see AudioOutputDescription::audioOutputListRelease()
     *
     * \see AudioOutputDescription . In case of error, NULL is returned.
     */
    std::vector<AudioOutputDescription> audioOutputList()
    {
        libvlc_audio_output_t* result = libvlc_audio_output_list_get(*this);
        std::vector<AudioOutputDescription> res;
        if ( result == NULL )
            return res;
        libvlc_audio_output_t* p = result;
        while ( p != NULL )
        {
            res.emplace_back( p );
            p = p->p_next;
        }
        libvlc_audio_output_list_release(result);
        return res;
    }

    /**
     * Gets a list of audio output devices for a given audio output module,
     *
     * \see Audio::outputDeviceSet() .
     *
     * \note Not all audio outputs support this. In particular, an empty
     * (NULL) list of devices does imply that the specified audio output does
     * not work.
     *
     * \note The list might not be exhaustive.
     *
     * \warning Some audio output devices in the list might not actually work
     * in some circumstances. By default, it is recommended to not specify
     * any explicit audio device.
     *
     * \param psz_aout  audio output name (as returned by
     * Instance::audioOutputList() )
     *
     * \return A vector containing all audio output devices for this module
     *
     * \version LibVLC 2.1.0 or later.
     */
    std::vector<AudioOutputDeviceDescription> audioOutputDeviceList(const std::string& aout)
    {
        libvlc_audio_output_device_t* devices = libvlc_audio_output_device_list_get( *this, aout.c_str() );
        std::vector<AudioOutputDeviceDescription> res;
        if ( devices == NULL )
            return res;
        for ( libvlc_audio_output_device_t* p = devices; p != NULL; p = p->p_next )
            res.emplace_back( p );
        libvlc_audio_output_device_list_release( devices );
        return res;
    }

};

} // namespace VLC

#endif

