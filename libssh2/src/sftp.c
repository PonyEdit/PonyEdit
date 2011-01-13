/* Copyright (c) 2004-2008, Sara Golemon <sarag@libssh2.org>
 * Copyright (c) 2007 Eli Fant <elifantu@mail.ru>
 * Copyright (c) 2009-2010 by Daniel Stenberg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 *   Redistributions of source code must retain the above
 *   copyright notice, this list of conditions and the
 *   following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials
 *   provided with the distribution.
 *
 *   Neither the name of the copyright holder nor the names
 *   of any other contributors may be used to endorse or
 *   promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include <assert.h>

#include "libssh2_priv.h"
#include "libssh2_sftp.h"
#include "channel.h"
#include "session.h"

/* Note: Version 6 was documented at the time of writing
 * However it was marked as "DO NOT IMPLEMENT" due to pending changes
 *
 * This release of libssh2 implements Version 5 with automatic downgrade
 * based on server's declaration
 */

/* SFTP packet types */
#define SSH_FXP_INIT                            1
#define SSH_FXP_VERSION                         2
#define SSH_FXP_OPEN                            3
#define SSH_FXP_CLOSE                           4
#define SSH_FXP_READ                            5
#define SSH_FXP_WRITE                           6
#define SSH_FXP_LSTAT                           7
#define SSH_FXP_FSTAT                           8
#define SSH_FXP_SETSTAT                         9
#define SSH_FXP_FSETSTAT                        10
#define SSH_FXP_OPENDIR                         11
#define SSH_FXP_READDIR                         12
#define SSH_FXP_REMOVE                          13
#define SSH_FXP_MKDIR                           14
#define SSH_FXP_RMDIR                           15
#define SSH_FXP_REALPATH                        16
#define SSH_FXP_STAT                            17
#define SSH_FXP_RENAME                          18
#define SSH_FXP_READLINK                        19
#define SSH_FXP_SYMLINK                         20
#define SSH_FXP_STATUS                          101
#define SSH_FXP_HANDLE                          102
#define SSH_FXP_DATA                            103
#define SSH_FXP_NAME                            104
#define SSH_FXP_ATTRS                           105
#define SSH_FXP_EXTENDED                        200
#define SSH_FXP_EXTENDED_REPLY                  201

#define LIBSSH2_SFTP_HANDLE_FILE        0
#define LIBSSH2_SFTP_HANDLE_DIR         1

/* S_IFREG */
#define LIBSSH2_SFTP_ATTR_PFILETYPE_FILE        0100000
/* S_IFDIR */
#define LIBSSH2_SFTP_ATTR_PFILETYPE_DIR         0040000

#define SSH_FXE_STATVFS_ST_RDONLY               0x00000001
#define SSH_FXE_STATVFS_ST_NOSUID               0x00000002

static int sftp_close_handle(LIBSSH2_SFTP_HANDLE *handle);

/* sftp_attrsize
 * Size that attr with this flagset will occupy when turned into a bin struct
 */
static int sftp_attrsize(unsigned long flags)
{
    return (4 +                                 /* flags(4) */
            ((flags & LIBSSH2_SFTP_ATTR_SIZE) ? 8 : 0) +
            ((flags & LIBSSH2_SFTP_ATTR_UIDGID) ? 8 : 0) +
            ((flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) ? 4 : 0) +
            ((flags & LIBSSH2_SFTP_ATTR_ACMODTIME) ? 8 : 0));
                                                /* atime + mtime as u32 */
}

/* _libssh2_store_u64
 */
static void _libssh2_store_u64(unsigned char **ptr, libssh2_uint64_t value)
{
    uint32_t msl = (uint32_t)(value >> 32);
    unsigned char *buf = *ptr;

    buf[0] = (unsigned char)((msl >> 24) & 0xFF);
    buf[1] = (unsigned char)((msl >> 16) & 0xFF);
    buf[2] = (unsigned char)((msl >> 8)  & 0xFF);
    buf[3] = (unsigned char)( msl        & 0xFF);

    buf[4] = (unsigned char)((value >> 24) & 0xFF);
    buf[5] = (unsigned char)((value >> 16) & 0xFF);
    buf[6] = (unsigned char)((value >> 8)  & 0xFF);
    buf[7] = (unsigned char)( value        & 0xFF);

    *ptr += 8;
}

/*
 * sftp_packet_add
 *
 * Add a packet to the SFTP packet brigade
 */
static int
sftp_packet_add(LIBSSH2_SFTP *sftp, unsigned char *data,
                size_t data_len)
{
    LIBSSH2_SESSION *session = sftp->channel->session;
    LIBSSH2_PACKET *packet;

    _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Received packet %d (len %d)",
                   (int) data[0], data_len);
    packet = LIBSSH2_ALLOC(session, sizeof(LIBSSH2_PACKET));
    if (!packet) {
        return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                              "Unable to allocate datablock for SFTP packet");
    }
    memset(packet, 0, sizeof(LIBSSH2_PACKET));

    packet->data = data;
    packet->data_len = data_len;
    packet->data_head = 5;

    _libssh2_list_add(&sftp->packets, &packet->node);

    return 0;
}

/*
 * sftp_packet_read
 *
 * Frame an SFTP packet off the channel
 */
static int
sftp_packet_read(LIBSSH2_SFTP *sftp)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    unsigned char buffer[4];    /* To store the packet length */
    unsigned char *packet;
    size_t packet_len, packet_received;
    ssize_t bytes_received;
    int rc;

    _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "recv packet");

    /* If there was a previous partial, start using it */
    if (sftp->partial_packet) {

        packet = sftp->partial_packet;
        packet_len = sftp->partial_len;
        packet_received = sftp->partial_received;
        sftp->partial_packet = NULL;

        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "partial read cont, len: %lu", packet_len);
    }
    else {
        rc = _libssh2_channel_read(channel, 0, (char *) buffer, 4);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        }
        else if (4 != rc) {
            /* TODO: this is stupid since we can in fact get 1-3 bytes in a
               legitimate working case as well if the connection happens to be
               super slow or something */
            return _libssh2_error(session, LIBSSH2_ERROR_CHANNEL_FAILURE,
                                  "Read part of packet");
        }

        packet_len = _libssh2_ntohu32(buffer);
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Data begin - Packet Length: %lu", packet_len);
        if (packet_len > LIBSSH2_SFTP_PACKET_MAXLEN) {
            return _libssh2_error(session, LIBSSH2_ERROR_CHANNEL_PACKET_EXCEEDED,
                                  "SFTP packet too large");
        }

        packet = LIBSSH2_ALLOC(session, packet_len);
        if (!packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate SFTP packet");
        }

        packet_received = 0;
    }

    /* Read as much of the packet as we can */
    while (packet_len > packet_received) {
        bytes_received =
            _libssh2_channel_read(channel, 0,
                                  (char *) packet + packet_received,
                                  packet_len - packet_received);

        if (bytes_received == LIBSSH2_ERROR_EAGAIN) {
            /*
             * We received EAGAIN, save what we have and
             * return to EAGAIN to the caller
             */
            sftp->partial_packet = packet;
            sftp->partial_len = packet_len;
            sftp->partial_received = packet_received;
            packet = NULL;

            return bytes_received;
        }
        else if (bytes_received < 0) {
            LIBSSH2_FREE(session, packet);
            return _libssh2_error(session, bytes_received,
                                  "Receive error waiting for SFTP packet");
        }
        packet_received += bytes_received;
    }

    rc = sftp_packet_add(sftp, packet, packet_len);
    if (rc) {
        LIBSSH2_FREE(session, packet);
        return rc;
    }

    return packet[0];
}

/*
 * sftp_packet_ask()
 *
 * Checks if there's a matching SFTP packet available.
 */
static int
sftp_packet_ask(LIBSSH2_SFTP *sftp, unsigned char packet_type,
                int request_id, unsigned char **data,
                size_t *data_len)
{
    LIBSSH2_SESSION *session = sftp->channel->session;
    LIBSSH2_PACKET *packet = _libssh2_list_first(&sftp->packets);
    unsigned char match_buf[5];
    int match_len;

    _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Asking for %d packet",
                   (int) packet_type);

    match_buf[0] = packet_type;
    if (packet_type == SSH_FXP_VERSION) {
        /* Special consideration when matching VERSION packet */
        match_len = 1;
    } else {
        match_len = 5;
        _libssh2_htonu32(match_buf + 1, request_id);
    }

    while (packet) {
        if (!memcmp((char *) packet->data, (char *) match_buf, match_len)) {

            /* Match! Fetch the data */
            *data = packet->data;
            *data_len = packet->data_len;

            /* unlink and free this struct */
            _libssh2_list_remove(&packet->node);
            LIBSSH2_FREE(session, packet);

            return 0;
        }
        /* check next struct in the list */
        packet = _libssh2_list_next(&packet->node);
    }
    return -1;
}

/* sftp_packet_require
 * A la libssh2_packet_require
 */
static int
sftp_packet_require(LIBSSH2_SFTP *sftp, unsigned char packet_type,
                    int request_id, unsigned char **data,
                    size_t *data_len)
{
    LIBSSH2_SESSION *session = sftp->channel->session;
    int ret;

    _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Requiring packet %d id %ld",
                   (int) packet_type, request_id);

    if (sftp_packet_ask(sftp, packet_type, request_id, data, data_len) == 0) {
        /* The right packet was available in the packet brigade */
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Got %d",
                       (int) packet_type);
        return 0;
    }

    while (session->socket_state == LIBSSH2_SOCKET_CONNECTED) {
        ret = sftp_packet_read(sftp);
        if (ret == LIBSSH2_ERROR_EAGAIN) {
            return ret;
        } else if (ret <= 0) {
            return -1;
        }

        /* data was read, check the queue again */
        if (!sftp_packet_ask(sftp, packet_type, request_id, data, data_len)) {
            /* The right packet was available in the packet brigade */
            _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Got %d",
                           (int) packet_type);
            return 0;
        }
    }

    /* Only reached if the socket died */
    return LIBSSH2_ERROR_SOCKET_DISCONNECT;
}

