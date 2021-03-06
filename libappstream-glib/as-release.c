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

/**
 * SECTION:as-release
 * @short_description: Object representing a single upstream release
 * @include: appstream-glib.h
 * @stability: Stable
 *
 * This object represents a single upstream release, typically a minor update.
 * Releases can contain a localized description of paragraph and list elements
 * and also have a version number and timestamp.
 *
 * Releases can be automatically generated by parsing upstream ChangeLogs or
 * .spec files, or can be populated using AppData files.
 *
 * See also: #AsApp
 */

#include "config.h"

#include <stdlib.h>

#include "as-cleanup.h"
#include "as-checksum-private.h"
#include "as-node-private.h"
#include "as-release-private.h"
#include "as-tag.h"
#include "as-utils-private.h"

typedef struct _AsReleasePrivate	AsReleasePrivate;
struct _AsReleasePrivate
{
	gchar			*version;
	GHashTable		*descriptions;
	guint64			 timestamp;
	GPtrArray		*locations;
	GPtrArray		*checksums;
};

G_DEFINE_TYPE_WITH_PRIVATE (AsRelease, as_release, G_TYPE_OBJECT)

#define GET_PRIVATE(o) (as_release_get_instance_private (o))

/**
 * as_release_finalize:
 **/
static void
as_release_finalize (GObject *object)
{
	AsRelease *release = AS_RELEASE (object);
	AsReleasePrivate *priv = GET_PRIVATE (release);

	g_free (priv->version);
	g_ptr_array_unref (priv->checksums);
	g_ptr_array_unref (priv->locations);
	if (priv->descriptions != NULL)
		g_hash_table_unref (priv->descriptions);

	G_OBJECT_CLASS (as_release_parent_class)->finalize (object);
}

/**
 * as_release_init:
 **/
static void
as_release_init (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	priv->locations = g_ptr_array_new_with_free_func (g_free);
	priv->checksums = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
}

/**
 * as_release_class_init:
 **/
static void
as_release_class_init (AsReleaseClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = as_release_finalize;
}

/**
 * as_release_vercmp:
 * @rel1: a #AsRelease instance.
 * @rel2: a #AsRelease instance.
 *
 * Compares two release.
 *
 * Returns: -1 if rel1 > rel2, +1 if rel1 < rel2, 0 otherwise
 *
 * Since: 0.4.2
 **/
gint
as_release_vercmp (AsRelease *rel1, AsRelease *rel2)
{
	AsReleasePrivate *priv1 = GET_PRIVATE (rel1);
	AsReleasePrivate *priv2 = GET_PRIVATE (rel2);
	gint val;

	/* prefer the version strings */
	val = as_utils_vercmp (priv2->version, priv1->version);
	if (val != G_MAXINT)
		return val;

	/* fall back to the timestamp */
	if (priv1->timestamp > priv2->timestamp)
		return -1;
	if (priv1->timestamp < priv2->timestamp)
		return 1;
	return 0;
}

/**
 * as_release_get_version:
 * @release: a #AsRelease instance.
 *
 * Gets the release version.
 *
 * Returns: string, or %NULL for not set or invalid
 *
 * Since: 0.1.0
 **/
const gchar *
as_release_get_version (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->version;
}

/**
 * as_release_get_locations:
 * @release: a #AsRelease instance.
 *
 * Gets the release locations, typically URLs.
 *
 * Returns: (transfer none) (element-type utf8): list of locations
 *
 * Since: 0.3.5
 **/
GPtrArray *
as_release_get_locations (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->locations;
}

/**
 * as_release_get_location_default:
 * @release: a #AsRelease instance.
 *
 * Gets the default release location, typically a URL.
 *
 * Returns: string, or %NULL for not set or invalid
 *
 * Since: 0.3.5
 **/
const gchar *
as_release_get_location_default (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	if (priv->locations->len == 0)
		return NULL;
	return g_ptr_array_index (priv->locations, 0);
}

/**
 * as_release_get_checksums:
 * @release: a #AsRelease instance.
 *
 * Gets the release checksums.
 *
 * Returns: (transfer none) (element-type AsChecksum): list of checksums
 *
 * Since: 0.4.2
 **/
GPtrArray *
as_release_get_checksums (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->checksums;
}

/**
 * as_release_get_checksum_by_fn:
 * @release: a #AsRelease instance
 * @fn: a file basename
 *
 * Gets the checksum for a release.
 *
 * Returns: (transfer none): an #AsChecksum, or %NULL for not found
 *
 * Since: 0.4.2
 **/
