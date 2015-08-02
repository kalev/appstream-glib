/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2014-2015 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#if !defined (__APPSTREAM_GLIB_H) && !defined (AS_COMPILATION)
#error "Only <appstream-glib.h> can be included directly."
#endif

#ifndef __AS_CHECKSUM_H
#define __AS_CHECKSUM_H

#include <glib-object.h>

#define AS_TYPE_CHECKSUM		(as_checksum_get_type())
#define AS_CHECKSUM(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), AS_TYPE_CHECKSUM, AsChecksum))
#define AS_CHECKSUM_CLASS(cls)		(G_TYPE_CHECK_CLASS_CAST((cls), AS_TYPE_CHECKSUM, AsChecksumClass))
#define AS_IS_CHECKSUM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), AS_TYPE_CHECKSUM))
#define AS_IS_CHECKSUM_CLASS(cls)	(G_TYPE_CHECK_CLASS_TYPE((cls), AS_TYPE_CHECKSUM))
#define AS_CHECKSUM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), AS_TYPE_CHECKSUM, AsChecksumClass))

G_BEGIN_DECLS

typedef struct _AsChecksum	AsChecksum;
typedef struct _AsChecksumClass	AsChecksumClass;

struct _AsChecksum
{
	GObject			parent;
};

struct _AsChecksumClass
{
	GObjectClass		parent_class;
	/*< private >*/
	void (*_as_reserved1)	(void);
	void (*_as_reserved2)	(void);
	void (*_as_reserved3)	(void);
	void (*_as_reserved4)	(void);
	void (*_as_reserved5)	(void);
	void (*_as_reserved6)	(void);
	void (*_as_reserved7)	(void);
	void (*_as_reserved8)	(void);
};

GType		 as_checksum_get_type		(void);
AsChecksum	*as_checksum_new		(void);

/* getters */
const gchar	*as_checksum_get_filename	(AsChecksum	*checksum);
const gchar	*as_checksum_get_value		(AsChecksum	*checksum);
GChecksumType	 as_checksum_get_kind		(AsChecksum	*checksum);

/* setters */
void		 as_checksum_set_filename	(AsChecksum	*checksum,
						 const gchar	*filename,
						 gssize		 filename_len);
void		 as_checksum_set_value		(AsChecksum	*checksum,
						 const gchar	*value);
void		 as_checksum_set_kind		(AsChecksum	*checksum,
						 GChecksumType	 kind);

G_END_DECLS

#endif /* __AS_CHECKSUM_H */