/* sftp_packet_requirev
 * Require one of N possible reponses
 */
static int
sftp_packet_requirev(LIBSSH2_SFTP *sftp, int num_valid_responses,
                     const unsigned char *valid_responses,
                     int request_id, unsigned char **data,
                     size_t *data_len)
{
    int i;
    int ret;

    /* If no timeout is active, start a new one */
    if (sftp->requirev_start == 0) {
        sftp->requirev_start = time(NULL);
    }

    while (sftp->channel->session->socket_state == LIBSSH2_SOCKET_CONNECTED) {
        for(i = 0; i < num_valid_responses; i++) {
            if (sftp_packet_ask(sftp, valid_responses[i], request_id,
                                data, data_len) == 0) {
                /*
                 * Set to zero before all returns to say
                 * the timeout is not active
                 */
                sftp->requirev_start = 0;
                return 0;
            }
        }

        ret = sftp_packet_read(sftp);
        if ((ret < 0) && (ret != LIBSSH2_ERROR_EAGAIN)) {
            sftp->requirev_start = 0;
            return -1;
        } else if (ret <= 0) {
            /* prevent busy-looping */
            long left =
                LIBSSH2_READ_TIMEOUT - (long)(time(NULL) - sftp->requirev_start);

            if (left <= 0) {
                sftp->requirev_start = 0;
                return LIBSSH2_ERROR_TIMEOUT;
            }
            else if (ret == LIBSSH2_ERROR_EAGAIN) {
                return ret;
            }
        }
    }

    sftp->requirev_start = 0;
    return -1;
}

/* sftp_attr2bin
 * Populate attributes into an SFTP block
 */
static int
sftp_attr2bin(unsigned char *p, const LIBSSH2_SFTP_ATTRIBUTES * attrs)
{
    unsigned char *s = p;
    uint32_t flag_mask =
        LIBSSH2_SFTP_ATTR_SIZE | LIBSSH2_SFTP_ATTR_UIDGID |
        LIBSSH2_SFTP_ATTR_PERMISSIONS | LIBSSH2_SFTP_ATTR_ACMODTIME;

    /* TODO: When we add SFTP4+ functionality flag_mask can get additional
       bits */

    if (!attrs) {
        _libssh2_htonu32(s, 0);
        return 4;
    }

    _libssh2_store_u32(&s, attrs->flags & flag_mask);

    if (attrs->flags & LIBSSH2_SFTP_ATTR_SIZE) {
        _libssh2_store_u64(&s, attrs->filesize);
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        _libssh2_store_u32(&s, attrs->uid);
        _libssh2_store_u32(&s, attrs->gid);
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        _libssh2_store_u32(&s, attrs->permissions);
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
        _libssh2_store_u32(&s, attrs->atime);
        _libssh2_store_u32(&s, attrs->mtime);
    }

    return (s - p);
}

/* sftp_bin2attr
 */
static int
sftp_bin2attr(LIBSSH2_SFTP_ATTRIBUTES * attrs, const unsigned char *p)
{
    const unsigned char *s = p;

    memset(attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    attrs->flags = _libssh2_ntohu32(s);
    s += 4;

    if (attrs->flags & LIBSSH2_SFTP_ATTR_SIZE) {
        attrs->filesize = _libssh2_ntohu64(s);
        s += 8;
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_UIDGID) {
        attrs->uid = _libssh2_ntohu32(s);
        s += 4;
        attrs->gid = _libssh2_ntohu32(s);
        s += 4;
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_PERMISSIONS) {
        attrs->permissions = _libssh2_ntohu32(s);
        s += 4;
    }

    if (attrs->flags & LIBSSH2_SFTP_ATTR_ACMODTIME) {
        attrs->atime = _libssh2_ntohu32(s);
        s += 4;
        attrs->mtime = _libssh2_ntohu32(s);
        s += 4;
    }

    return (s - p);
}

/* ************
 * SFTP API *
 ************ */

LIBSSH2_CHANNEL_CLOSE_FUNC(libssh2_sftp_dtor);

/* libssh2_sftp_dtor
 * Shutdown an SFTP stream when the channel closes
 */
LIBSSH2_CHANNEL_CLOSE_FUNC(libssh2_sftp_dtor)
{
    LIBSSH2_SFTP *sftp = (LIBSSH2_SFTP *) (*channel_abstract);

    (void) session_abstract;
    (void) channel;

    /* Free the partial packet storage for sftp_packet_read */
    if (sftp->partial_packet) {
        LIBSSH2_FREE(session, sftp->partial_packet);
    }

    /* Free the packet storage for _libssh2_sftp_packet_readdir */
    if (sftp->readdir_packet) {
        LIBSSH2_FREE(session, sftp->readdir_packet);
    }

    LIBSSH2_FREE(session, sftp);
}

/*
 * sftp_init
 *
 * Startup an SFTP session
 */
static LIBSSH2_SFTP *sftp_init(LIBSSH2_SESSION *session)
{
    unsigned char *data, *s;
    size_t data_len;
    int rc;
    LIBSSH2_SFTP *sftp_handle;

    if (session->sftpInit_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Initializing SFTP subsystem");

        /*
         * The 'sftpInit_sftp' and 'sftpInit_channel' struct fields within the
         * session struct are only to be used during the setup phase. As soon
         * as the SFTP session is created they are cleared and can thus be
         * re-used again to allow any amount of SFTP handles per sessions.
         *
         * Note that you MUST NOT try to call libssh2_sftp_init() again to get
         * another handle until the previous call has finished and either
         * succesffully made a handle or failed and returned error (not
         * including *EAGAIN).
         */

        assert(session->sftpInit_sftp == NULL);
        session->sftpInit_sftp = NULL;
        session->sftpInit_state = libssh2_NB_state_created;
    }

    sftp_handle = session->sftpInit_sftp;

    if (session->sftpInit_state == libssh2_NB_state_created) {
        session->sftpInit_channel =
            _libssh2_channel_open(session, "session", sizeof("session") - 1,
                                  LIBSSH2_CHANNEL_WINDOW_DEFAULT,
                                  LIBSSH2_CHANNEL_PACKET_DEFAULT, NULL, 0);
        if (!session->sftpInit_channel) {
            if (libssh2_session_last_errno(session) == LIBSSH2_ERROR_EAGAIN) {
                _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                               "Would block starting up channel");
            }
            else {
                _libssh2_error(session, LIBSSH2_ERROR_CHANNEL_FAILURE,
                               "Unable to startup channel");
                session->sftpInit_state = libssh2_NB_state_idle;
            }
            return NULL;
        }

        session->sftpInit_state = libssh2_NB_state_sent;
    }

    if (session->sftpInit_state == libssh2_NB_state_sent) {
        rc = _libssh2_channel_process_startup(session->sftpInit_channel,
                                              "subsystem",
                                              sizeof("subsystem") - 1, "sftp",
                                              strlen("sftp"));
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                           "Would block to request SFTP subsystem");
            return NULL;
        } else if (rc) {
            _libssh2_error(session, LIBSSH2_ERROR_CHANNEL_FAILURE,
                           "Unable to request SFTP subsystem");
            goto sftp_init_error;
        }

        session->sftpInit_state = libssh2_NB_state_sent1;
    }

    if (session->sftpInit_state == libssh2_NB_state_sent1) {
        rc = _libssh2_channel_extended_data(session->sftpInit_channel,
                                            LIBSSH2_CHANNEL_EXTENDED_DATA_IGNORE);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                           "Would block requesting handle extended data");
            return NULL;
        }

        sftp_handle =
            session->sftpInit_sftp =
            LIBSSH2_ALLOC(session, sizeof(LIBSSH2_SFTP));
        if (!sftp_handle) {
            _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                           "Unable to allocate a new SFTP structure");
            goto sftp_init_error;
        }
        memset(sftp_handle, 0, sizeof(LIBSSH2_SFTP));
        sftp_handle->channel = session->sftpInit_channel;
        sftp_handle->request_id = 0;

        _libssh2_htonu32(session->sftpInit_buffer, 5);
        session->sftpInit_buffer[4] = SSH_FXP_INIT;
        _libssh2_htonu32(session->sftpInit_buffer + 5, LIBSSH2_SFTP_VERSION);
        session->sftpInit_sent = 0; /* nothing's sent yet */

        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Sending FXP_INIT packet advertising version %d support",
                       (int) LIBSSH2_SFTP_VERSION);

        session->sftpInit_state = libssh2_NB_state_sent2;
    }

    if (session->sftpInit_state == libssh2_NB_state_sent2) {
        /* sent off what's left of the init buffer to send */
        rc = _libssh2_channel_write(session->sftpInit_channel, 0,
                                    (char *)session->sftpInit_buffer +
                                    session->sftpInit_sent,
                                    9 - session->sftpInit_sent);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                           "Would block sending SSH_FXP_INIT");
            return NULL;
        }
        else if(rc < 0) {
            _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                           "Unable to send SSH_FXP_INIT");
            goto sftp_init_error;
        }
        else {
            /* add up the number of bytes sent */
            session->sftpInit_sent += rc;

            if(session->sftpInit_sent == 9)
                /* move on */
                session->sftpInit_state = libssh2_NB_state_sent3;

            /* if less than 9, we remain in this state to send more later on */
        }
    }

    rc = sftp_packet_require(sftp_handle, SSH_FXP_VERSION,
                             0, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                       "Would block waiting for response from SFTP subsystem");
        return NULL;
    } else if (rc) {
        _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                       "Timeout waiting for response from SFTP subsystem");
        goto sftp_init_error;
    }
    if (data_len < 5) {
        _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                       "Invalid SSH_FXP_VERSION response");
        goto sftp_init_error;
    }

    s = data + 1;
    sftp_handle->version = _libssh2_ntohu32(s);
    s += 4;
    if (sftp_handle->version > LIBSSH2_SFTP_VERSION) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Truncating remote SFTP version from %lu",
                       sftp_handle->version);
        sftp_handle->version = LIBSSH2_SFTP_VERSION;
    }
    _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                   "Enabling SFTP version %lu compatability",
                   sftp_handle->version);
    while (s < (data + data_len)) {
        size_t extname_len, extdata_len;

        extname_len = _libssh2_ntohu32(s);
        s += 4;
        /* the extension name starts here */
        s += extname_len;

        extdata_len = _libssh2_ntohu32(s);
        s += 4;

        /* TODO: Actually process extensions */
        s += extdata_len;

    }
    LIBSSH2_FREE(session, data);

    /* Make sure that when the channel gets closed, the SFTP service is shut
       down too */
    sftp_handle->channel->abstract = sftp_handle;
    sftp_handle->channel->close_cb = libssh2_sftp_dtor;

    session->sftpInit_state = libssh2_NB_state_idle;

    /* clear the sftp and channel pointers in this session struct now */
    session->sftpInit_sftp = NULL;
    session->sftpInit_channel = NULL;

    _libssh2_list_init(&sftp_handle->sftp_handles);

    return sftp_handle;

  sftp_init_error:
    while (_libssh2_channel_free(session->sftpInit_channel) ==
           LIBSSH2_ERROR_EAGAIN);
    session->sftpInit_channel = NULL;
    if (session->sftpInit_sftp) {
        LIBSSH2_FREE(session, session->sftpInit_sftp);
        session->sftpInit_sftp = NULL;
    }
    session->sftpInit_state = libssh2_NB_state_idle;
    return NULL;
}