AsChecksum *
as_release_get_checksum_by_fn (AsRelease *release, const gchar *fn)
{
	AsChecksum *checksum;
	AsReleasePrivate *priv = GET_PRIVATE (release);
	guint i;

	for (i = 0; i < priv->checksums->len; i++) {
		checksum = g_ptr_array_index (priv->checksums, i);
		if (g_strcmp0 (fn, as_checksum_get_filename (checksum)) == 0)
			return checksum;
	}
	return NULL;
}

/**
 * as_release_get_checksum_by_target:
 * @release: a #AsRelease instance
 * @target: a #AsChecksumTarget, e.g. %AS_CHECKSUM_TARGET_CONTAINER
 *
 * Gets the checksum for a release.
 *
 * Returns: (transfer none): an #AsChecksum, or %NULL for not found
 *
 * Since: 0.4.2
 **/
AsChecksum *
as_release_get_checksum_by_target (AsRelease *release, AsChecksumTarget target)
{
	AsChecksum *checksum;
	AsReleasePrivate *priv = GET_PRIVATE (release);
	guint i;

	for (i = 0; i < priv->checksums->len; i++) {
		checksum = g_ptr_array_index (priv->checksums, i);
		if (as_checksum_get_target (checksum) == target)
			return checksum;
	}
	return NULL;
}

/**
 * as_release_get_timestamp:
 * @release: a #AsRelease instance.
 *
 * Gets the release timestamp.
 *
 * Returns: timestamp, or 0 for unset
 *
 * Since: 0.1.0
 **/
guint64
as_release_get_timestamp (AsRelease *release)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	return priv->timestamp;
}

/**
 * as_release_get_description:
 * @release: a #AsRelease instance.
 * @locale: the locale, or %NULL. e.g. "en_GB"
 *
 * Gets the release description markup for a given locale.
 *
 * Returns: markup, or %NULL for not set or invalid
 *
 * Since: 0.1.0
 **/
const gchar *
as_release_get_description (AsRelease *release, const gchar *locale)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	if (priv->descriptions == NULL)
		return NULL;
	return as_hash_lookup_by_locale (priv->descriptions, locale);
}

/**
 * as_release_set_version:
 * @release: a #AsRelease instance.
 * @version: the version string.
 *
 * Sets the release version.
 *
 * Since: 0.1.0
 **/
void
as_release_set_version (AsRelease *release, const gchar *version)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	g_free (priv->version);
	priv->version = g_strdup (version);
}

/**
 * as_release_add_location:
 * @release: a #AsRelease instance.
 * @location: the location string.
 *
 * Adds a release location.
 *
 * Since: 0.3.5
 **/
void
as_release_add_location (AsRelease *release, const gchar *location)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);

	/* deduplicate */
	if (as_ptr_array_find_string (priv->locations, location))
		return;

	g_ptr_array_add (priv->locations, g_strdup (location));
}

/**
 * as_release_add_checksum:
 * @release: a #AsRelease instance.
 * @checksum: a #AsChecksum instance.
 *
 * Adds a release checksum.
 *
 * Since: 0.4.2
 **/
void
as_release_add_checksum (AsRelease *release, AsChecksum *checksum)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	g_ptr_array_add (priv->checksums, g_object_ref (checksum));
}

/**
 * as_release_set_timestamp:
 * @release: a #AsRelease instance.
 * @timestamp: the timestamp value.
 *
 * Sets the release timestamp.
 *
 * Since: 0.1.0
 **/
void
as_release_set_timestamp (AsRelease *release, guint64 timestamp)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	priv->timestamp = timestamp;
}

/**
 * as_release_set_description:
 * @release: a #AsRelease instance.
 * @locale: the locale, or %NULL. e.g. "en_GB"
 * @description: the description markup.
 *
 * Sets the description release markup.
 *
 * Since: 0.1.0
 **/
void
as_release_set_description (AsRelease *release,
			    const gchar *locale,
			    const gchar *description)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	if (locale == NULL)
		locale = "C";
	if (priv->descriptions == NULL) {
		priv->descriptions = g_hash_table_new_full (g_str_hash,
							    g_str_equal,
							    g_free,
							    g_free);
	}
	g_hash_table_insert (priv->descriptions,
			     g_strdup (locale),
			     g_strdup (description));
}

/**
 * as_release_node_insert: (skip)
 * @release: a #AsRelease instance.
 * @parent: the parent #GNode to use..
 * @ctx: the #AsNodeContext
 *
 * Inserts the release into the DOM tree.
 *
 * Returns: (transfer none): A populated #GNode
 *
 * Since: 0.1.1
 **/
