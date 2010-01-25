/*
 * vinagre-vnc-tunnel.c
 * VNC SSH Tunneling for Vinagre
 * This file is part of vinagre
 *
 * Copyright (C) 2009 - Jonh Wendell <wendell@bani.com.br>
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

#include <config.h>
#include <netinet/in.h>
#include <unistd.h>
#include <glib/gi18n.h>

#include <vinagre/vinagre-ssh.h>
#include "vinagre-vnc-tunnel.h"

#define TUNNEL_PORT_OFFSET 5500

static int
find_free_port (void)
{
  int sock, port;
  struct sockaddr_in6 addr;

  memset (&addr, 0, sizeof (addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;

  sock = socket (AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0)
    return 0;

  for (port = TUNNEL_PORT_OFFSET + 99; port > TUNNEL_PORT_OFFSET; port--)
    {
      addr.sin6_port = htons (port);
      if (bind (sock, (struct sockaddr *)&addr, sizeof (addr)) == 0)
	{
	  close (sock);
	  return port;
	}
    }

  close (sock);
  return 0;
}

gboolean
vinagre_vnc_tunnel_create (gchar **original_host, gchar **original_port, gchar *gateway, GError **error)
{
  int local_port;
  gchar **tunnel_str, **command_str;

  local_port = find_free_port ();
  if (local_port == 0)
    {
      g_set_error (error,
		   VINAGRE_VNC_TUNNEL_ERROR,
		   VINAGRE_VNC_TUNNEL_ERROR_NO_FREE_PORT,
		   _("Unable to find a free TCP port"));
      return FALSE;
    }

  tunnel_str = g_new (gchar *, 4);
  tunnel_str[0] = g_strdup ("-f");
  tunnel_str[1] = g_strdup ("-L");
  tunnel_str[2] = g_strdup_printf ("%d:%s:%s",
				   local_port,
				   *original_host,
				   *original_port);
  tunnel_str[3] = NULL;

  command_str = g_new (gchar *, 5);
  command_str[0] = g_strdup ("echo");
  command_str[1] = g_strdup_printf ("%s;", VINAGRE_SSH_CHECK);
  command_str[2] = g_strdup ("sleep");
  command_str[3] = g_strdup ("15");
  command_str[4] = NULL;

  if (!vinagre_ssh_connect (gateway,
			    22,
			    NULL,
			    tunnel_str,
			    command_str,
			    NULL,
			    error))
    {
      g_strfreev (tunnel_str);
      g_strfreev (command_str);
      return FALSE;
    }

  g_strfreev (tunnel_str);
  g_strfreev (command_str);
  g_free (*original_host);
  *original_host = g_strdup ("localhost");
  g_free (*original_port);
  *original_port = g_strdup_printf ("%d", local_port);

  return TRUE;
}

GQuark 
vinagre_vnc_tunnel_error_quark (void)
{
  static GQuark quark = 0;

  if (!quark)
    quark = g_quark_from_string ("vinagre_vnc_tunnel_error");

  return quark;
}

/* vim: set ts=8: */