/*
 * libssh2_sftp_init
 *
 * Startup an SFTP session
 */
LIBSSH2_API LIBSSH2_SFTP *libssh2_sftp_init(LIBSSH2_SESSION *session)
{
    LIBSSH2_SFTP *ptr;

    if(!session)
        return NULL;

    if(!(session->state & LIBSSH2_STATE_AUTHENTICATED)) {
        _libssh2_error(session, LIBSSH2_ERROR_INVAL,
                       "session not authenticated yet");
        return NULL;
    }

    BLOCK_ADJUST_ERRNO(ptr, session, sftp_init(session));
    return ptr;
}

/*
 * sftp_shutdown
 *
 * Shutsdown the SFTP subsystem
 */
static int
sftp_shutdown(LIBSSH2_SFTP *sftp)
{
    int rc;
    LIBSSH2_SESSION *session = sftp->channel->session;
    /*
     * Make sure all memory used in the state variables are free
     */
    if (sftp->partial_packet) {
        LIBSSH2_FREE(session, sftp->partial_packet);
        sftp->partial_packet = NULL;
    }
    if (sftp->open_packet) {
        LIBSSH2_FREE(session, sftp->open_packet);
        sftp->open_packet = NULL;
    }
    if (sftp->readdir_packet) {
        LIBSSH2_FREE(session, sftp->readdir_packet);
        sftp->readdir_packet = NULL;
    }
    if (sftp->write_packet) {
        LIBSSH2_FREE(session, sftp->write_packet);
        sftp->write_packet = NULL;
    }
    if (sftp->fstat_packet) {
        LIBSSH2_FREE(session, sftp->fstat_packet);
        sftp->fstat_packet = NULL;
    }
    if (sftp->unlink_packet) {
        LIBSSH2_FREE(session, sftp->unlink_packet);
        sftp->unlink_packet = NULL;
    }
    if (sftp->rename_packet) {
        LIBSSH2_FREE(session, sftp->rename_packet);
        sftp->rename_packet = NULL;
    }
    if (sftp->fstatvfs_packet) {
        LIBSSH2_FREE(session, sftp->fstatvfs_packet);
        sftp->fstatvfs_packet = NULL;
    }
    if (sftp->statvfs_packet) {
        LIBSSH2_FREE(session, sftp->statvfs_packet);
        sftp->statvfs_packet = NULL;
    }
    if (sftp->mkdir_packet) {
        LIBSSH2_FREE(session, sftp->mkdir_packet);
        sftp->mkdir_packet = NULL;
    }
    if (sftp->rmdir_packet) {
        LIBSSH2_FREE(session, sftp->rmdir_packet);
        sftp->rmdir_packet = NULL;
    }
    if (sftp->stat_packet) {
        LIBSSH2_FREE(session, sftp->stat_packet);
        sftp->stat_packet = NULL;
    }
    if (sftp->symlink_packet) {
        LIBSSH2_FREE(session, sftp->symlink_packet);
        sftp->symlink_packet = NULL;
    }

    /* TODO: We should consider walking over the sftp_handles list and kill
     * any remaining sftp handles ... */

    rc = _libssh2_channel_free(sftp->channel);

    return rc;
}

/* libssh2_sftp_shutdown
 * Shutsdown the SFTP subsystem
 */
LIBSSH2_API int
libssh2_sftp_shutdown(LIBSSH2_SFTP *sftp)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session, sftp_shutdown(sftp));
    return rc;
}

/* *******************************
 * SFTP File and Directory Ops *
 ******************************* */

/* sftp_open
 */
static LIBSSH2_SFTP_HANDLE *
sftp_open(LIBSSH2_SFTP *sftp, const char *filename,
          size_t filename_len, uint32_t flags, long mode,
          int open_type)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    LIBSSH2_SFTP_HANDLE *fp;
    LIBSSH2_SFTP_ATTRIBUTES attrs = {
        LIBSSH2_SFTP_ATTR_PERMISSIONS, 0, 0, 0, 0, 0, 0
    };
    unsigned char *s;
    int rc;
    int open_file = (open_type == LIBSSH2_SFTP_OPENFILE)?1:0;

    if (sftp->open_state == libssh2_NB_state_idle) {
        /* packet_len(4) + packet_type(1) + request_id(4) + filename_len(4) +
           flags(4) */
        sftp->open_packet_len = filename_len + 13 +
            (open_file? (4 + sftp_attrsize(LIBSSH2_SFTP_ATTR_PERMISSIONS)) : 0);

        /* surprise! this starts out with nothing sent */
        sftp->open_packet_sent = 0;
        s = sftp->open_packet = LIBSSH2_ALLOC(session, sftp->open_packet_len);
        if (!sftp->open_packet) {
            _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                           "Unable to allocate memory for FXP_OPEN or "
                           "FXP_OPENDIR packet");
            return NULL;
        }
        /* Filetype in SFTP 3 and earlier */
        attrs.permissions = mode |
            (open_file ? LIBSSH2_SFTP_ATTR_PFILETYPE_FILE :
             LIBSSH2_SFTP_ATTR_PFILETYPE_DIR);

        _libssh2_store_u32(&s, sftp->open_packet_len - 4);
        *(s++) = open_file? SSH_FXP_OPEN : SSH_FXP_OPENDIR;
        sftp->open_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->open_request_id);
        _libssh2_store_str(&s, filename, filename_len);

        if (open_file) {
            _libssh2_store_u32(&s, flags);
            s += sftp_attr2bin(s, &attrs);
        }

        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Sending %s open request",
                       open_file? "file" : "directory");

        sftp->open_state = libssh2_NB_state_created;
    }

    if (sftp->open_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->open_packet+
                                    sftp->open_packet_sent,
                                    sftp->open_packet_len -
                                    sftp->open_packet_sent);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                           "Would block sending FXP_OPEN or FXP_OPENDIR command");
            return NULL;
        }
        else if(rc < 0) {
            _libssh2_error(session, rc, "Unable to send FXP_OPEN*");
            LIBSSH2_FREE(session, sftp->open_packet);
            sftp->open_packet = NULL;
            sftp->open_state = libssh2_NB_state_idle;
            return NULL;
        }

        /* bump the sent counter and remain in this state until the whole
           data is off */
        sftp->open_packet_sent += rc;

        if(sftp->open_packet_len == sftp->open_packet_sent) {
            LIBSSH2_FREE(session, sftp->open_packet);
            sftp->open_packet = NULL;

            sftp->open_state = libssh2_NB_state_sent;
        }
    }

    if (sftp->open_state == libssh2_NB_state_sent) {
        size_t data_len;
        unsigned char *data;
        static const unsigned char fopen_responses[2] =
            { SSH_FXP_HANDLE, SSH_FXP_STATUS };
        rc = sftp_packet_requirev(sftp, 2, fopen_responses,
                                  sftp->open_request_id, &data,
                                  &data_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            _libssh2_error(session, LIBSSH2_ERROR_EAGAIN,
                           "Would block waiting for status message");
            return NULL;
        }
        sftp->open_state = libssh2_NB_state_idle;
        if (rc) {
            _libssh2_error(session, rc, "Timeout waiting for status message");
            return NULL;
        }

        /* OPEN can basically get STATUS or HANDLE back, where HANDLE implies
           a fine response while STATUS means error. It seems though that at
           times we get an SSH_FX_OK back in a STATUS, followed the "real"
           HANDLE so we need to properly deal with that. */
        if (data[0] == SSH_FXP_STATUS) {
            int badness = 1;

            if(data_len < 9) {
                _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                               "Too small FXP_STATUS");
                LIBSSH2_FREE(session, data);
                return NULL;
            }

            sftp->last_errno = _libssh2_ntohu32(data + 5);

            if(LIBSSH2_FX_OK == sftp->last_errno) {
                _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "got HANDLE FXOK!");

                LIBSSH2_FREE(session, data);

                /* silly situation, but check for a HANDLE */
                rc = sftp_packet_require(sftp, SSH_FXP_HANDLE,
                                         sftp->open_request_id, &data,
                                         &data_len);
                if(rc == LIBSSH2_ERROR_EAGAIN) {
                    /* go back to sent state and wait for something else */
                    sftp->open_state = libssh2_NB_state_sent;
                    return NULL;
                }
                else if(!rc)
                    /* we got the handle so this is not a bad situation */
                    badness = 0;
            }

            if(badness) {
                _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                               "Failed opening remote file");
                _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "got FXP_STATUS %d",
                               sftp->last_errno);
                LIBSSH2_FREE(session, data);
                return NULL;
            }
        }

        if(data_len < 10) {
            _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                           "Too small FXP_HANDLE");
            LIBSSH2_FREE(session, data);
            return NULL;
        }

        fp = LIBSSH2_ALLOC(session, sizeof(LIBSSH2_SFTP_HANDLE));
        if (!fp) {
            _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                           "Unable to allocate new SFTP handle structure");
            LIBSSH2_FREE(session, data);
            return NULL;
        }
        memset(fp, 0, sizeof(LIBSSH2_SFTP_HANDLE));
        fp->handle_type = open_file ? LIBSSH2_SFTP_HANDLE_FILE :
            LIBSSH2_SFTP_HANDLE_DIR;

        fp->handle_len = _libssh2_ntohu32(data + 5);
        if (fp->handle_len > SFTP_HANDLE_MAXLEN)
            /* SFTP doesn't allow handles longer than 256 characters */
            fp->handle_len = SFTP_HANDLE_MAXLEN;

        if(fp->handle_len > (data_len - 9))
            /* do not reach beyond the end of the data we got */
            fp->handle_len = data_len - 9;

        memcpy(fp->handle, data + 9, fp->handle_len);

        LIBSSH2_FREE(session, data);

        /* add this file handle to the list kept in the sftp session */
        _libssh2_list_add(&sftp->sftp_handles, &fp->node);

        fp->sftp = sftp; /* point to the parent struct */

        fp->u.file.offset = 0;

        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Open command successful");
        return fp;
    }
    return NULL;
}

