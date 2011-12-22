/*
 * Copyright (c) Tony Bybell 2012.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "gconf.h"
#include "wavealloca.h"

#ifdef WAVE_HAVE_GCONF

static GConfClient* client = NULL;

static void
open_callback(GConfClient* client,
                     guint cnxn_id,
                     GConfEntry *entry,
                     gpointer user_data)
{
  if (gconf_entry_get_value (entry) == NULL)
    {
      /* value is unset */
    }
  else
    {
      if (gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
        {
	  fprintf(stderr, "GTKWAVE | RPC Open: '%s'\n", gconf_value_get_string (gconf_entry_get_value (entry)) );

	  deal_with_rpc_open(gconf_value_get_string (gconf_entry_get_value (entry)), NULL);
	  gconf_entry_set_value(entry, NULL);
        }
      else
        {
          /* value is of wrong type */
        }
    }
}


static void remove_client(void)
{
if(client)
	{
	g_object_unref(client);
	}
}


void wave_gconf_init(int argc, char **argv)
{
gconf_init(argc, argv, NULL);
client = gconf_client_get_default();
atexit(remove_client);

gconf_client_add_dir(client,
	WAVE_GCONF_DIR,
        GCONF_CLIENT_PRELOAD_NONE,
        NULL);

gconf_client_notify_add(client, WAVE_GCONF_DIR"/open",
                          open_callback,
                          NULL, /* user data */
                          NULL, NULL);

}


gboolean wave_gconf_client_set_string(const gchar *key, const gchar *val)
{
if(key)
	{
	char *ks = wave_alloca(WAVE_GCONF_DIR_LEN + strlen(key) + 1);

	strcpy(ks, WAVE_GCONF_DIR);
	strcpy(ks+WAVE_GCONF_DIR_LEN, key);
	return(gconf_client_set_string(client, ks, val ? val : "", NULL));
	}

return(FALSE);
}


#else

void wave_gconf_init(int argc, char **argv)
{
}

gboolean wave_gconf_client_set_string(const gchar *key, const gchar *val)
{
return(FALSE);
}

#endif

/*

Examples of RPC manipulation:

gconftool-2 --dump /com.geda.gtkwave
gconftool-2 --dump /com.geda.gtkwave --recursive-unset
gconftool-2 --type string --set /com.geda.gtkwave/open /pub/systema_packed.fst
gconftool-2 --type string --set /com.geda.gtkwave/open `pwd`/`basename -- des.gtkw`

*/
