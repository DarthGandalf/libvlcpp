/*****************************************************************************
 * MediaDiscoverer.hpp: MediaDiscoverer API
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

#ifndef LIBVLC_CXX_MEDIADISCOVERER_H
#define LIBVLC_CXX_MEDIADISCOVERER_H

#include "common.hpp"

#include <string>
#include <vlc/libvlc_version.h>

namespace VLC
{

class MediaDiscovererEventManager;
class Instance;

class MediaDiscoverer : public Internal<libvlc_media_discoverer_t>
{
public:
    /**
     * Discover media service by name.
     *
     * \param p_inst  libvlc instance
     *
     * \param psz_name  service name
     * \warning This is returned as a pointer, as this is not refcounter by VLC, and is
     *          fairly expensive to instantiate.
     */
    MediaDiscoverer(Instance& inst, const std::string& name)
#if LIBVLC_VERSION(3, 0, 0, 0) >= LIBVLC_VERSION_INT
        : Internal{ libvlc_media_discoverer_new(getInternalPtr<libvlc_instance_t>( inst ), name.c_str()),
#else
        : Internal{ libvlc_media_discoverer_new_from_name(getInternalPtr<libvlc_instance_t>( inst ), name.c_str()),
#endif
                    libvlc_media_discoverer_release }
    {
    }

#if LIBVLC_VERSION(3, 0, 0, 0) >= LIBVLC_VERSION_INT
    /**
     * Start media discovery.
     *
     * To stop it, call MediaDiscover::stop() or
     * destroy the object directly.
     *
     * \see MediaDiscover::stop
     *
     * \return false in case of error, true otherwise
     */
    bool start()
    {
        return libvlc_media_discoverer_start( *this ) == 0;
    }

    /**
     * Stop media discovery.
     *
     * \see MediaDiscoverer::start()
     */
    void stop()
    {
        libvlc_media_discoverer_stop( *this );
    }
#endif

    /**
     * Get media service discover object its localized name.
     *
     * \return localized name
     */
    std::string localizedName()
    {
        auto str = wrapCStr( libvlc_media_discoverer_localized_name(*this) );
        if ( str == nullptr )
            return {};
        return str.get();
    }

    /**
     * Get event manager from media service discover object.
     *
     * \return event manager object.
     */
    MediaDiscovererEventManager& eventManager()
    {
        if ( m_eventManager )
        {
            libvlc_event_manager_t* obj = libvlc_media_discoverer_event_manager( *this );
            m_eventManager = std::make_shared<MediaDiscovererEventManager>( obj );
        }
        return *m_eventManager;
    }

    /**
     * Query if media service discover object is running.
     *
     * \return true if running, false if not
     */
    bool isRunning()
    {
        return libvlc_media_discoverer_is_running(*this) != 0;
    }

private:
    std::shared_ptr<MediaDiscovererEventManager> m_eventManager;
};

} // namespace VLC

#endif