/* libssh2_sftp_open_ex
 */
LIBSSH2_API LIBSSH2_SFTP_HANDLE *
libssh2_sftp_open_ex(LIBSSH2_SFTP *sftp, const char *filename,
                     unsigned int filename_len, unsigned long flags, long mode,
                     int open_type)
{
    LIBSSH2_SFTP_HANDLE *hnd;

    if(!sftp)
        return NULL;

    BLOCK_ADJUST_ERRNO(hnd, sftp->channel->session,
                       sftp_open(sftp, filename, filename_len, flags, mode,
                                 open_type));
    return hnd;
}

/* sftp_read
 * Read from an SFTP file handle
 */
static ssize_t sftp_read(LIBSSH2_SFTP_HANDLE * handle, char *buffer,
                         size_t buffer_maxlen)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len, request_id = 0;
    /* 25 = packet_len(4) + packet_type(1) + request_id(4) + handle_len(4) +
       offset(8) + length(4) */
    ssize_t packet_len = handle->handle_len + 25;
    unsigned char *packet, *s, *data;
    size_t bytes_read = 0;
    size_t bytes_requested = 0;
    size_t total_read = 0;
    int retcode;

    if (sftp->read_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Reading %lu bytes from SFTP handle",
                       (unsigned long) buffer_maxlen);
        packet = handle->request_packet;
        sftp->read_state = libssh2_NB_state_allocated;
    } else {
        packet = sftp->read_packet;
        request_id = sftp->read_request_id;
        total_read = sftp->read_total_read;
    }

    while (total_read < buffer_maxlen) {
        s = packet;
        /*
         * If buffer_maxlen bytes will be requested, server may return all
         * with one packet.  But libssh2 have packet length limit.
         * So we request data by pieces.
         */
        bytes_requested = buffer_maxlen - total_read;
        /* 10 = packet_type(1) + request_id(4) + data_length(4) +
           end_of_line_flag(1)

           10 is changed to 13 below simple because it seems there's a
           "GlobalScape" SFTP server that responds with a slightly too big
           buffer at times and we can apparently compensate for that by doing
           this trick.

           Further details on this issue:

           https://sourceforge.net/mailarchive/forum.php?thread_name=9c3275a90811261517v6c0b1da2u918cc1b8370abf83%40mail.gmail.com&forum_name=libssh2-devel

           http://forums.globalscape.com/tm.aspx?m=15249

        */
        if (bytes_requested > LIBSSH2_SFTP_PACKET_MAXLEN - 13) {
            bytes_requested = LIBSSH2_SFTP_PACKET_MAXLEN - 13;
        }
#ifdef LIBSSH2_DEBUG_SFTP
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Requesting %lu bytes from SFTP handle",
                       (unsigned long) bytes_requested);
#endif

        if (sftp->read_state == libssh2_NB_state_allocated) {
            _libssh2_store_u32(&s, packet_len - 4);
            *(s++) = SSH_FXP_READ;
            request_id = sftp->request_id++;
            _libssh2_store_u32(&s, request_id);
            _libssh2_store_str(&s, handle->handle, handle->handle_len);
            _libssh2_store_u64(&s, handle->u.file.offset);
            _libssh2_store_u32(&s, bytes_requested);

            sftp->read_state = libssh2_NB_state_created;
        }

        if (sftp->read_state == libssh2_NB_state_created) {
            retcode = _libssh2_channel_write(channel, 0, (char *) packet,
                                             packet_len);
            if (retcode == LIBSSH2_ERROR_EAGAIN) {
                sftp->read_packet = packet;
                sftp->read_request_id = request_id;
                sftp->read_total_read = total_read;
                return retcode;
            } else if (packet_len != retcode) {
                /* TODO: a partial write is not a critical error when in
                   non-blocking mode! */
                sftp->read_packet = NULL;
                sftp->read_state = libssh2_NB_state_idle;
                return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                      "_libssh2_channel_write() failed");
            }
            sftp->read_packet = packet;
            sftp->read_request_id = request_id;
            sftp->read_total_read = total_read;
            sftp->read_state = libssh2_NB_state_sent;
        }

        if (sftp->read_state == libssh2_NB_state_sent) {
            static const unsigned char read_responses[2] =
                { SSH_FXP_DATA, SSH_FXP_STATUS };
            retcode =
                sftp_packet_requirev(sftp, 2, read_responses,
                                     request_id, &data, &data_len);
            if (retcode == LIBSSH2_ERROR_EAGAIN) {
                return _libssh2_error(session, retcode,
                                      "Would block waiting for status message");
            } else if (retcode) {
                sftp->read_packet = NULL;
                sftp->read_state = libssh2_NB_state_idle;
                return _libssh2_error(session, retcode,
                                      "Timeout waiting for status message");
            }

            sftp->read_state = libssh2_NB_state_sent1;
        }
        else
            /* internal error, 'data' is not assigned */
            return -1;

        switch (data[0]) {
        case SSH_FXP_STATUS:
            retcode = _libssh2_ntohu32(data + 5);
            LIBSSH2_FREE(session, data);
            sftp->read_packet = NULL;
            sftp->read_state = libssh2_NB_state_idle;

            if (retcode == LIBSSH2_FX_EOF) {
                return total_read;
            } else {
                sftp->last_errno = retcode;
                return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                      "SFTP Protocol Error");
            }

        case SSH_FXP_DATA:
            bytes_read = _libssh2_ntohu32(data + 5);
            if (bytes_read > (data_len - 9)) {
                sftp->read_packet = NULL;
                sftp->read_state = libssh2_NB_state_idle;
                return -1;
            }
#ifdef LIBSSH2_DEBUG_SFTP
            _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "%lu bytes returned",
                           (unsigned long) bytes_read);
#endif
            memcpy(buffer + total_read, data + 9, bytes_read);
            handle->u.file.offset += bytes_read;
            total_read += bytes_read;
            LIBSSH2_FREE(session, data);
            /*
             * Set the state back to allocated, so a new one will be
             * created to either request more data or get EOF
             */
            sftp->read_state = libssh2_NB_state_allocated;
        }
    }

    sftp->read_packet = NULL;
    sftp->read_state = libssh2_NB_state_idle;
    return total_read;
}

/* libssh2_sftp_read
 * Read from an SFTP file handle
 */
LIBSSH2_API ssize_t
libssh2_sftp_read(LIBSSH2_SFTP_HANDLE *hnd, char *buffer,
                  size_t buffer_maxlen)
{
    ssize_t rc;
    if(!hnd)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, hnd->sftp->channel->session,
                 sftp_read(hnd, buffer, buffer_maxlen));
    return rc;
}

/* sftp_readdir
 * Read from an SFTP directory handle
 */
