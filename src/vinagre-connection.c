/*
 * vinagre-connection.c
 * This file is part of vinagre
 *
 * Copyright (C) 2007,2008 - Jonh Wendell <wendell@bani.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtkicontheme.h>

#include "vinagre-connection.h"
#include "vinagre-enums.h"
#include "vinagre-bookmarks.h"

struct _VinagreConnectionPrivate
{
  VinagreConnectionProtocol protocol;
  gchar *host;
  gint   port;
  gchar *name;
  gchar *password;
  gchar *desktop_name;
  gboolean view_only;
  gboolean scaling;
  gboolean fullscreen;
};

enum
{
  PROP_0,
  PROP_PROTOCOL,
  PROP_HOST,
  PROP_PORT,
  PROP_NAME,
  PROP_PASSWORD,
  PROP_DESKTOP_NAME,
  PROP_BEST_NAME,
  PROP_ICON,
  PROP_VIEW_ONLY,
  PROP_SCALING,
  PROP_FULLSCREEN
};

#define VINAGRE_CONNECTION_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate))
G_DEFINE_TYPE (VinagreConnection, vinagre_connection, G_TYPE_OBJECT);

static void
vinagre_connection_init (VinagreConnection *conn)
{
  conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, VINAGRE_TYPE_CONNECTION, VinagreConnectionPrivate);

  conn->priv->protocol = VINAGRE_CONNECTION_PROTOCOL_VNC;
  conn->priv->host = NULL;
  conn->priv->port = 0;
  conn->priv->password = NULL;
  conn->priv->name = NULL;
  conn->priv->desktop_name = NULL;
  conn->priv->view_only = FALSE;
  conn->priv->scaling = FALSE;
  conn->priv->fullscreen = FALSE;
}

static void
vinagre_connection_finalize (GObject *object)
{
  VinagreConnection *conn = VINAGRE_CONNECTION (object);

  if (conn->priv->host)
    g_free (conn->priv->host);
  conn->priv->host = NULL;

  if (conn->priv->password)
    g_free (conn->priv->password);
  conn->priv->password = NULL;

  if (conn->priv->name)
    g_free (conn->priv->name);
  conn->priv->name = NULL;

  if (conn->priv->desktop_name)
    g_free (conn->priv->desktop_name);
  conn->priv->desktop_name = NULL;

  G_OBJECT_CLASS (vinagre_connection_parent_class)->finalize (object);
}

static void
vinagre_connection_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_CONNECTION (object));

  conn = VINAGRE_CONNECTION (object);

  switch (prop_id)
    {
      case PROP_PROTOCOL:
	vinagre_connection_set_protocol (conn, g_value_get_enum (value));
	break;

      case PROP_HOST:
	vinagre_connection_set_host (conn, g_value_get_string (value));
	break;

      case PROP_PORT:
	vinagre_connection_set_port (conn, g_value_get_int (value));
	break;

      case PROP_PASSWORD:
	vinagre_connection_set_password (conn, g_value_get_string (value));
	break;

      case PROP_NAME:
	vinagre_connection_set_name (conn, g_value_get_string (value));
	break;

      case PROP_DESKTOP_NAME:
	vinagre_connection_set_desktop_name (conn, g_value_get_string (value));
	break;

      case PROP_VIEW_ONLY:
	vinagre_connection_set_view_only (conn, g_value_get_boolean (value));
	break;

      case PROP_SCALING:
	vinagre_connection_set_scaling (conn, g_value_get_boolean (value));
	break;

      case PROP_FULLSCREEN:
	vinagre_connection_set_fullscreen (conn, g_value_get_boolean (value));
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_connection_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  VinagreConnection *conn;

  g_return_if_fail (VINAGRE_IS_CONNECTION (object));

  conn = VINAGRE_CONNECTION (object);


  switch (prop_id)
    {
      case PROP_PROTOCOL:
	g_value_set_enum (value, conn->priv->protocol);
	break;

      case PROP_HOST:
	g_value_set_string (value, conn->priv->host);
	break;

      case PROP_PORT:
	g_value_set_int (value, conn->priv->port);
	break;

      case PROP_PASSWORD:
	g_value_set_string (value, conn->priv->password);
	break;

      case PROP_NAME:
	g_value_set_string (value, conn->priv->name);
	break;

      case PROP_DESKTOP_NAME:
	g_value_set_string (value, conn->priv->desktop_name);
	break;

      case PROP_BEST_NAME:
	g_value_set_string (value, vinagre_connection_get_best_name (conn));
	break;

      case PROP_ICON:
	g_value_set_object (value, vinagre_connection_get_icon (conn));
	break;

      case PROP_VIEW_ONLY:
	g_value_set_boolean (value, conn->priv->view_only);
	break;

      case PROP_SCALING:
	g_value_set_boolean (value, conn->priv->scaling);
	break;

      case PROP_FULLSCREEN:
	g_value_set_boolean (value, conn->priv->fullscreen);
	break;

      default:
	G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	break;
    }
}

static void
vinagre_connection_class_init (VinagreConnectionClass *klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS (klass);
  GObjectClass* parent_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (VinagreConnectionPrivate));

  object_class->finalize = vinagre_connection_finalize;
  object_class->set_property = vinagre_connection_set_property;
  object_class->get_property = vinagre_connection_get_property;

  g_object_class_install_property (object_class,
                                   PROP_PROTOCOL,
                                   g_param_spec_enum ("protocol",
                                                      "protocol",
	                                              "connection protocol",
                                                      VINAGRE_TYPE_CONNECTION_PROTOCOL,
	                                              VINAGRE_CONNECTION_PROTOCOL_VNC,
	                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_NICK |
                                                      G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_HOST,
                                   g_param_spec_string ("host",
                                                        "hostname",
	                                                "hostname or ip address of this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_PORT,
                                   g_param_spec_int ("port",
                                                     "port",
	                                              "tcp/ip port of this connection",
                                                      0,
                                                      G_MAXINT,
                                                      5900,
	                                              G_PARAM_READWRITE |
                                                      G_PARAM_CONSTRUCT |
                                                      G_PARAM_STATIC_NICK |
                                                      G_PARAM_STATIC_NAME |
                                                      G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_PASSWORD,
                                   g_param_spec_string ("password",
                                                        "password",
	                                                "password (if any) necessary for complete this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "connection name",
	                                                "friendly name for this connection",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_DESKTOP_NAME,
                                   g_param_spec_string ("desktop-name",
                                                        "desktop-name",
	                                                "name of this connection as reported by the server",
                                                        NULL,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_BEST_NAME,
                                   g_param_spec_string ("best-name",
                                                        "best-name",
	                                                "preferred name for this connection",
                                                        NULL,
	                                                G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

  g_object_class_install_property (object_class,
                                   PROP_ICON,
                                   g_param_spec_object ("icon",
                                                        "icon",
	                                                "icon of this connection",
                                                        GDK_TYPE_PIXBUF,
	                                                G_PARAM_READABLE |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_VIEW_ONLY,
                                   g_param_spec_boolean ("view-only",
                                                        "View-only connection",
	                                                "Whether this connection is a view-only one",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_SCALING,
                                   g_param_spec_boolean ("scaling",
                                                        "Use scaling",
	                                                "Whether to use scaling on this connection",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));
  g_object_class_install_property (object_class,
                                   PROP_FULLSCREEN,
                                   g_param_spec_boolean ("fullscreen",
                                                        "Full screen connection",
	                                                "Whether this connection is a view-only one",
                                                        FALSE,
	                                                G_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT |
                                                        G_PARAM_STATIC_NICK |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB));

}

VinagreConnection *
vinagre_connection_new ()
{
  return g_object_new (VINAGRE_TYPE_CONNECTION, NULL);
}

void
vinagre_connection_set_protocol (VinagreConnection *conn,
			         VinagreConnectionProtocol protocol)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->protocol = protocol;
}
VinagreConnectionProtocol
vinagre_connection_get_protocol (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn),
                        VINAGRE_CONNECTION_PROTOCOL_INVALID);

  return conn->priv->protocol;
}

void
vinagre_connection_set_host (VinagreConnection *conn,
			     const gchar *host)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  if (conn->priv->host)
    g_free (conn->priv->host);
  conn->priv->host = g_strdup (host);
}
const gchar *
vinagre_connection_get_host (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->host;
}

void
vinagre_connection_set_port (VinagreConnection *conn,
			     gint port)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->port = port;
}
gint
vinagre_connection_get_port (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), 0);

  return conn->priv->port;
}

void
vinagre_connection_set_password (VinagreConnection *conn,
			         const gchar *password)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  if (conn->priv->password)
    g_free (conn->priv->password);
  conn->priv->password = g_strdup (password);
}
const gchar *
vinagre_connection_get_password (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->password;
}

void
vinagre_connection_set_name (VinagreConnection *conn,
			     const gchar *name)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  if (conn->priv->name)
    g_free (conn->priv->name);
  conn->priv->name = g_strdup (name);
}
const gchar *
vinagre_connection_get_name (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->name;
}

void
vinagre_connection_set_desktop_name (VinagreConnection *conn,
			             const gchar *desktop_name)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  if (conn->priv->desktop_name)
    g_free (conn->priv->desktop_name);
  conn->priv->desktop_name = g_strdup (desktop_name);
}
const gchar *
vinagre_connection_get_desktop_name (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  return conn->priv->desktop_name;
}

gchar *
vinagre_connection_get_best_name (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  if (conn->priv->name)
    return g_strdup (conn->priv->name);

  if (conn->priv->desktop_name)
    return g_strdup (conn->priv->desktop_name);

  if (conn->priv->host)
        return g_strdup_printf ("%s:%d", conn->priv->host, conn->priv->port);

  return NULL;
}

VinagreConnection *
vinagre_connection_clone (VinagreConnection *conn)
{
  VinagreConnection *new_conn;

  new_conn = vinagre_connection_new ();

  vinagre_connection_set_host (new_conn, vinagre_connection_get_host (conn));
  vinagre_connection_set_port (new_conn, vinagre_connection_get_port (conn));
  vinagre_connection_set_password (new_conn, vinagre_connection_get_password (conn));
  vinagre_connection_set_name (new_conn, vinagre_connection_get_name (conn));
  vinagre_connection_set_desktop_name (new_conn, vinagre_connection_get_desktop_name (conn));

  return new_conn;
}

VinagreConnection *
vinagre_connection_new_from_string (const gchar *uri, gchar **error_msg)
{
  VinagreConnection *conn;
  gchar **server, **url;
  gint    port;
  gchar  *host;

  *error_msg = NULL;

  url = g_strsplit (uri, "://", 2);
  if (g_strv_length (url) == 2)
    {
      if (g_strcmp0 (url[0], "vnc"))
	{
	  *error_msg = g_strdup_printf (_("The protocol %s is not supported."),
					url[0]);
	  g_strfreev (url);
	  return NULL;
	}
      host = url[1];
    }
  else
    host = (gchar *) uri;

  if (g_strrstr (host, "::") != NULL)
    {
      server = g_strsplit (host, "::", 2);
      port = server[1] ? atoi (server[1]) : 5900;
    }
  else
    {
      server = g_strsplit (host, ":", 2);
      port = server[1] ? atoi (server[1]) : 5900;

      if (port < 1024)
        port += 5900;
    }

  host = server[0];

  conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (),
                                   host,
                                   port);
  if (!conn)
    {
      conn = vinagre_connection_new ();
      vinagre_connection_set_host (conn, host);
      vinagre_connection_set_port (conn, port);
    }

  g_strfreev (server);
  g_strfreev (url);

  return conn;
}

VinagreConnection *
vinagre_connection_new_from_file (const gchar *uri, gchar **error_msg)
{
  GKeyFile          *file;
  GError            *error = NULL;
  gboolean           loaded;
  VinagreConnection *conn = NULL;
  gchar             *host = NULL;
  gint               port;
  int                file_size;
  char              *data = NULL;
  GFile             *file_a;

  *error_msg = NULL;

  file_a = g_file_new_for_commandline_arg (uri);
  loaded = g_file_load_contents (file_a,
				 NULL,
				 &data,
				 &file_size,
				 NULL,
				 &error);
  if (!loaded)
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}

      if (data)
	g_free (data);

      g_object_unref (file_a);
      return NULL;
    }

  file = g_key_file_new ();
  loaded = g_key_file_load_from_data (file,
				      data,
				      file_size,
				      G_KEY_FILE_NONE,
				      &error);
  if (loaded)
    {
      host = g_key_file_get_string (file, "connection", "host", NULL);
      port = g_key_file_get_integer (file, "connection", "port", NULL);
      if (host)
	{
	  conn = vinagre_bookmarks_exists (vinagre_bookmarks_get_default (),
                                           host,
                                           port);
	  if (!conn)
	    {
	      conn = vinagre_connection_new ();
	      vinagre_connection_set_host (conn, host);
	      vinagre_connection_set_port (conn, port);
	    }
	  g_free (host);
	}
    }
  else
    {
      if (error)
	{
	  *error_msg = g_strdup (error->message);
	  g_error_free (error);
	}
    }

  if (data)
    g_free (data);

  g_key_file_free (file);
  g_object_unref (file_a);

  return conn;
}

GdkPixbuf *
vinagre_connection_get_icon (VinagreConnection *conn)
{
  GdkPixbuf         *pixbuf;
  GtkIconTheme      *icon_theme;

  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), NULL);

  icon_theme = gtk_icon_theme_get_default ();
  pixbuf = gtk_icon_theme_load_icon (icon_theme,
				     "application-x-vnc",
				     16,
				     0,
				     NULL);

  return pixbuf;
}

void
vinagre_connection_set_view_only (VinagreConnection *conn,
				  gboolean value)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->view_only = value;
}
gboolean
vinagre_connection_get_view_only (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  return conn->priv->view_only;
}

void
vinagre_connection_set_scaling (VinagreConnection *conn,
				gboolean value)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->scaling = value;
}
gboolean
vinagre_connection_get_scaling (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  return conn->priv->scaling;
}

void
vinagre_connection_set_fullscreen (VinagreConnection *conn,
				  gboolean value)
{
  g_return_if_fail (VINAGRE_IS_CONNECTION (conn));

  conn->priv->fullscreen = value;
}
gboolean
vinagre_connection_get_fullscreen (VinagreConnection *conn)
{
  g_return_val_if_fail (VINAGRE_IS_CONNECTION (conn), FALSE);

  return conn->priv->fullscreen;
}

/* vim: ts=8 */