GNode *
as_release_node_insert (AsRelease *release, GNode *parent, AsNodeContext *ctx)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	AsChecksum *checksum;
	GNode *n;

	n = as_node_insert (parent, "release", NULL,
			    AS_NODE_INSERT_FLAG_NONE,
			    NULL);
	if (priv->timestamp > 0) {
		_cleanup_free_ gchar *timestamp_str = NULL;
		timestamp_str = g_strdup_printf ("%" G_GUINT64_FORMAT,
						 priv->timestamp);
		as_node_add_attribute (n, "timestamp", timestamp_str);
	}
	if (priv->version != NULL)
		as_node_add_attribute (n, "version", priv->version);
	if (as_node_context_get_version (ctx) >= 0.9) {
		const gchar *tmp;
		guint i;
		for (i = 0; i < priv->locations->len; i++) {
			tmp = g_ptr_array_index (priv->locations, i);
			as_node_insert (n, "location", tmp,
					AS_NODE_INSERT_FLAG_NONE, NULL);
		}
		for (i = 0; i < priv->checksums->len; i++) {
			checksum = g_ptr_array_index (priv->checksums, i);
			as_checksum_node_insert (checksum, n, ctx);
		}
	}
	if (priv->descriptions != NULL && as_node_context_get_version (ctx) >= 0.6) {
		as_node_insert_localized (n, "description", priv->descriptions,
					  AS_NODE_INSERT_FLAG_PRE_ESCAPED |
					  AS_NODE_INSERT_FLAG_DEDUPE_LANG);
	}
	return n;
}

/**
 * as_release_node_parse:
 * @release: a #AsRelease instance.
 * @node: a #GNode.
 * @ctx: a #AsNodeContext.
 * @error: A #GError or %NULL.
 *
 * Populates the object from a DOM node.
 *
 * Returns: %TRUE for success
 *
 * Since: 0.1.0
 **/
gboolean
as_release_node_parse (AsRelease *release, GNode *node,
		       AsNodeContext *ctx, GError **error)
{
	AsReleasePrivate *priv = GET_PRIVATE (release);
	GNode *n;
	const gchar *tmp;
	gchar *taken;

	tmp = as_node_get_attribute (node, "timestamp");
	if (tmp != NULL)
		as_release_set_timestamp (release, atol (tmp));
	taken = as_node_take_attribute (node, "version");
	if (taken != NULL) {
		g_free (priv->version);
		priv->version = taken;
	}

	/* get optional locations */
	g_ptr_array_set_size (priv->locations, 0);
	for (n = node->children; n != NULL; n = n->next) {
		if (as_node_get_tag (n) != AS_TAG_LOCATION)
			continue;
		g_ptr_array_add (priv->locations, as_node_take_data (n));
	}

	/* get optional checksums */
	for (n = node->children; n != NULL; n = n->next) {
		_cleanup_object_unref_ AsChecksum *csum = NULL;
		if (as_node_get_tag (n) != AS_TAG_CHECKSUM)
			continue;
		csum = as_checksum_new ();
		if (!as_checksum_node_parse (csum, n, ctx, error))
			return FALSE;
		as_release_add_checksum (release, csum);
	}

	/* AppStream: multiple <description> tags */
	if (as_node_context_get_source_kind (ctx) == AS_APP_SOURCE_KIND_APPSTREAM) {
		for (n = node->children; n != NULL; n = n->next) {
			_cleanup_string_free_ GString *xml = NULL;
			if (as_node_get_tag (n) != AS_TAG_DESCRIPTION)
				continue;
			if (n->children == NULL)
				continue;
			xml = as_node_to_xml (n->children,
					      AS_NODE_TO_XML_FLAG_INCLUDE_SIBLINGS);
			if (xml == NULL)
				continue;
			as_release_set_description (release,
						    as_node_get_attribute (n, "xml:lang"),
						    xml->str);
		}

	/* AppData: mutliple languages encoded in one <description> tag */
	} else {
		n = as_node_find (node, "description");
		if (n != NULL) {
			if (priv->descriptions != NULL)
				g_hash_table_unref (priv->descriptions);
			priv->descriptions = as_node_get_localized_unwrap (n, error);
			if (priv->descriptions == NULL)
				return FALSE;
		}
	}

	return TRUE;
}

/**
 * as_release_new:
 *
 * Creates a new #AsRelease.
 *
 * Returns: (transfer full): a #AsRelease
 *
 * Since: 0.1.0
 **/
AsRelease *
as_release_new (void)
{
	AsRelease *release;
	release = g_object_new (AS_TYPE_RELEASE, NULL);
	return AS_RELEASE (release);
}