static int sftp_readdir(LIBSSH2_SFTP_HANDLE *handle, char *buffer,
                        size_t buffer_maxlen, char *longentry,
                        size_t longentry_maxlen,
                        LIBSSH2_SFTP_ATTRIBUTES *attrs)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len, filename_len, longentry_len, num_names;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + handle_len(4) */
    ssize_t packet_len = handle->handle_len + 13;
    unsigned char *s, *data;
    static const unsigned char read_responses[2] = {
        SSH_FXP_NAME, SSH_FXP_STATUS };
    int retcode;

    if (sftp->readdir_state == libssh2_NB_state_idle) {
        if (handle->u.dir.names_left) {
            /*
             * A prior request returned more than one directory entry,
             * feed it back from the buffer
             */
            LIBSSH2_SFTP_ATTRIBUTES attrs_dummy;
            size_t real_longentry_len;
            unsigned char *s = (unsigned char *) handle->u.dir.next_name;
            size_t real_filename_len = _libssh2_ntohu32(s);

            s += 4;

            filename_len = real_filename_len;
            if (filename_len >= buffer_maxlen)
                /* leave room for the trailing zero */
                filename_len = buffer_maxlen - 1;

            memcpy(buffer, s, filename_len);
            buffer[filename_len] = '\0';           /* zero terminate */
            s += real_filename_len;

            real_longentry_len = _libssh2_ntohu32(s);
            s += 4;

            if (longentry && (longentry_maxlen>1)) {
                longentry_len = real_longentry_len;

                if (longentry_len >= longentry_maxlen)
                    /* leave room for the trailing zero */
                    longentry_len = longentry_maxlen -1;

                memcpy(longentry, s, longentry_len);
                longentry[longentry_len] = '\0'; /* zero terminate */
            }
            s += real_longentry_len;

            if (attrs)
                memset(attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));

            s += sftp_bin2attr(attrs ? attrs : &attrs_dummy, s);

            handle->u.dir.next_name = (char *) s;
            if ((--handle->u.dir.names_left) == 0)
                LIBSSH2_FREE(session, handle->u.dir.names_packet);

            _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                           "libssh2_sftp_readdir_ex() return %d",
                           filename_len);
            return filename_len;
        }

        /* Request another entry(entries?) */

        s = sftp->readdir_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->readdir_packet)
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for "
                                  "FXP_READDIR packet");

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_READDIR;
        sftp->readdir_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->readdir_request_id);
        _libssh2_store_str(&s, handle->handle, handle->handle_len);

        sftp->readdir_state = libssh2_NB_state_created;
    }

    if (sftp->readdir_state == libssh2_NB_state_created) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Reading entries from directory handle");
        retcode = _libssh2_channel_write(channel, 0,
                                         (char *) sftp->readdir_packet,
                                         packet_len);
        if (retcode == LIBSSH2_ERROR_EAGAIN) {
            return retcode;
        }
        else if (packet_len != retcode) {
            LIBSSH2_FREE(session, sftp->readdir_packet);
            sftp->readdir_packet = NULL;
            sftp->readdir_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "_libssh2_channel_write() failed");
        }

        LIBSSH2_FREE(session, sftp->readdir_packet);
        sftp->readdir_packet = NULL;

        sftp->readdir_state = libssh2_NB_state_sent;
    }

    retcode = sftp_packet_requirev(sftp, 2, read_responses,
                                   sftp->readdir_request_id, &data,
                                   &data_len);
    if (retcode == LIBSSH2_ERROR_EAGAIN)
        return retcode;
    else if (retcode) {
        sftp->readdir_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    if (data[0] == SSH_FXP_STATUS) {
        retcode = _libssh2_ntohu32(data + 5);
        LIBSSH2_FREE(session, data);
        if (retcode == LIBSSH2_FX_EOF) {
            sftp->readdir_state = libssh2_NB_state_idle;
            return 0;
        }
        else {
            sftp->last_errno = retcode;
            sftp->readdir_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                  "SFTP Protocol Error");
        }
    }

    sftp->readdir_state = libssh2_NB_state_idle;

    num_names = _libssh2_ntohu32(data + 5);
    _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "%lu entries returned",
                   num_names);
    if (num_names <= 0) {
        LIBSSH2_FREE(session, data);
        return (num_names == 0) ? 0 : -1;
    }

    handle->u.dir.names_left = num_names;
    handle->u.dir.names_packet = data;
    handle->u.dir.next_name = (char *) data + 9;

    /* use the name popping mechanism from the start of the function */
    return sftp_readdir(handle, buffer, buffer_maxlen, longentry,
                        longentry_maxlen, attrs);
}

/* libssh2_sftp_readdir_ex
 * Read from an SFTP directory handle
 */
LIBSSH2_API int
libssh2_sftp_readdir_ex(LIBSSH2_SFTP_HANDLE *hnd, char *buffer,
                        size_t buffer_maxlen, char *longentry,
                        size_t longentry_maxlen,
                        LIBSSH2_SFTP_ATTRIBUTES *attrs)
{
    int rc;
    if(!hnd)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, hnd->sftp->channel->session,
                 sftp_readdir(hnd, buffer, buffer_maxlen, longentry,
                              longentry_maxlen, attrs));
    return rc;
}

/*
 * sftp_write
 *
 * Write data to an SFTP handle. Returns the number of bytes written, or
 * a negative error code.
 */
static ssize_t sftp_write(LIBSSH2_SFTP_HANDLE *handle, const char *buffer,
                          size_t count)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len, retcode;
    /* 25 = packet_len(4) + packet_type(1) + request_id(4) + handle_len(4) +
       offset(8) + count(4) */
    ssize_t packet_len;
    unsigned char *s, *data;
    int rc;

    /* we limit this to just send a single SSH packet at a time */
    if(count > 32500)
        count = 32500;

    packet_len = handle->handle_len + count + 25;

    if (sftp->write_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Writing %lu bytes",
                       (unsigned long) count);
        s = sftp->write_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->write_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_WRITE");
        }
        _libssh2_store_u32(&s, packet_len - 4);

        *(s++) = SSH_FXP_WRITE;
        sftp->write_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->write_request_id);
        _libssh2_store_str(&s, handle->handle, handle->handle_len);
        _libssh2_store_u64(&s, handle->u.file.offset);
        _libssh2_store_str(&s, buffer, count);

        sftp->write_state = libssh2_NB_state_created;
    }

    if (sftp->write_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *)sftp->write_packet,
                                    packet_len);
        if(rc < 0) {
            /* error */
            return rc;
        }
        else if(0 == rc) {
            /* nothing sent is an error */
            return LIBSSH2_ERROR_SOCKET_SEND;
        }
        else if (packet_len != rc) {
            return rc;
        }
        LIBSSH2_FREE(session, sftp->write_packet);
        sftp->write_packet = NULL;
        sftp->write_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_STATUS,
                             sftp->write_request_id, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    }
    else if (rc) {
        sftp->write_state = libssh2_NB_state_idle;
        return _libssh2_error(session, rc,
                              "Timeout waiting for status message");
    }

    sftp->write_state = libssh2_NB_state_idle;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    if (retcode == LIBSSH2_FX_OK) {
        handle->u.file.offset += count;
        return count;
    }
    sftp->last_errno = retcode;

    return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                          "SFTP Protocol Error");
}

/* libssh2_sftp_write
 * Write data to a file handle
 */
LIBSSH2_API ssize_t
libssh2_sftp_write(LIBSSH2_SFTP_HANDLE *hnd, const char *buffer,
                   size_t count)
{
    ssize_t rc;
    if(!hnd)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, hnd->sftp->channel->session,
                 sftp_write(hnd, buffer, count));
    return rc;

}

/*
 * sftp_fstat
 *
 * Get or Set stat on a file
 */
static int sftp_fstat(LIBSSH2_SFTP_HANDLE *handle,
                      LIBSSH2_SFTP_ATTRIBUTES *attrs, int setstat)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + handle_len(4) */
    ssize_t packet_len =
        handle->handle_len + 13 + (setstat ? sftp_attrsize(attrs->flags) : 0);
    unsigned char *s, *data;
    static const unsigned char fstat_responses[2] =
        { SSH_FXP_ATTRS, SSH_FXP_STATUS };
    int rc;

    if (sftp->fstat_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Issuing %s command",
                       setstat ? "set-stat" : "stat");
        s = sftp->fstat_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->fstat_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for "
                                  "FSTAT/FSETSTAT packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = setstat ? SSH_FXP_FSETSTAT : SSH_FXP_FSTAT;
        sftp->fstat_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->fstat_request_id);
        _libssh2_store_str(&s, handle->handle, handle->handle_len);

        if (setstat) {
            s += sftp_attr2bin(s, attrs);
        }

        sftp->fstat_state = libssh2_NB_state_created;
    }

    if (sftp->fstat_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->fstat_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->fstat_packet);
            sftp->fstat_packet = NULL;
            sftp->fstat_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  (setstat ? "Unable to send FXP_FSETSTAT"
                                   : "Unable to send FXP_FSTAT command"));
        }
        LIBSSH2_FREE(session, sftp->fstat_packet);
        sftp->fstat_packet = NULL;

        sftp->fstat_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_requirev(sftp, 2, fstat_responses,
                              sftp->fstat_request_id, &data,
                              &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->fstat_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->fstat_state = libssh2_NB_state_idle;

    if (data[0] == SSH_FXP_STATUS) {
        int retcode;

        retcode = _libssh2_ntohu32(data + 5);
        LIBSSH2_FREE(session, data);
        if (retcode == LIBSSH2_FX_OK) {
            return 0;
        } else {
            sftp->last_errno = retcode;
            return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                  "SFTP Protocol Error");
        }
    }

    sftp_bin2attr(attrs, data + 5);
    LIBSSH2_FREE(session, data);

    return 0;
}

