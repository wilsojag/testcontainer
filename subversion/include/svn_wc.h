/*
 * svn_wc.h :  public interface for the Subversion Working Copy Library
 *
 * ================================================================
 * Copyright (c) 2000 CollabNet.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. The end-user documentation included with the redistribution, if
 * any, must include the following acknowlegement: "This product includes
 * software developed by CollabNet (http://www.Collab.Net)."
 * Alternately, this acknowlegement may appear in the software itself, if
 * and wherever such third-party acknowlegements normally appear.
 * 
 * 4. The hosted project names must not be used to endorse or promote
 * products derived from this software without prior written
 * permission. For written permission, please contact info@collab.net.
 * 
 * 5. Products derived from this software may not use the "Tigris" name
 * nor may "Tigris" appear in their names without prior written
 * permission of CollabNet.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL COLLABNET OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by many
 * individuals on behalf of CollabNet.
 */



/* ==================================================================== */

/* 
 * Requires:  
 *            A working copy
 * 
 * Provides: 
 *            - Ability to manipulate working copy's versioned data.
 *            - Ability to manipulate working copy's administrative files.
 *
 * Used By:   
 *            Clients.
 */

#ifndef SVN_WC_H
#define SVN_WC_H

#include <apr_tables.h>
#include "svn_types.h"
#include "svn_string.h"
#include "svn_delta.h"
#include "svn_error.h"



/* Where you see an argument like
 * 
 *   apr_array_header_t *paths
 *
 * it means an array of (svn_string_t *) types, each one of which is
 * a file or directory path.  This is so we can do atomic operations
 * on any random set of files and directories.
 */

/* kff todo: these do nothing and return SVN_NO_ERROR right now. */
svn_error_t *svn_wc_rename (svn_string_t *src, svn_string_t *dst);
svn_error_t *svn_wc_copy   (svn_string_t *src, svn_string_t *dst);
svn_error_t *svn_wc_add    (apr_array_header_t *paths);
svn_error_t *svn_wc_delete (apr_array_header_t *paths);




/* Do a depth-first crawl of the local changes in a working copy,
   beginning at ROOT_DIRECTORY.  Push synthesized xml (representing a
   coherent tree-delta) at XML_PARSER.

   Presumably, the client library will grab an svn_delta_edit_fns_t
   from libsvn_ra, build an svn_xml_parser_t around it, and then pass
   the parser to this routine.  This is how local changes in the
   working copy are ultimately translated into network requests.  */
svn_error_t * svn_wc_crawl_local_mods (svn_string_t *root_directory,
                                       svn_delta_edit_fns_t *edit_fns,
                                       apr_pool_t *pool);



/*
 * Return an editor for effecting changes in a working copy, including
 * creating the working copy (i.e., update and checkout).
 * 
 * If DEST is non-null, the editor will ensure its existence as a
 * directory (i.e., it is created if it does not already exist), and
 * it will be prepended to every path the delta causes to be touched.
 *
 * It is the caller's job to make sure that DEST is not some other
 * working copy, or that if it is, it will not be damaged by the
 * application of this delta.  The wc library tries to detect
 * such a case and do as little damage as possible, but makes no
 * promises.
 *
 * REPOS is the repository string to be recorded in this working
 * copy.
 *
 * VERSION is the repository version that results from this set of
 * changes.
 *
 * EDITOR, EDIT_BATON, and DIR_BATON are all returned by reference,
 * and the latter two must be used as parameters to editor functions.
 *
 * kff todo: Actually, REPOS is one of several possible non-delta-ish
 * things that may be needed by a editor when creating new
 * administrative subdirs.  Other things might be username and/or auth
 * info, which aren't necessarily included in the repository string.
 * Thinking more on this question...
 */
svn_error_t *svn_wc_get_update_editor (svn_string_t *dest,
                                       svn_string_t *repos,
                                       svn_vernum_t version,
                                       const svn_delta_edit_fns_t **editor,
                                       void **edit_baton,
                                       void **dir_baton,
                                       apr_pool_t *pool);


#if 0
/* kff: Will have to think about the interface here a bit more. */

/* GJS: the function will look something like this:
 *
 * svn_wc_commit(source, commit_editor, commit_edit_baton, dir_baton, pool)
 *
 * The Client Library will fetch the commit_editor (& baton) from RA.
 * Source is something that describes the files/dirs (and recursion) to
 * commit. Internally, WC will edit the local dirs and push changes into
 * the commit editor.
 */

svn_error_t *svn_wc_make_skelta (void *delta_src,
                                 svn_delta_write_fn_t *delta_stream_writer,
                                 apr_array_header_t *paths);


svn_error_t *svn_wc_make_delta (void *delta_src,
                                svn_delta_write_fn_t *delta_stream_writer,
                                apr_array_header_t *paths);
#endif /* 0 */


/* A word about the implementation of working copy property storage:
 *
 * Since properties are key/val pairs, you'd think we store them in
 * some sort of Berkeley DB-ish format, and even store pending changes
 * to them that way too.
 *
 * However, we already have libsvn_subr/hashdump.c working, and it
 * uses a human-readable format.  That will be very handy when we're
 * debugging, and presumably we will not be dealing with any huge
 * properties or property lists initially.  Therefore, we will
 * continue to use hashdump as the internal mechanism for storing and
 * reading from property lists, but note that the interface here is
 * _not_ dependent on that.  We can swap in a DB-based implementation
 * at any time and users of this library will never know the
 * difference.
 */

/* kff todo: does nothing and returns SVN_NO_ERROR, currently. */
/* Return local value of PROPNAME for the file or directory PATH. */
svn_error_t *svn_wc_get_path_prop (svn_string_t **value,
                                   svn_string_t *propname,
                                   svn_string_t *path);

/* kff todo: does nothing and returns SVN_NO_ERROR, currently. */
/* Return local value of PROPNAME for the directory entry PATH. */
svn_error_t *svn_wc_get_dirent_prop (svn_string_t **value,
                                     svn_string_t *propname,
                                     svn_string_t *path);

#endif  /* SVN_WC_H */

/* --------------------------------------------------------------
 * local variables:
 * eval: (load-file "../svn-dev.el")
 * end: 
 */
