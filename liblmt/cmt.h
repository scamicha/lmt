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


struct client_io
{
    char ostname[256];
    char nodename[256];

    uint64_t read_bytes;
    uint64_t write_bytes;
};


int lmt_cmt_string_v1 (pctx_t ctx, char *s, int len);

int lmt_cmt_decode_v1(const char *s, List *lp);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