/* libssh2_sftp_fstat_ex
 * Get or Set stat on a file
 */
LIBSSH2_API int
libssh2_sftp_fstat_ex(LIBSSH2_SFTP_HANDLE *hnd,
                      LIBSSH2_SFTP_ATTRIBUTES *attrs, int setstat)
{
    int rc;
    if(!hnd || !attrs)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, hnd->sftp->channel->session,
                 sftp_fstat(hnd, attrs, setstat));
    return rc;
}

/* libssh2_sftp_seek
 * Set the read/write pointer to an arbitrary position within the file
 */
LIBSSH2_API void
libssh2_sftp_seek(LIBSSH2_SFTP_HANDLE *handle, size_t offset)
{
    if(handle)
        handle->u.file.offset = offset;
}

/* libssh2_sftp_seek64
 * Set the read/write pointer to an arbitrary position within the file
 */
LIBSSH2_API void
libssh2_sftp_seek64(LIBSSH2_SFTP_HANDLE *handle, libssh2_uint64_t offset)
{
    if(handle)
        handle->u.file.offset = offset;
}

/* libssh2_sftp_tell
 * Return the current read/write pointer's offset
 */
LIBSSH2_API size_t
libssh2_sftp_tell(LIBSSH2_SFTP_HANDLE *handle)
{
    if(!handle)
        return 0; /* no handle, no size */

    /* NOTE: this may very well truncate the size if it is larger than what
       size_t can hold, so libssh2_sftp_tell64() is really the function you
       should use */
    return (size_t)(handle->u.file.offset);
}

/* libssh2_sftp_tell64
 * Return the current read/write pointer's offset
 */
LIBSSH2_API libssh2_uint64_t
libssh2_sftp_tell64(LIBSSH2_SFTP_HANDLE *handle)
{
    if(!handle)
        return 0; /* no handle, no size */

    return handle->u.file.offset;
}

/* sftp_close_handle
 *
 * Close a file or directory handle
 * Also frees handle resource and unlinks it from the SFTP structure
 */
static int
sftp_close_handle(LIBSSH2_SFTP_HANDLE *handle)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    int retcode;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + handle_len(4) */
    ssize_t packet_len = handle->handle_len + 13;
    unsigned char *s, *data = NULL;
    int rc;

    if (handle->close_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Closing handle");
        s = handle->close_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!handle->close_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_CLOSE "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_CLOSE;
        handle->close_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, handle->close_request_id);
        _libssh2_store_str(&s, handle->handle, handle->handle_len);
        handle->close_state = libssh2_NB_state_created;
    }

    if (handle->close_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) handle->close_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, handle->close_packet);
            handle->close_packet = NULL;
            handle->close_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send FXP_CLOSE command");
        }
        LIBSSH2_FREE(session, handle->close_packet);
        handle->close_packet = NULL;

        handle->close_state = libssh2_NB_state_sent;
    }

    if (handle->close_state == libssh2_NB_state_sent) {
        rc = sftp_packet_require(sftp, SSH_FXP_STATUS,
                                 handle->close_request_id, &data,
                                 &data_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (rc) {
            handle->close_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                                  "Timeout waiting for status message");
        }

        handle->close_state = libssh2_NB_state_sent1;
    }

    if(!data)
        /* if it reaches this point with data unset, something unwanted
           happened (like this function is called again when in
           libssh2_NB_state_sent1 state) and we just bail out */
        return LIBSSH2_ERROR_INVAL;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    if (retcode != LIBSSH2_FX_OK) {
        sftp->last_errno = retcode;
        handle->close_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error");
    }

    /* remove this handle from the parent's list */
    _libssh2_list_remove(&handle->node);

    if ((handle->handle_type == LIBSSH2_SFTP_HANDLE_DIR)
        && handle->u.dir.names_left) {
        LIBSSH2_FREE(session, handle->u.dir.names_packet);
    }

    handle->close_state = libssh2_NB_state_idle;

    LIBSSH2_FREE(session, handle);

    return 0;
}

/* libssh2_sftp_close_handle
 *
 * Close a file or directory handle
 * Also frees handle resource and unlinks it from the SFTP structure
 */
LIBSSH2_API int
libssh2_sftp_close_handle(LIBSSH2_SFTP_HANDLE *hnd)
{
    int rc;
    if(!hnd)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, hnd->sftp->channel->session, sftp_close_handle(hnd));
    return rc;
}

/* sftp_unlink
 * Delete a file from the remote server
 */
static int sftp_unlink(LIBSSH2_SFTP *sftp, const char *filename,
                       unsigned int filename_len)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    int retcode;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + filename_len(4) */
    ssize_t packet_len = filename_len + 13;
    unsigned char *s, *data;
    int rc;

    if (sftp->unlink_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Unlinking %s", filename);
        s = sftp->unlink_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->unlink_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_REMOVE "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_REMOVE;
        sftp->unlink_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->unlink_request_id);
        _libssh2_store_str(&s, filename, filename_len);
        sftp->unlink_state = libssh2_NB_state_created;
    }

    if (sftp->unlink_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->unlink_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->unlink_packet);
            sftp->unlink_packet = NULL;
            sftp->unlink_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send FXP_REMOVE command");
        }
        LIBSSH2_FREE(session, sftp->unlink_packet);
        sftp->unlink_packet = NULL;

        sftp->unlink_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_STATUS,
                             sftp->unlink_request_id, &data,
                             &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    }
    else if (rc) {
        sftp->unlink_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->unlink_state = libssh2_NB_state_idle;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    if (retcode == LIBSSH2_FX_OK) {
        return 0;
    } else {
        sftp->last_errno = retcode;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error");
    }
}

/* libssh2_sftp_unlink_ex
 * Delete a file from the remote server
 */
LIBSSH2_API int
libssh2_sftp_unlink_ex(LIBSSH2_SFTP *sftp, const char *filename,
                       unsigned int filename_len)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_unlink(sftp, filename, filename_len));
    return rc;
}

/*
 * sftp_rename
 *
 * Rename a file on the remote server
 */
static int sftp_rename(LIBSSH2_SFTP *sftp, const char *source_filename,
                       unsigned int source_filename_len,
                       const char *dest_filename,
                       unsigned int dest_filename_len, long flags)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    int retcode;
    ssize_t packet_len =
        source_filename_len + dest_filename_len + 17 + (sftp->version >=
                                                        5 ? 4 : 0);
    /* packet_len(4) + packet_type(1) + request_id(4) +
       source_filename_len(4) + dest_filename_len(4) + flags(4){SFTP5+) */
    unsigned char *data;
    int rc;

    if (sftp->version < 2) {
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "Server does not support RENAME");
    }

    if (sftp->rename_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Renaming %s to %s",
                       source_filename, dest_filename);
        sftp->rename_s = sftp->rename_packet =
            LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->rename_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_RENAME "
                                  "packet");
        }

        _libssh2_store_u32(&sftp->rename_s, packet_len - 4);
        *(sftp->rename_s++) = SSH_FXP_RENAME;
        sftp->rename_request_id = sftp->request_id++;
        _libssh2_store_u32(&sftp->rename_s, sftp->rename_request_id);
        _libssh2_store_str(&sftp->rename_s, source_filename,
                           source_filename_len);
        _libssh2_store_str(&sftp->rename_s, dest_filename, dest_filename_len);

        if (sftp->version >= 5)
            _libssh2_store_u32(&sftp->rename_s, flags);

        sftp->rename_state = libssh2_NB_state_created;
    }

    if (sftp->rename_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->rename_packet,
                                    sftp->rename_s - sftp->rename_packet);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->rename_packet);
            sftp->rename_packet = NULL;
            sftp->rename_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send FXP_RENAME command");
        }
        LIBSSH2_FREE(session, sftp->rename_packet);
        sftp->rename_packet = NULL;

        sftp->rename_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_STATUS,
                             sftp->rename_request_id, &data,
                             &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->rename_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->rename_state = libssh2_NB_state_idle;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    sftp->last_errno = retcode;

    /* now convert the SFTP error code to libssh2 return code or error
       message */
    switch (retcode) {
    case LIBSSH2_FX_OK:
        retcode = LIBSSH2_ERROR_NONE;
        break;

    case LIBSSH2_FX_FILE_ALREADY_EXISTS:
        retcode = _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                 "File already exists and "
                                 "SSH_FXP_RENAME_OVERWRITE not specified");
        break;

    case LIBSSH2_FX_OP_UNSUPPORTED:
        retcode = _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                 "Operation Not Supported");
        break;

    default:
        retcode = _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                 "SFTP Protocol Error");
        break;
    }

    return retcode;
}

/* libssh2_sftp_rename_ex
 * Rename a file on the remote server
 */
LIBSSH2_API int
libssh2_sftp_rename_ex(LIBSSH2_SFTP *sftp, const char *source_filename,
                       unsigned int source_filename_len,
                       const char *dest_filename,
                       unsigned int dest_filename_len, long flags)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_rename(sftp, source_filename, source_filename_len,
                             dest_filename, dest_filename_len, flags));
    return rc;
}

/*
 * sftp_fstatvfs
 *
 * Get file system statistics
 */
