/*****************************************************************************
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License (as published by the
 *  Free Software Foundation) version 2, dated June 1991.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA or see
 *  <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#if STDC_HEADERS
#include <string.h>
#endif /* STDC_HEADERS */
#include <errno.h>
#include <sys/utsname.h>
#include <inttypes.h>
#include <math.h>

#include "list.h"
#include "hash.h"
#include "error.h"

#include "proc.h"
#include "stat.h"
#include "meminfo.h"
#include "lustre.h"

#include "lmt.h"
#include "router.h"
#include "util.h"
#include "lmtconf.h"

// the fact that we define these things here probably means _itemize_mdt_export_stats should be moved somewhere else
int 
proc_lustre_get_read_and_write_bytes(pctx_t ctx, char *mdt_name, char* node_name, uint64_t* read_bytes, uint64_t* write_bytes);


static int
_itemize_mdt_export_stats (pctx_t ctx, char *mdt_name, char *s, int len)
{
    uint64_t read_bytes, write_bytes;
    int ret = -1;
    List l = list_create ((ListDelF)free);
    ListIterator itr = NULL;
    char *name;

    ret = proc_lustre_ost_exportlist (ctx, mdt_name, &l);

    /* Don't fail if there are no exports -- just skip collection. */
    if ((ret < 0 && errno == ENOENT) || list_count (l) == 0) {
        ret = 0;
        goto done;
    } else if (ret < 0) {
        goto done;
    }

	strcat(s, mdt_name);
	strcat(s, ";");
	char buf[256];
    itr = list_iterator_create (l);
    while ((name = list_next (itr))) {
		proc_lustre_get_read_and_write_bytes(ctx, mdt_name, name, &read_bytes, &write_bytes);
		int n = snprintf(buf, 256, "%s;%" PRIu64 ";%" PRIu64 ";", name, read_bytes, write_bytes);
		if (n + strlen(s) > len) {
        if (lmt_conf_get_proto_debug ())
            msg ("string overflow");
        return -1;
		}
		strcat( s, buf);
    }
done:
    if (itr)
        list_iterator_destroy (itr);
    list_destroy (l);
    return ret;
}


int
_get_ostinfo_string (pctx_t ctx, char *name, char *s, int len)
{
    char *uuid = NULL;
    hash_t stats_hash = NULL;
    int retval = -1;

	printf( "_get_ostinfo_string - dir is %s\n", name);
    if (proc_lustre_uuid (ctx, name, &uuid) < 0) {
        if (lmt_conf_get_proto_debug ())
            err ("error reading lustre %s uuid from proc", name);
        goto done;
    }

	_itemize_mdt_export_stats( ctx, name, s, len );
	printf( "  %s\n", s );

    retval = 0;
done:
    if (uuid)
        free (uuid);
    if (stats_hash)
        hash_destroy (stats_hash);
    return retval;
}

int
lmt_cmt_string_v1 (pctx_t ctx, char *s, int len)
{
    printf( "lmt_cmt_string_v1\n");
//    static uint64_t cpuused = 0, cputot = 0;
    ListIterator itr = NULL;
    List ostlist = NULL;
//    struct utsname uts;
//    double cpupct, mempct;
    int retval = -1;
    char *name;

    if (proc_lustre_ostlist (ctx, &ostlist) < 0)
        goto done;
    if (list_count (ostlist) == 0) {
        errno = 0;
        goto done;
    }
   
    strcpy(s, "1;");
    itr = list_iterator_create (ostlist);
    while ((name = list_next (itr))) {
        int used = strlen (s);
        printf( "OST name: %s\n", name );
       if (_get_ostinfo_string (ctx, name, s + used, len - used) < 0)
            goto done;
    }
    retval = 0;
done:
    if (itr)
        list_iterator_destroy (itr);
    if (ostlist)
        list_destroy (ostlist);
    return retval;
}

int
lmt_cmt_decode_v1 (const char *s, char **ostname, uint64_t *read_bytes,
                      uint64_t *write_bytes)
{
    int retval = -1;
    char *buf = xmalloc (strlen (s) + 1);
    //uint64_t bytes = 0;

	strcpy( buf, s );
    printf("%s\n",s);
    strtok( buf, ";");    
    strdup( strtok(NULL, ";"));  // skip the ossname
	strdup(strtok(NULL, ";"));  // skip the mdtname
	*ostname = strdup(strtok(NULL, ";"));
    if (sscanf( strtok(NULL, ";"), "%"PRIu64, read_bytes ) != 1)
		goto done;
    if (sscanf( strtok(NULL, ";"), "%"PRIu64, write_bytes ) != 1)
		goto done;

    retval = 0;
done:
    printf( "lmt_cmt_decode_v1: %s, %"PRIu64", %"PRIu64"\n", *ostname, *read_bytes, *write_bytes );
    free (buf);
    return retval;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
