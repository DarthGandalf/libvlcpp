/*****************************************************************************
 * Media.hpp: Media API
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

#ifndef LIBVLC_CXX_MEDIA_H
#define LIBVLC_CXX_MEDIA_H

#include "common.hpp"

#include <vector>
#include <stdexcept>

namespace VLC
{

class MediaPlayer;
class MediaEventManager;
class Instance;
class MediaList;

class Media : public Internal<libvlc_media_t>
{
public:

    enum FromType
    {
        /**
         * Create a media for a certain file path.
         */
        FromPath,
        /**
         * Create a media with a certain given media resource location,
         * for instance a valid URL.
         *
         * \note To refer to a local file with this function,
         * the file://... URI syntax <b>must</b> be used (see IETF RFC3986).
         * We recommend using FromPath instead when dealing with
         * local files.
         */
        FromLocation,
        /**
         * Create a media as an empty node with a given name.
         */
        AsNode,
    };

    /**
     * @brief Media Constructs a libvlc Media instance
     * @param instance  A libvlc instance
     * @param mrl       A path, location, or node name, depending on the 3rd parameter
     * @param type      The type of the 2nd argument. \sa{FromType}
     */
    Media(Instance& instance, const std::string& mrl, FromType type)
        : Internal{ libvlc_media_release }
    {
        InternalPtr ptr = nullptr;
        switch (type)
        {
        case FromLocation:
            ptr = libvlc_media_new_location( getInternalPtr<libvlc_instance_t>( instance ), mrl.c_str() );
            break;
        case FromPath:
            ptr = libvlc_media_new_path( getInternalPtr<libvlc_instance_t>( instance ), mrl.c_str() );
            break;
        case AsNode:
            ptr = libvlc_media_new_as_node( getInternalPtr<libvlc_instance_t>( instance ), mrl.c_str() );
            break;
        default:
            break;
        }
        if ( ptr == nullptr )
            throw std::runtime_error("Failed to construct a media");
        m_obj.reset( ptr, libvlc_media_release );
    }

    /**
     * Create a media for an already open file descriptor.
     * The file descriptor shall be open for reading (or reading and writing).
     *
     * Regular file descriptors, pipe read descriptors and character device
     * descriptors (including TTYs) are supported on all platforms.
     * Block device descriptors are supported where available.
     * Directory descriptors are supported on systems that provide fdopendir().
     * Sockets are supported on all platforms where they are file descriptors,
     * i.e. all except Windows.
     *
     * \note This library will <b>not</b> automatically close the file descriptor
     * under any circumstance. Nevertheless, a file descriptor can usually only be
     * rendered once in a media player. To render it a second time, the file
     * descriptor should probably be rewound to the beginning with lseek().
     *
     * \param p_instance the instance
     * \param fd open file descriptor
     * \return the newly created media or NULL on error
     */
    Media(Instance& instance, int fd)
        : Internal { libvlc_media_new_fd( getInternalPtr<libvlc_instance_t>( instance ), fd ),
                     libvlc_media_release }
    {
    }

    /**
     * Get media instance from this media list instance. This action will increase
     * the refcount on the media instance.
     * The libvlc_media_list_lock should NOT be held upon entering this function.
     *
     * \param p_ml a media list instance
     * \return media instance
     */
    Media(MediaList& list)
        : Internal{ libvlc_media_list_media( getInternalPtr<libvlc_media_list_t>( list ) ),
                    libvlc_media_release }
    {
    }

    explicit Media( Internal::InternalPtr ptr, bool incrementRefCount)
        : Internal{ ptr, libvlc_media_release }
    {
        if (incrementRefCount)
            retain();
    }

    /**
     * Create an empty VLC Media instance.
     *
     * Calling any method on such an instance is undefined.
    */
    Media() = default;

    /**
     * Check if 2 Media objects contain the same libvlc_media_t.
     * \param another another Media
     * \return true if they contain the same libvlc_media_t
     */
    bool operator==(const Media& another) const
    {
        return m_obj == another.m_obj;
    }

    /**
     * Add an option to the media.
     *
     * This option will be used to determine how the media_player will read
     * the media. This allows to use VLC's advanced reading/streaming options
     * on a per-media basis.
     *
     * \note The options are listed in 'vlc long-help' from the command line,
     * e.g. "-sout-all". Keep in mind that available options and their
     * semantics vary across LibVLC versions and builds.
     *
     * \warning Not all options affects libvlc_media_t objects: Specifically,
     * due to architectural issues most audio and video options, such as text
     * renderer options, have no effects on an individual media. These
     * options must be set through Instance::Instance() instead.
     *
     * \param psz_options  the options (as a string)
     */
    void addOption(const std::string& psz_options)
    {
        libvlc_media_add_option(*this,psz_options.c_str());
    }

    /**
     * Add an option to the media with configurable flags.
     *
     * This option will be used to determine how the media_player will read
     * the media. This allows to use VLC's advanced reading/streaming options
     * on a per-media basis.
     *
     * The options are detailed in vlc long-help, for instance "--sout-all".
     * Note that all options are not usable on medias: specifically, due to
     * architectural issues, video-related options such as text renderer
     * options cannot be set on a single media. They must be set on the whole
     * libvlc instance instead.
     *
     * \param psz_options  the options (as a string)
     *
     * \param i_flags  the flags for this option
     */
    void addOptionFlag(const std::string& psz_options, unsigned i_flags)
    {
        libvlc_media_add_option_flag(*this,psz_options.c_str(), i_flags);
    }

    /**
     * Get the media resource locator (mrl) from a media descriptor object
     *
     * \return string with mrl of media descriptor object
     */
    std::string mrl()
    {
        auto str = wrapCStr( libvlc_media_get_mrl(*this) );
        if ( str == nullptr )
            return {};
        return str.get();
    }

    /**
     * Duplicate a media descriptor object.
     */
    MediaPtr duplicate()
    {
        auto obj = libvlc_media_duplicate(*this);
        return std::make_shared<Media>( obj, false );
    }

    /**
     * Read the meta of the media.
     *
     * If the media has not yet been parsed this will return NULL.
     *
     * This methods automatically calls parseAsync() , so after
     * calling it you may receive a libvlc_MediaMetaChanged event. If you
     * prefer a synchronous version ensure that you call parse()
     * before get_meta().
     *
     * \see parse()
     *
     * \see parseAsync()
     *
     * \see libvlc_MediaMetaChanged
     *
     * \param e_meta  the meta to read
     *
     * \return the media's meta
     */
    std::string meta(libvlc_meta_t e_meta)
    {
        auto str = wrapCStr(libvlc_media_get_meta(*this, e_meta) );
        if ( str == nullptr )
            return {};
        return str.get();
    }

    /**
     * Set the meta of the media (this function will not save the meta, call
     * libvlc_media_save_meta in order to save the meta)
     *
     * \param e_meta  the meta to write
     *
     * \param psz_value  the media's meta
     */
    void setMeta(libvlc_meta_t e_meta, const std::string& psz_value)
    {
        libvlc_media_set_meta(*this, e_meta, psz_value.c_str());
    }


    /**
     * Save the meta previously set
     *
     * \return true if the write operation was successful
     */
    int saveMeta()
    {
        return libvlc_media_save_meta(*this);
    }

    /**
     * Get current state of media descriptor object. Possible media states
     * are defined in libvlc_structures.c ( libvlc_NothingSpecial=0,
     * libvlc_Opening, libvlc_Buffering, libvlc_Playing, libvlc_Paused,
     * libvlc_Stopped, libvlc_Ended, libvlc_Error).
     *
     * \see libvlc_state_t
     *
     * \return state of media descriptor object
     */
    libvlc_state_t state()
    {
        return libvlc_media_get_state(*this);
    }

    /**
     * Get the current statistics about the media
     *
     * \param p_stats  structure that contain the statistics about the media
     * (this structure must be allocated by the caller)
     *
     * \return true if the statistics are available, false otherwise
     */
    bool stats(libvlc_media_stats_t * p_stats)
    {
        return libvlc_media_get_stats(*this, p_stats) != 0;
    }

    /**
     * Get event manager from media descriptor object. NOTE: this function
     * doesn't increment reference counting.
     *
     * \return event manager object
     */
    MediaEventManager& eventManager()
    {
        if ( m_eventManager == NULL )
        {
            libvlc_event_manager_t* obj = libvlc_media_event_manager(*this);
            m_eventManager = std::make_shared<MediaEventManager>( obj );
        }
        return *m_eventManager;
    }

    /**
     * Get duration (in ms) of media descriptor object item.
     *
     * \return duration of media item or -1 on error
     */
    libvlc_time_t duration()
    {
        return libvlc_media_get_duration(*this);
    }

    /**
     * Parse a media.
     *
     * This fetches (local) meta data and tracks information. The method is
     * synchronous.
     *
     * \see parseAsync()
     *
     * \see meta()
     *
     * \see tracksInfo()
     */
    void parse()
    {
        libvlc_media_parse(*this);
    }

    /**
     * Parse a media.
     *
     * This fetches (local) meta data and tracks information. The method is
     * the asynchronous of parse() .
     *
     * To track when this is over you can listen to libvlc_MediaParsedChanged
     * event. However if the media was already parsed you will not receive
     * this event.
     *
     * \see parse()
     *
     * \see libvlc_MediaParsedChanged
     *
     * \see meta()
     *
     * \see tracks()
     */
    void parseAsync()
    {
        libvlc_media_parse_async(*this);
    }

    /**
     * Get Parsed status for media descriptor object.
     *
     * \see libvlc_MediaParsedChanged
     *
     * \return true if media object has been parsed otherwise it returns
     * false
     */
    bool isParsed()
    {
        return libvlc_media_is_parsed(*this) != 0;
    }

    /**
     * Sets media descriptor's user_data. user_data is specialized data
     * accessed by the host application, VLC.framework uses it as a pointer
     * to an native object that references a libvlc_media_t pointer
     *
     * \param p_new_user_data  pointer to user data
     */
    void setUserData(void * p_new_user_data)
    {
        libvlc_media_set_user_data(*this, p_new_user_data);
    }

    /**
     * Get media descriptor's user_data. user_data is specialized data
     * accessed by the host application, VLC.framework uses it as a pointer
     * to an native object that references a libvlc_media_t pointer
     */
    void* userData()
    {
        return libvlc_media_get_user_data(*this);
    }

    /**
     * Get media descriptor's elementary streams description
     *
     * Note, you need to call parse() or play the media at least once
     * before calling this function. Not doing this will result in an empty
     * array.
     *
     * \version LibVLC 2.1.0 and later.
     *
     * \return a vector containing all tracks
     */
    std::vector<MediaTrack> tracks()
    {
        libvlc_media_track_t**  tracks;
        uint32_t                nbTracks = libvlc_media_tracks_get(*this, &tracks);
        std::vector<MediaTrack> res;

        if ( nbTracks == 0 )
            return res;

        for ( uint32_t i = 0; i < nbTracks; ++i )
            res.emplace_back( tracks[i] );
        libvlc_media_tracks_release( tracks, nbTracks );
        return res;
    }

private:

    /**
     * Retain a reference to a media descriptor object (libvlc_media_t). Use
     * release() to decrement the reference count of a media
     * descriptor object.
     */
    void retain()
    {
        if ( isValid() )
            libvlc_media_retain(*this);
    }


private:
    std::shared_ptr<MediaEventManager> m_eventManager;
};

} // namespace VLC

#endif