static int sftp_fstatvfs(LIBSSH2_SFTP_HANDLE *handle, LIBSSH2_SFTP_STATVFS *st)
{
    LIBSSH2_SFTP *sftp = handle->sftp;
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    /* 17 = packet_len(4) + packet_type(1) + request_id(4) + ext_len(4)
          + handle_len (4) */
    /* 20 = strlen ("fstatvfs@openssh.com") */
    ssize_t packet_len = handle->handle_len + 20 + 17;
    unsigned char *packet, *s, *data;
    int rc;
    unsigned int flag;

    if (sftp->fstatvfs_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Getting file system statistics");
        s = packet = LIBSSH2_ALLOC(session, packet_len);
        if (!packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_EXTENDED "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_EXTENDED;
        sftp->fstatvfs_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->fstatvfs_request_id);
        _libssh2_store_str(&s, "fstatvfs@openssh.com", 20);
        _libssh2_store_str(&s, handle->handle, handle->handle_len);

        sftp->fstatvfs_state = libssh2_NB_state_created;
    }
    else {
        packet = sftp->fstatvfs_packet;
    }

    if (sftp->fstatvfs_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) packet, packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN || (0 <= rc && rc < packet_len)) {
            sftp->fstatvfs_packet = packet;
            return LIBSSH2_ERROR_EAGAIN;
        }

        LIBSSH2_FREE(session, packet);
        sftp->fstatvfs_packet = NULL;

        if (rc < 0) {
            sftp->fstatvfs_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "_libssh2_channel_write() failed");
        }
        sftp->fstatvfs_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_EXTENDED_REPLY,
                             sftp->fstatvfs_request_id, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->fstatvfs_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    } else if (data_len < 93) {
        LIBSSH2_FREE(session, data);
        sftp->fstatvfs_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error: short response");
    }

    sftp->fstatvfs_state = libssh2_NB_state_idle;

    st->f_bsize = _libssh2_ntohu64(data + 5);
    st->f_frsize = _libssh2_ntohu64(data + 13);
    st->f_blocks = _libssh2_ntohu64(data + 21);
    st->f_bfree = _libssh2_ntohu64(data + 29);
    st->f_bavail = _libssh2_ntohu64(data + 37);
    st->f_files = _libssh2_ntohu64(data + 45);
    st->f_ffree = _libssh2_ntohu64(data + 53);
    st->f_favail = _libssh2_ntohu64(data + 61);
    st->f_fsid = _libssh2_ntohu64(data + 69);
    flag = _libssh2_ntohu64(data + 77);
    st->f_namemax = _libssh2_ntohu64(data + 85);

    st->f_flag = (flag & SSH_FXE_STATVFS_ST_RDONLY)
                  ? LIBSSH2_SFTP_ST_RDONLY : 0;
    st->f_flag |= (flag & SSH_FXE_STATVFS_ST_NOSUID)
                  ? LIBSSH2_SFTP_ST_NOSUID : 0;

    LIBSSH2_FREE(session, data);
    return 0;
}

/* libssh2_sftp_fstatvfs
 * Get filesystem space and inode utilization (requires fstatvfs@openssh.com
 * support on the server)
 */
LIBSSH2_API int
libssh2_sftp_fstatvfs(LIBSSH2_SFTP_HANDLE *handle, LIBSSH2_SFTP_STATVFS *st)
{
    int rc;
    if(!handle || !st)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, handle->sftp->channel->session, sftp_fstatvfs(handle, st));
    return rc;
}

/*
 * sftp_statvfs
 *
 * Get file system statistics
 */
static int sftp_statvfs(LIBSSH2_SFTP *sftp, const char *path,
                        unsigned int path_len, LIBSSH2_SFTP_STATVFS *st)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    /* 17 = packet_len(4) + packet_type(1) + request_id(4) + ext_len(4)
          + path_len (4) */
    /* 19 = strlen ("statvfs@openssh.com") */
    ssize_t packet_len = path_len + 19 + 17;
    unsigned char *packet, *s, *data;
    int rc;
    unsigned int flag;

    if (sftp->statvfs_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Getting file system statistics of %s", path);
        s = packet = LIBSSH2_ALLOC(session, packet_len);
        if (!packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_EXTENDED "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_EXTENDED;
        sftp->statvfs_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->statvfs_request_id);
        _libssh2_store_str(&s, "statvfs@openssh.com", 19);
        _libssh2_store_str(&s, path, path_len);

        sftp->statvfs_state = libssh2_NB_state_created;
    }
    else {
        packet = sftp->statvfs_packet;
    }

    if (sftp->statvfs_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) packet, packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN || (0 <= rc && rc < packet_len)) {
            sftp->statvfs_packet = packet;
            return LIBSSH2_ERROR_EAGAIN;
        }

        LIBSSH2_FREE(session, packet);
        sftp->statvfs_packet = NULL;

        if (rc < 0) {
            sftp->statvfs_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "_libssh2_channel_write() failed");
        }
        sftp->statvfs_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_EXTENDED_REPLY,
                             sftp->statvfs_request_id, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->statvfs_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    } else if (data_len < 93) {
        LIBSSH2_FREE(session, data);
        sftp->fstatvfs_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error: short response");
    }

    sftp->statvfs_state = libssh2_NB_state_idle;

    st->f_bsize = _libssh2_ntohu64(data + 5);
    st->f_frsize = _libssh2_ntohu64(data + 13);
    st->f_blocks = _libssh2_ntohu64(data + 21);
    st->f_bfree = _libssh2_ntohu64(data + 29);
    st->f_bavail = _libssh2_ntohu64(data + 37);
    st->f_files = _libssh2_ntohu64(data + 45);
    st->f_ffree = _libssh2_ntohu64(data + 53);
    st->f_favail = _libssh2_ntohu64(data + 61);
    st->f_fsid = _libssh2_ntohu64(data + 69);
    flag = _libssh2_ntohu64(data + 77);
    st->f_namemax = _libssh2_ntohu64(data + 85);

    st->f_flag = (flag & SSH_FXE_STATVFS_ST_RDONLY)
                  ? LIBSSH2_SFTP_ST_RDONLY : 0;
    st->f_flag |= (flag & SSH_FXE_STATVFS_ST_NOSUID)
                  ? LIBSSH2_SFTP_ST_NOSUID : 0;

    LIBSSH2_FREE(session, data);
    return 0;
}

/* libssh2_sftp_statvfs_ex
 * Get filesystem space and inode utilization (requires statvfs@openssh.com
 * support on the server)
 */
LIBSSH2_API int
libssh2_sftp_statvfs(LIBSSH2_SFTP *sftp, const char *path,
                     size_t path_len, LIBSSH2_SFTP_STATVFS *st)
{
    int rc;
    if(!sftp || !st)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session, sftp_statvfs(sftp, path, path_len,
                                                          st));
    return rc;
}


/*
 * sftp_mkdir
 *
 * Create an SFTP directory
 */
static int sftp_mkdir(LIBSSH2_SFTP *sftp, const char *path,
                      unsigned int path_len, long mode)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    LIBSSH2_SFTP_ATTRIBUTES attrs = {
        LIBSSH2_SFTP_ATTR_PERMISSIONS, 0, 0, 0, 0, 0, 0
    };
    size_t data_len;
    int retcode;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + path_len(4) */
    ssize_t packet_len = path_len + 13 +
        sftp_attrsize(LIBSSH2_SFTP_ATTR_PERMISSIONS);
    unsigned char *packet, *s, *data;
    int rc;

    if (sftp->mkdir_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP,
                       "Creating directory %s with mode 0%lo", path, mode);
        s = packet = LIBSSH2_ALLOC(session, packet_len);
        if (!packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_MKDIR "
                                  "packet");
        }
        /* Filetype in SFTP 3 and earlier */
        attrs.permissions = mode | LIBSSH2_SFTP_ATTR_PFILETYPE_DIR;

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_MKDIR;
        sftp->mkdir_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->mkdir_request_id);
        _libssh2_store_str(&s, path, path_len);

        s += sftp_attr2bin(s, &attrs);

        sftp->mkdir_state = libssh2_NB_state_created;
    }
    else {
        packet = sftp->mkdir_packet;
    }

    if (sftp->mkdir_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) packet, packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            sftp->mkdir_packet = packet;
            return rc;
        }
        if (packet_len != rc) {
            LIBSSH2_FREE(session, packet);
            sftp->mkdir_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "_libssh2_channel_write() failed");
        }
        LIBSSH2_FREE(session, packet);
        sftp->mkdir_state = libssh2_NB_state_sent;
        sftp->mkdir_packet = NULL;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_STATUS, sftp->mkdir_request_id,
                             &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->mkdir_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->mkdir_state = libssh2_NB_state_idle;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    if (retcode == LIBSSH2_FX_OK) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "OK!");
        return 0;
    } else {
        sftp->last_errno = retcode;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error");
    }
}

/*
 * libssh2_sftp_mkdir_ex
 *
 * Create an SFTP directory
 */
LIBSSH2_API int
libssh2_sftp_mkdir_ex(LIBSSH2_SFTP *sftp, const char *path,
                      unsigned int path_len, long mode)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_mkdir(sftp, path, path_len, mode));
    return rc;
}

/* sftp_rmdir
 * Remove a directory
 */
static int sftp_rmdir(LIBSSH2_SFTP *sftp, const char *path,
                      unsigned int path_len)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    int retcode;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + path_len(4) */
    ssize_t packet_len = path_len + 13;
    unsigned char *s, *data;
    int rc;

    if (sftp->rmdir_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "Removing directory: %s",
                       path);
        s = sftp->rmdir_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->rmdir_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_RMDIR "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);
        *(s++) = SSH_FXP_RMDIR;
        sftp->rmdir_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->rmdir_request_id);
        _libssh2_store_str(&s, path, path_len);

        sftp->rmdir_state = libssh2_NB_state_created;
    }

    if (sftp->rmdir_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->rmdir_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->rmdir_packet);
            sftp->rmdir_packet = NULL;
            sftp->rmdir_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send FXP_RMDIR command");
        }
        LIBSSH2_FREE(session, sftp->rmdir_packet);
        sftp->rmdir_packet = NULL;

        sftp->rmdir_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_require(sftp, SSH_FXP_STATUS,
                             sftp->rmdir_request_id, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->rmdir_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->rmdir_state = libssh2_NB_state_idle;

    retcode = _libssh2_ntohu32(data + 5);
    LIBSSH2_FREE(session, data);

    if (retcode == LIBSSH2_FX_OK) {
        return 0;
    } else {
        sftp->last_errno = retcode;
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "SFTP Protocol Error");
    }
}

/* libssh2_sftp_rmdir_ex
 * Remove a directory
 */
LIBSSH2_API int
libssh2_sftp_rmdir_ex(LIBSSH2_SFTP *sftp, const char *path,
                      unsigned int path_len)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_rmdir(sftp, path, path_len));
    return rc;
}

/* sftp_stat
 * Stat a file or symbolic link
 */
static int sftp_stat(LIBSSH2_SFTP *sftp, const char *path,
                     unsigned int path_len, int stat_type,
                     LIBSSH2_SFTP_ATTRIBUTES * attrs)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + path_len(4) */
    ssize_t packet_len =
        path_len + 13 +
        ((stat_type ==
          LIBSSH2_SFTP_SETSTAT) ? sftp_attrsize(attrs->flags) : 0);
    unsigned char *s, *data;
    static const unsigned char stat_responses[2] =
        { SSH_FXP_ATTRS, SSH_FXP_STATUS };
    int rc;

    if (sftp->stat_state == libssh2_NB_state_idle) {
        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "%s %s",
                       (stat_type == LIBSSH2_SFTP_SETSTAT) ? "Set-statting" :
                       (stat_type ==
                        LIBSSH2_SFTP_LSTAT ? "LStatting" : "Statting"), path);
        s = sftp->stat_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->stat_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for FXP_*STAT "
                                  "packet");
        }

        _libssh2_store_u32(&s, packet_len - 4);

        switch (stat_type) {
        case LIBSSH2_SFTP_SETSTAT:
            *(s++) = SSH_FXP_SETSTAT;
            break;

        case LIBSSH2_SFTP_LSTAT:
            *(s++) = SSH_FXP_LSTAT;
            break;

        case LIBSSH2_SFTP_STAT:
        default:
            *(s++) = SSH_FXP_STAT;
        }
        sftp->stat_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->stat_request_id);
        _libssh2_store_str(&s, path, path_len);

        if (stat_type == LIBSSH2_SFTP_SETSTAT)
            s += sftp_attr2bin(s, attrs);

        sftp->stat_state = libssh2_NB_state_created;
    }

    if (sftp->stat_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->stat_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->stat_packet);
            sftp->stat_packet = NULL;
            sftp->stat_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send STAT/LSTAT/SETSTAT command");
        }
        LIBSSH2_FREE(session, sftp->stat_packet);
        sftp->stat_packet = NULL;

        sftp->stat_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_requirev(sftp, 2, stat_responses,
                              sftp->stat_request_id, &data, &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    } else if (rc) {
        sftp->stat_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->stat_state = libssh2_NB_state_idle;

    if (data[0] == SSH_FXP_STATUS) {
        int retcode;

        retcode = _libssh2_ntohu32(data + 5);
        LIBSSH2_FREE(session, data);
        if (retcode == LIBSSH2_FX_OK) {
            return 0;
        } else {
            sftp->last_errno = retcode;
            return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                  "SFTP Protocol Error");
        }
    }

    memset(attrs, 0, sizeof(LIBSSH2_SFTP_ATTRIBUTES));
    sftp_bin2attr(attrs, data + 5);
    LIBSSH2_FREE(session, data);

    return 0;
}

/* libssh2_sftp_stat_ex
 * Stat a file or symbolic link
 */
LIBSSH2_API int
libssh2_sftp_stat_ex(LIBSSH2_SFTP *sftp, const char *path,
                     unsigned int path_len, int stat_type,
                     LIBSSH2_SFTP_ATTRIBUTES *attrs)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_stat(sftp, path, path_len, stat_type, attrs));
    return rc;
}

/* sftp_symlink
 * Read or set a symlink
 */
static int sftp_symlink(LIBSSH2_SFTP *sftp, const char *path,
                        unsigned int path_len, char *target,
                        unsigned int target_len, int link_type)
{
    LIBSSH2_CHANNEL *channel = sftp->channel;
    LIBSSH2_SESSION *session = channel->session;
    size_t data_len, link_len;
    /* 13 = packet_len(4) + packet_type(1) + request_id(4) + path_len(4) */
    ssize_t packet_len =
        path_len + 13 +
        ((link_type == LIBSSH2_SFTP_SYMLINK) ? (4 + target_len) : 0);
    unsigned char *s, *data;
    static const unsigned char link_responses[2] =
        { SSH_FXP_NAME, SSH_FXP_STATUS };
    int rc;

    if ((sftp->version < 3) && (link_type != LIBSSH2_SFTP_REALPATH)) {
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "Server does not support SYMLINK or READLINK");
    }

    if (sftp->symlink_state == libssh2_NB_state_idle) {
        s = sftp->symlink_packet = LIBSSH2_ALLOC(session, packet_len);
        if (!sftp->symlink_packet) {
            return _libssh2_error(session, LIBSSH2_ERROR_ALLOC,
                                  "Unable to allocate memory for "
                                  "SYMLINK/READLINK/REALPATH packet");
        }

        _libssh2_debug(session, LIBSSH2_TRACE_SFTP, "%s %s on %s",
                       (link_type ==
                        LIBSSH2_SFTP_SYMLINK) ? "Creating" : "Reading",
                       (link_type ==
                        LIBSSH2_SFTP_REALPATH) ? "realpath" : "symlink", path);

        _libssh2_store_u32(&s, packet_len - 4);

        switch (link_type) {
        case LIBSSH2_SFTP_REALPATH:
            *(s++) = SSH_FXP_REALPATH;
            break;

        case LIBSSH2_SFTP_SYMLINK:
            *(s++) = SSH_FXP_SYMLINK;
            break;

        case LIBSSH2_SFTP_READLINK:
        default:
            *(s++) = SSH_FXP_READLINK;
        }
        sftp->symlink_request_id = sftp->request_id++;
        _libssh2_store_u32(&s, sftp->symlink_request_id);
        _libssh2_store_str(&s, path, path_len);

        if (link_type == LIBSSH2_SFTP_SYMLINK)
            _libssh2_store_str(&s, target, target_len);

        sftp->symlink_state = libssh2_NB_state_created;
    }

    if (sftp->symlink_state == libssh2_NB_state_created) {
        rc = _libssh2_channel_write(channel, 0, (char *) sftp->symlink_packet,
                                    packet_len);
        if (rc == LIBSSH2_ERROR_EAGAIN) {
            return rc;
        } else if (packet_len != rc) {
            LIBSSH2_FREE(session, sftp->symlink_packet);
            sftp->symlink_packet = NULL;
            sftp->symlink_state = libssh2_NB_state_idle;
            return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_SEND,
                                  "Unable to send SYMLINK/READLINK command");
        }
        LIBSSH2_FREE(session, sftp->symlink_packet);
        sftp->symlink_packet = NULL;

        sftp->symlink_state = libssh2_NB_state_sent;
    }

    rc = sftp_packet_requirev(sftp, 2, link_responses,
                              sftp->symlink_request_id, &data,
                              &data_len);
    if (rc == LIBSSH2_ERROR_EAGAIN) {
        return rc;
    }
    else if (rc) {
        sftp->symlink_state = libssh2_NB_state_idle;
        return _libssh2_error(session, LIBSSH2_ERROR_SOCKET_TIMEOUT,
                              "Timeout waiting for status message");
    }

    sftp->symlink_state = libssh2_NB_state_idle;

    if (data[0] == SSH_FXP_STATUS) {
        int retcode;

        retcode = _libssh2_ntohu32(data + 5);
        LIBSSH2_FREE(session, data);
        if (retcode == LIBSSH2_FX_OK) {
            return 0;
        } else {
            sftp->last_errno = retcode;
            return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                                  "SFTP Protocol Error");
        }
    }

    if (_libssh2_ntohu32(data + 5) < 1) {
        LIBSSH2_FREE(session, data);
        return _libssh2_error(session, LIBSSH2_ERROR_SFTP_PROTOCOL,
                              "Invalid READLINK/REALPATH response, "
                              "no name entries");
    }

    link_len = _libssh2_ntohu32(data + 9);
    if (link_len >= target_len) {
        link_len = target_len - 1;
    }
    memcpy(target, data + 13, link_len);
    target[link_len] = 0;
    LIBSSH2_FREE(session, data);

    return link_len;
}

/* libssh2_sftp_symlink_ex
 * Read or set a symlink
 */
LIBSSH2_API int
libssh2_sftp_symlink_ex(LIBSSH2_SFTP *sftp, const char *path,
                        unsigned int path_len, char *target,
                        unsigned int target_len, int link_type)
{
    int rc;
    if(!sftp)
        return LIBSSH2_ERROR_BAD_USE;
    BLOCK_ADJUST(rc, sftp->channel->session,
                 sftp_symlink(sftp, path, path_len, target, target_len,
                              link_type));
    return rc;
}

/* libssh2_sftp_last_error
 * Returns the last error code reported by SFTP
 */
LIBSSH2_API unsigned long
libssh2_sftp_last_error(LIBSSH2_SFTP *sftp)
{
    if(!sftp)
       return 0;

    return sftp->last_errno;
}
