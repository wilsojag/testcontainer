#include "private/svn_diff_private.h"
#include "private/svn_subr_private.h"
#include "private/svn_io_private.h"
/* Calculate the repository relative path of DIFF_RELPATH, using RA_SESSION
 * and WC_CTX, and return the result in *REPOS_RELPATH.
 * ORIG_TARGET is the related original target passed to the diff command,
 * ANCHOR is the local path where the diff editor is anchored.
make_repos_relpath(const char **repos_relpath,
                   const char *diff_relpath,
                   const char *orig_target,
                   svn_ra_session_t *ra_session,
                   svn_wc_context_t *wc_ctx,
                   const char *anchor,
                   apr_pool_t *result_pool,
                   apr_pool_t *scratch_pool)
  const char *orig_repos_relpath = NULL;
  if (! ra_session
      || (anchor && !svn_path_is_url(orig_target)))
      svn_error_t *err;
      SVN_ERR(svn_dirent_get_absolute(&local_abspath,
                                      svn_dirent_join(anchor, diff_relpath,
                                                      scratch_pool),
                                      scratch_pool));

      err = svn_wc__node_get_repos_info(NULL, repos_relpath, NULL, NULL,
                                        wc_ctx, local_abspath,
                                        result_pool, scratch_pool);

      if (!ra_session
          || ! err
          || (err && err->apr_err != SVN_ERR_WC_PATH_NOT_FOUND))
        {
           return svn_error_trace(err);
        }
      /* The path represents a local working copy path, but does not
         exist. Fall through to calculate an in-repository location
         based on the ra session */
      /* ### Maybe we should use the nearest existing ancestor instead? */
      svn_error_clear(err);
  {
    const char *url;
    const char *repos_root_url;

    /* Would be nice if the RA layer could just provide the parent
       repos_relpath of the ra session */
      SVN_ERR(svn_ra_get_session_url(ra_session, &url, scratch_pool));

      SVN_ERR(svn_ra_get_repos_root2(ra_session, &repos_root_url,
                                     scratch_pool));
      orig_repos_relpath = svn_uri_skip_ancestor(repos_root_url, url,
                                                 scratch_pool);

      *repos_relpath = svn_relpath_join(orig_repos_relpath, diff_relpath,
                                        result_pool);
  }
/* Adjust *INDEX_PATH, *ORIG_PATH_1 and *ORIG_PATH_2, representing the changed
 * node and the two original targets passed to the diff command, to handle the
 * directory the diff target should be considered relative to.
 * ANCHOR is the local path where the diff editor is anchored. The resulting
 * values are allocated in RESULT_POOL and temporary allocations are performed
 * in SCRATCH_POOL. */
adjust_paths_for_diff_labels(const char **index_path,
                             const char *anchor,
                             apr_pool_t *result_pool,
                             apr_pool_t *scratch_pool)
  const char *new_path = *index_path;
  if (anchor)
    new_path = svn_dirent_join(anchor, new_path, result_pool);
                                                   result_pool);
      else if (! strcmp(relative_to_dir, new_path))
      child_path = svn_dirent_is_child(relative_to_dir, new_path1,
                                       result_pool);
    }

  {
    apr_size_t len;
    svn_boolean_t is_url1;
    svn_boolean_t is_url2;
    /* ### Holy cow.  Due to anchor/target weirdness, we can't
       simply join diff_cmd_baton->orig_path_1 with path, ditto for
       orig_path_2.  That will work when they're directory URLs, but
       not for file URLs.  Nor can we just use anchor1 and anchor2
       from do_diff(), at least not without some more logic here.
       What a nightmare.

       For now, to distinguish the two paths, we'll just put the
       unique portions of the original targets in parentheses after
       the received path, with ellipses for handwaving.  This makes
       the labels a bit clumsy, but at least distinctive.  Better
       solutions are possible, they'll just take more thought. */

    /* ### BH: We can now just construct the repos_relpath, etc. as the
           anchor is available. See also make_repos_relpath() */

    is_url1 = svn_path_is_url(new_path1);
    is_url2 = svn_path_is_url(new_path2);

    if (is_url1 && is_url2)
      len = strlen(svn_uri_get_longest_ancestor(new_path1, new_path2,
                                                scratch_pool));
    else if (!is_url1 && !is_url2)
      len = strlen(svn_dirent_get_longest_ancestor(new_path1, new_path2,
                                                   scratch_pool));
    else
      len = 0; /* Path and URL */

    new_path1 += len;
    new_path2 += len;
  }
  /* ### Should diff labels print paths in local style?  Is there
     already a standard for this?  In any case, this code depends on
     a particular style, so not calling svn_dirent_local_style() on the
     paths below.*/
  if (new_path[0] == '\0')
    new_path = ".";
  if (new_path1[0] == '\0')
    new_path1 = new_path;
  else if (svn_path_is_url(new_path1))
    new_path1 = apr_psprintf(result_pool, "%s\t(%s)", new_path, new_path1);
  else if (new_path1[0] == '/')
    new_path1 = apr_psprintf(result_pool, "%s\t(...%s)", new_path, new_path1);
  else
    new_path1 = apr_psprintf(result_pool, "%s\t(.../%s)", new_path, new_path1);

  if (new_path2[0] == '\0')
    new_path2 = new_path;
  else if (svn_path_is_url(new_path2))
    new_path1 = apr_psprintf(result_pool, "%s\t(%s)", new_path, new_path2);
  else if (new_path2[0] == '/')
    new_path2 = apr_psprintf(result_pool, "%s\t(...%s)", new_path, new_path2);
  else
    new_path2 = apr_psprintf(result_pool, "%s\t(.../%s)", new_path, new_path2);

  *index_path = new_path;
                             const char *copyfrom_path,
                             svn_revnum_t copyfrom_rev,
                             const char *path,
  if (copyfrom_rev != SVN_INVALID_REVNUM)
    SVN_ERR(svn_stream_printf_from_utf8(os, header_encoding, result_pool,
                                        "copy from %s@%ld%s", copyfrom_path,
                                        copyfrom_rev, APR_EOL_STR));
  else
    SVN_ERR(svn_stream_printf_from_utf8(os, header_encoding, result_pool,
                                        "copy from %s%s", copyfrom_path,
                                        APR_EOL_STR));
 * revisions being diffed. COPYFROM_PATH and COPYFROM_REV indicate where the
 * diffed item was copied from.
                      svn_revnum_t copyfrom_rev,
                                           copyfrom_path, copyfrom_rev,
                                           repos_relpath2,
   to OUTSTREAM.   Of course, OUTSTREAM will probably be whatever was
   passed to svn_client_diff6(), which is probably stdout.
   ANCHOR is the local path where the diff editor is anchored. */
                   const char *diff_relpath,
                   const char *anchor,
                   svn_stream_t *outstream,
                   apr_pool_t *scratch_pool)
  const char *repos_relpath1 = NULL;
  const char *repos_relpath2 = NULL;
  const char *index_path = diff_relpath;
  const char *adjusted_path1 = orig_path1;
  const char *adjusted_path2 = orig_path2;
      SVN_ERR(make_repos_relpath(&repos_relpath1, diff_relpath, orig_path1,
                                 ra_session, wc_ctx, anchor,
                                 scratch_pool, scratch_pool));
      SVN_ERR(make_repos_relpath(&repos_relpath2, diff_relpath, orig_path2,
                                 ra_session, wc_ctx, anchor,
                                 scratch_pool, scratch_pool));
  SVN_ERR(adjust_paths_for_diff_labels(&index_path, &adjusted_path1,
                                       &adjusted_path2,
                                       relative_to_dir, anchor,
                                       scratch_pool, scratch_pool));
      label1 = diff_label(adjusted_path1, rev1, scratch_pool);
      label2 = diff_label(adjusted_path2, rev2, scratch_pool);
      SVN_ERR(svn_stream_printf_from_utf8(outstream, encoding, scratch_pool,
                                          "Index: %s" APR_EOL_STR
                                          SVN_DIFF__EQUAL_STRING APR_EOL_STR,
                                          index_path));
        SVN_ERR(print_git_diff_header(outstream, &label1, &label2,
                                      svn_diff_op_modified,
                                      repos_relpath1, repos_relpath2,
                                      rev1, rev2, NULL,
                                      SVN_INVALID_REVNUM,
                                      encoding, scratch_pool));

      /* --- label1
       * +++ label2 */
      SVN_ERR(svn_diff__unidiff_write_header(
        outstream, encoding, label1, label2, scratch_pool));
  SVN_ERR(svn_stream_printf_from_utf8(outstream, encoding, scratch_pool,
                                      _("%sProperty changes on: %s%s"),
                                      APR_EOL_STR,
                                      use_git_diff_format
                                            ? repos_relpath1
                                            : index_path,
                                      APR_EOL_STR));
  SVN_ERR(svn_stream_printf_from_utf8(outstream, encoding, scratch_pool,
                                      SVN_DIFF__UNDER_STRING APR_EOL_STR));
  SVN_ERR(svn_diff__display_prop_diffs(
            outstream, encoding, propchanges, original_props,
            TRUE /* pretty_print_mergeinfo */, scratch_pool));
  svn_stream_t *outstream;
  svn_stream_t *errstream;
     svn_client_diff6(), either may be SVN_INVALID_REVNUM.  We need these
  /* Whether property differences are ignored. */
  svn_boolean_t ignore_properties;

  /* Whether to show only property changes. */
  svn_boolean_t properties_only;

  /* Whether addition of a file is summarized versus showing a full diff. */
  svn_boolean_t no_diff_added;

  /* Whether deletion of a file is summarized versus showing a full diff. */
  svn_boolean_t no_diff_deleted;

  /* Whether to ignore copyfrom information when showing adds */
  svn_boolean_t no_copyfrom_on_add;

  /* Empty files for creating diffs or NULL if not used yet */
  const char *empty_file;

  /* Whether the local diff target of a repos->wc diff is a copy. */
  svn_boolean_t repos_wc_diff_target_is_copy;
diff_props_changed(const char *diff_relpath,
                   svn_revnum_t rev1,
                   svn_revnum_t rev2,
                   svn_boolean_t show_diff_header,
                   struct diff_cmd_baton *diff_cmd_baton,

  /* If property differences are ignored, there's nothing to do. */
  if (diff_cmd_baton->ignore_properties)
    return SVN_NO_ERROR;
      SVN_ERR(display_prop_diffs(props, original_props,
                                 diff_relpath,
                                 diff_cmd_baton->anchor,
                                 rev1,
                                 rev2,
                                 diff_cmd_baton->outstream,
                       const char *diff_relpath,
  return svn_error_trace(diff_props_changed(diff_relpath,
                                            /* ### These revs be filled
                                             * ### with per node info */
                                            dir_was_added
                                                ? 0 /* Magic legacy value */
                                                : diff_cmd_baton->revnum1,
                                            diff_cmd_baton->revnum2,
                                            TRUE /* show_diff_header */,
                                            diff_cmd_baton,
/* Show differences between TMPFILE1 and TMPFILE2. DIFF_RELPATH, REV1, and
   REV2 are used in the headers to indicate the file and revisions.  If either
   but instead print a warning message.

   If FORCE_DIFF is TRUE, always write a diff, even for empty diffs.

   Set *WROTE_HEADER to TRUE if a diff header was written */
diff_content_changed(svn_boolean_t *wrote_header,
                     const char *diff_relpath,
                     svn_boolean_t force_diff,
                     svn_revnum_t copyfrom_rev,
                     struct diff_cmd_baton *diff_cmd_baton,
                     apr_pool_t *scratch_pool)
  svn_stream_t *errstream = diff_cmd_baton->errstream;
  svn_stream_t *outstream = diff_cmd_baton->outstream;
  const char *index_path = diff_relpath;
  const char *path1 = diff_cmd_baton->orig_path_1;
  const char *path2 = diff_cmd_baton->orig_path_2;
  /* If only property differences are shown, there's nothing to do. */
  if (diff_cmd_baton->properties_only)
    return SVN_NO_ERROR;
  SVN_ERR(adjust_paths_for_diff_labels(&index_path, &path1, &path2,
                                       rel_to_dir, diff_cmd_baton->anchor,
                                       scratch_pool, scratch_pool));
  label1 = diff_label(path1, rev1, scratch_pool);
  label2 = diff_label(path2, rev2, scratch_pool);
      SVN_ERR(svn_stream_printf_from_utf8(outstream,
               diff_cmd_baton->header_encoding, scratch_pool,
               "Index: %s" APR_EOL_STR
               SVN_DIFF__EQUAL_STRING APR_EOL_STR,
               index_path));
      SVN_ERR(svn_stream_printf_from_utf8(outstream,
               diff_cmd_baton->header_encoding, scratch_pool,
        SVN_ERR(svn_stream_printf_from_utf8(outstream,
                 diff_cmd_baton->header_encoding, scratch_pool,
        SVN_ERR(svn_stream_printf_from_utf8(outstream,
                 diff_cmd_baton->header_encoding, scratch_pool,
            SVN_ERR(svn_stream_printf_from_utf8(outstream,
                     diff_cmd_baton->header_encoding, scratch_pool,
            SVN_ERR(svn_stream_printf_from_utf8(outstream,
                     diff_cmd_baton->header_encoding, scratch_pool,
      apr_file_t *outfile;
      apr_file_t *errfile;
      const char *outfilename;
      const char *errfilename;
      svn_stream_t *stream;

      SVN_ERR(svn_stream_printf_from_utf8(outstream,
               diff_cmd_baton->header_encoding, scratch_pool,
               "Index: %s" APR_EOL_STR
               SVN_DIFF__EQUAL_STRING APR_EOL_STR,
               index_path));
      /* We deal in streams, but svn_io_run_diff2() deals in file handles,
         so we may need to make temporary files and then copy the contents
         to our stream. */
      outfile = svn_stream__aprfile(outstream);
      if (outfile)
        outfilename = NULL;
      else
        SVN_ERR(svn_io_open_unique_file3(&outfile, &outfilename, NULL,
                                         svn_io_file_del_on_pool_cleanup,
                                         scratch_pool, scratch_pool));

      errfile = svn_stream__aprfile(errstream);
      if (errfile)
        errfilename = NULL;
      else
        SVN_ERR(svn_io_open_unique_file3(&errfile, &errfilename, NULL,
                                         svn_io_file_del_on_pool_cleanup,
                                         scratch_pool, scratch_pool));

                               &exitcode, outfile, errfile,
                               diff_cmd_baton->diff_cmd, scratch_pool));

      /* Now, open and copy our files to our output streams. */
      if (outfilename)
        {
          SVN_ERR(svn_io_file_close(outfile, scratch_pool));
          SVN_ERR(svn_stream_open_readonly(&stream, outfilename,
                                           scratch_pool, scratch_pool));
          SVN_ERR(svn_stream_copy3(stream, svn_stream_disown(outstream,
                                                             scratch_pool),
                                   NULL, NULL, scratch_pool));
        }
      if (errfilename)
        {
          SVN_ERR(svn_io_file_close(errfile, scratch_pool));
          SVN_ERR(svn_stream_open_readonly(&stream, errfilename,
                                           scratch_pool, scratch_pool));
          SVN_ERR(svn_stream_copy3(stream, svn_stream_disown(errstream,
                                                             scratch_pool),
                                   NULL, NULL, scratch_pool));
        }
      *wrote_header = TRUE;
                                   scratch_pool));
      if (force_diff
          || diff_cmd_baton->use_git_diff_format
          || svn_diff_contains_diffs(diff))
          SVN_ERR(svn_stream_printf_from_utf8(outstream,
                   diff_cmd_baton->header_encoding, scratch_pool,
                   "Index: %s" APR_EOL_STR
                   SVN_DIFF__EQUAL_STRING APR_EOL_STR,
                   index_path));
              const char *repos_relpath1;
              const char *repos_relpath2;
              SVN_ERR(make_repos_relpath(&repos_relpath1, diff_relpath,
                                         diff_cmd_baton->orig_path_1,
                                         diff_cmd_baton->ra_session,
                                         diff_cmd_baton->wc_ctx,
                                         diff_cmd_baton->anchor,
                                         scratch_pool, scratch_pool));
              SVN_ERR(make_repos_relpath(&repos_relpath2, diff_relpath,
                                         diff_cmd_baton->orig_path_2,
                                         diff_cmd_baton->ra_session,
                                         diff_cmd_baton->wc_ctx,
                                         diff_cmd_baton->anchor,
                                         scratch_pool, scratch_pool));
              SVN_ERR(print_git_diff_header(outstream, &label1, &label2,
                                            operation,
                                            repos_relpath1, repos_relpath2,
                                            rev1, rev2,
                                            copyfrom_rev,
                                            scratch_pool));
          if (force_diff || svn_diff_contains_diffs(diff))
            SVN_ERR(svn_diff_file_output_unified3(outstream, diff,
                     tmpfile1, tmpfile2, label1, label2,
                     scratch_pool));
          *wrote_header = TRUE;
                 const char *diff_relpath,
                  const char *diff_relpath,
  svn_boolean_t wrote_header = FALSE;

  /* During repos->wc diff of a copy revision numbers obtained
   * from the working copy are always SVN_INVALID_REVNUM. */
  if (diff_cmd_baton->repos_wc_diff_target_is_copy)
    {
      if (rev1 == SVN_INVALID_REVNUM &&
          diff_cmd_baton->revnum1 != SVN_INVALID_REVNUM)
        rev1 = diff_cmd_baton->revnum1;

      if (rev2 == SVN_INVALID_REVNUM &&
          diff_cmd_baton->revnum2 != SVN_INVALID_REVNUM)
        rev2 = diff_cmd_baton->revnum2;
    }

    SVN_ERR(diff_content_changed(&wrote_header, diff_relpath,
                                 svn_diff_op_modified, FALSE,
                                 NULL,
                                 SVN_INVALID_REVNUM, diff_cmd_baton,
                                 scratch_pool));
    SVN_ERR(diff_props_changed(diff_relpath, rev1, rev2, FALSE, prop_changes,
                               original_props, !wrote_header,
                               diff_cmd_baton, scratch_pool));
                const char *diff_relpath,
  svn_boolean_t wrote_header = FALSE;

  /* During repos->wc diff of a copy revision numbers obtained
   * from the working copy are always SVN_INVALID_REVNUM. */
  if (diff_cmd_baton->repos_wc_diff_target_is_copy)
    {
      if (rev1 == SVN_INVALID_REVNUM &&
          diff_cmd_baton->revnum1 != SVN_INVALID_REVNUM)
        rev1 = diff_cmd_baton->revnum1;
      if (rev2 == SVN_INVALID_REVNUM &&
          diff_cmd_baton->revnum2 != SVN_INVALID_REVNUM)
        rev2 = diff_cmd_baton->revnum2;
    }
  if (diff_cmd_baton->no_copyfrom_on_add
      && (copyfrom_path || SVN_IS_VALID_REVNUM(copyfrom_revision)))
    {
      apr_hash_t *empty_hash = apr_hash_make(scratch_pool);
      apr_array_header_t *new_changes;

      /* Rebase changes on having no left source. */
      if (!diff_cmd_baton->empty_file)
        SVN_ERR(svn_io_open_unique_file3(NULL, &diff_cmd_baton->empty_file,
                                         NULL, svn_io_file_del_on_pool_cleanup,
                                         diff_cmd_baton->pool, scratch_pool));

      SVN_ERR(svn_prop_diffs(&new_changes,
                             svn_prop__patch(original_props, prop_changes,
                                             scratch_pool),
                             empty_hash,
                             scratch_pool));

      tmpfile1 = diff_cmd_baton->empty_file;
      prop_changes = new_changes;
      original_props = empty_hash;
      copyfrom_revision = SVN_INVALID_REVNUM;
    }
  if (diff_cmd_baton->no_diff_added)
    {
      const char *index_path = diff_relpath;

      if (diff_cmd_baton->anchor)
        index_path = svn_dirent_join(diff_cmd_baton->anchor, diff_relpath,
                                     scratch_pool);

      SVN_ERR(svn_stream_printf_from_utf8(diff_cmd_baton->outstream,
                diff_cmd_baton->header_encoding, scratch_pool,
                "Index: %s (added)" APR_EOL_STR
                SVN_DIFF__EQUAL_STRING APR_EOL_STR,
                index_path));
      wrote_header = TRUE;
    }
  else if (tmpfile1 && copyfrom_path)
    SVN_ERR(diff_content_changed(&wrote_header, diff_relpath,
                                 svn_diff_op_copied,
                                 TRUE /* force diff output */,
                                 copyfrom_path,
                                 copyfrom_revision, diff_cmd_baton,
                                 scratch_pool));
    SVN_ERR(diff_content_changed(&wrote_header, diff_relpath,
                                 svn_diff_op_added,
                                 TRUE /* force diff output */,
                                 NULL, SVN_INVALID_REVNUM,
                                 diff_cmd_baton, scratch_pool));

    SVN_ERR(diff_props_changed(diff_relpath, rev1, rev2,
                               FALSE, prop_changes,
                               original_props, ! wrote_header,
                               diff_cmd_baton, scratch_pool));
diff_file_deleted(svn_wc_notify_state_t *state,
                  svn_boolean_t *tree_conflicted,
                  const char *diff_relpath,
                  const char *tmpfile1,
                  const char *tmpfile2,
                  const char *mimetype1,
                  const char *mimetype2,
                  apr_hash_t *original_props,
                  void *diff_baton,
                  apr_pool_t *scratch_pool)
  if (diff_cmd_baton->no_diff_deleted)
    {
      const char *index_path = diff_relpath;
      if (diff_cmd_baton->anchor)
        index_path = svn_dirent_join(diff_cmd_baton->anchor, diff_relpath,
                                     scratch_pool);
      SVN_ERR(svn_stream_printf_from_utf8(diff_cmd_baton->outstream,
                diff_cmd_baton->header_encoding, scratch_pool,
                "Index: %s (deleted)" APR_EOL_STR
                SVN_DIFF__EQUAL_STRING APR_EOL_STR,
                index_path));
    }
  else
    {
      svn_boolean_t wrote_header = FALSE;
      if (tmpfile1)
        SVN_ERR(diff_content_changed(&wrote_header, diff_relpath,
                                     tmpfile1, tmpfile2,
                                     diff_cmd_baton->revnum1,
                                     diff_cmd_baton->revnum2,
                                     mimetype1, mimetype2,
                                     svn_diff_op_deleted, FALSE,
                                     NULL, SVN_INVALID_REVNUM,
                                     diff_cmd_baton,
                                     scratch_pool));

      /* Should we also report the properties as deleted? */
    }
  /* We don't list all the deleted properties. */
               const char *diff_relpath,
                 const char *diff_relpath,
                const char *diff_relpath,
                const char *diff_relpath,
static const svn_wc_diff_callbacks4_t diff_callbacks =
{
  diff_file_opened,
  diff_file_changed,
  diff_file_added,
  diff_file_deleted,
  diff_dir_deleted,
  diff_dir_opened,
  diff_dir_added,
  diff_dir_props_changed,
  diff_dir_closed
};
      1. path is not a URL and start_revision != end_revision
      2. path is not a URL and start_revision == end_revision
      3. path is a URL and start_revision != end_revision
      4. path is a URL and start_revision == end_revision
      5. path is not a URL and no revisions given
   other.  When path is a URL there is no working copy. Thus
/** Check if paths PATH_OR_URL1 and PATH_OR_URL2 are urls and if the
 * revisions REVISION1 and REVISION2 are local. If PEG_REVISION is not
 * unspecified, ensure that at least one of the two revisions is not
 * BASE or WORKING.
 * If PATH_OR_URL1 can only be found in the repository, set *IS_REPOS1
 * to TRUE. If PATH_OR_URL2 can only be found in the repository, set
 * *IS_REPOS2 to TRUE. */
            const char *path_or_url1,
            const char *path_or_url2,
  /* Revisions can be said to be local or remote.
   * BASE and WORKING are local revisions.  */
  if (peg_revision->kind != svn_opt_revision_unspecified &&
      is_local_rev1 && is_local_rev2)
    return svn_error_create(SVN_ERR_CLIENT_BAD_REVISION, NULL,
                            _("At least one revision must be something other "
                              "than BASE or WORKING when diffing a URL"));
  /* Working copy paths with non-local revisions get turned into
     URLs.  We don't do that here, though.  We simply record that it
     needs to be done, which is information that helps us choose our
     diff helper function.  */
  *is_repos1 = ! is_local_rev1 || svn_path_is_url(path_or_url1);
  *is_repos2 = ! is_local_rev2 || svn_path_is_url(path_or_url2);

/* Return in *RESOLVED_URL the URL which PATH_OR_URL@PEG_REVISION has in
 * REVISION. If the object has no location in REVISION, set *RESOLVED_URL
 * to NULL. */
  err = svn_client__repos_locations(resolved_url, NULL,
                                    NULL, NULL,
                                    NULL,
/** Prepare a repos repos diff between PATH_OR_URL1 and
 * PATH_OR_URL2@PEG_REVISION, in the revision range REVISION1:REVISION2.
                         const char *path_or_url1,
                         const char *path_or_url2,
  const char *abspath_or_url2;
  const char *abspath_or_url1;
  const char *repos_root_url;
  const char *wri_abspath = NULL;
  if (!svn_path_is_url(path_or_url2))
    {
      SVN_ERR(svn_dirent_get_absolute(&abspath_or_url2, path_or_url2, pool));
      SVN_ERR(svn_wc__node_get_url(url2, ctx->wc_ctx, abspath_or_url2,
                                   pool, pool));
      wri_abspath = abspath_or_url2;
    }
    *url2 = abspath_or_url2 = apr_pstrdup(pool, path_or_url2);
  if (!svn_path_is_url(path_or_url1))
    {
      SVN_ERR(svn_dirent_get_absolute(&abspath_or_url1, path_or_url1, pool));
      SVN_ERR(svn_wc__node_get_url(url1, ctx->wc_ctx, abspath_or_url1,
                                   pool, pool));
      wri_abspath = abspath_or_url1;
    }
    *url1 = abspath_or_url1 = apr_pstrdup(pool, path_or_url1);
     calculated for PATH_OR_URL2 override the one for PATH_OR_URL1
     (since the diff will be "applied" to URL2 anyway). */
  if (strcmp(*url1, path_or_url1) != 0)
    *base_path = path_or_url1;
  if (strcmp(*url2, path_or_url2) != 0)
    *base_path = path_or_url2;
  SVN_ERR(svn_client_open_ra_session2(ra_session, *url2, wri_abspath,
                                      ctx, pool, pool));
                                             path_or_url2, peg_revision,
                                             path_or_url1, peg_revision,
       * show a diff that adds or removes the object (see issue #4153). */
           (path_or_url2 == *url2) ? NULL : abspath_or_url2,
           (strcmp(path_or_url1, *url1) == 0) ? NULL : abspath_or_url1,
                                 _("Diff targets '%s' and '%s' were not found "
  SVN_ERR(svn_ra_get_repos_root2(*ra_session, &repos_root_url, pool));

  /* If none of the targets is the repository root open the parent directory
     to allow describing replacement of the target itself */
  if (strcmp(*url1, repos_root_url) != 0
      && strcmp(*url2, repos_root_url) != 0)
      if (*base_path
          && (*kind1 == svn_node_file || *kind2 == svn_node_file))
   This function is really svn_client_diff6().  If you read the public
   API description for svn_client_diff6(), it sounds quite Grand.  It
   Since Subversion 1.8 we also have a variant of svn_wc_diff called
   svn_client__arbitrary_nodes_diff, that allows handling WORKING-WORKING
   comparisions between nodes in the working copy.

   pigeonholed into one of these use-cases, we currently bail with a
   friendly apology.
   Perhaps someday a brave soul will truly make svn_client_diff6()
                          _("Sorry, svn_client_diff6 was called in a way "
   All other options are the same as those passed to svn_client_diff6(). */
    return unsupported_diff_error(
       svn_error_create(SVN_ERR_INCORRECT_PARAMS, NULL,
                        _("Only diffs between a path's text-base "
                          "and its working files are supported at this time"
                          )));

  SVN_ERR(svn_wc_read_kind2(&kind, ctx->wc_ctx, abspath1,
                            TRUE, FALSE, pool));
   PATH_OR_URL1 and PATH_OR_URL2 may be either URLs or the working copy paths.
   If PEG_REVISION is specified, PATH_OR_URL2 is the path at the peg revision,
   history from PATH_OR_URL2.
   All other options are the same as those passed to svn_client_diff6(). */
                 const char *path_or_url1,
                 const char *path_or_url2,
  const svn_diff_tree_processor_t *diff_processor;

  const char *wri_abspath = NULL;
                                   ctx, path_or_url1, path_or_url2,
  /* Find a WC path for the ra session */
  if (!svn_path_is_url(path_or_url1))
    SVN_ERR(svn_dirent_get_absolute(&wri_abspath, path_or_url1, pool));
  else if (!svn_path_is_url(path_or_url2))
    SVN_ERR(svn_dirent_get_absolute(&wri_abspath, path_or_url2, pool));

  /* Set up the repos_diff editor on BASE_PATH, if available.
     Otherwise, we just use "". */

  SVN_ERR(svn_wc__wrap_diff_callbacks(&diff_processor,
                                      callbacks, callback_baton,
                                      TRUE /* walk_deleted_dirs */,
                                      pool, pool));

  /* The repository can bring in a new working copy, but not delete
     everything. Luckily our new diff handler can just be reversed. */
  if (kind2 == svn_node_none)
      const char *str_tmp;
      svn_revnum_t rev_tmp;

      str_tmp = url2;
      url2 = url1;
      url1 = str_tmp;

      rev_tmp = rev2;
      rev2 = rev1;
      rev1 = rev_tmp;

      str_tmp = anchor2;
      anchor2 = anchor1;
      anchor1 = str_tmp;

      str_tmp = target2;
      target2 = target1;
      target1 = str_tmp;

      diff_processor = svn_diff__tree_processor_reverse_create(diff_processor,
                                                               NULL, pool);
  /* Filter the first path component using a filter processor, until we fixed
     the diff processing to handle this directly */
  if ((kind1 != svn_node_file && kind2 != svn_node_file) && target1[0] != '\0')
  {
    diff_processor = svn_diff__tree_processor_filter_create(diff_processor,
                                                            target1, pool);
  }

  SVN_ERR(svn_client_open_ra_session2(&extra_ra_session, anchor1, wri_abspath,
                                      ctx, pool, pool));
  SVN_ERR(svn_client__get_diff_editor2(
                extra_ra_session, depth,
                rev1,
                TRUE /* text_deltas */,
                diff_processor,
                pool));
  SVN_ERR(svn_ra_do_diff3(ra_session, &reporter, &reporter_baton,
                          rev2, target1,
                          depth, ignore_ancestry, TRUE /* text_deltas */,
                          url2, diff_editor, diff_edit_baton, pool));
  return svn_error_trace(reporter->finish_report(reporter_baton, pool));
   PATH_OR_URL1 may be either a URL or a working copy path.  PATH2 is a
   If PEG_REVISION is specified, then PATH_OR_URL1 is the path in the peg
   All other options are the same as those passed to svn_client_diff6(). */
diff_repos_wc(const char *path_or_url1,
              void *callback_baton,
              struct diff_cmd_baton *cmd_baton,
              apr_pool_t *scratch_pool)
  apr_pool_t *pool = scratch_pool;
  const char *abspath_or_url1;
  svn_boolean_t is_copy;
  svn_revnum_t cf_revision;
  const char *cf_repos_relpath;
  const char *cf_repos_root_url;
  if (!svn_path_is_url(path_or_url1))
    {
      SVN_ERR(svn_dirent_get_absolute(&abspath_or_url1, path_or_url1, pool));
      SVN_ERR(svn_wc__node_get_url(&url1, ctx->wc_ctx, abspath_or_url1,
                                   pool, pool));
    }
    {
      url1 = path_or_url1;
      abspath_or_url1 = path_or_url1;
    }
  /* Convert path_or_url1 to a URL to feed to do_diff. */
  SVN_ERR_ASSERT(anchor_url != NULL);
      SVN_ERR(svn_client__repos_locations(&url1, NULL, NULL, NULL,
                                          path_or_url1,
                                          revision1, NULL,
          cmd_baton->orig_path_1 = url1;
          cmd_baton->orig_path_2 =
          cmd_baton->orig_path_1 =
          cmd_baton->orig_path_2 = url1;
  SVN_ERR(svn_client_open_ra_session2(&ra_session, url1, abspath2,
                                      ctx, pool, pool));
                                          (strcmp(path_or_url1, url1) == 0)
                                                    ? NULL : abspath_or_url1,
  SVN_ERR(svn_wc_read_kind2(&kind2, ctx->wc_ctx, abspath2,
                            TRUE, FALSE, pool));
  cmd_baton->ra_session = ra_session;
  cmd_baton->anchor = anchor;
    cmd_baton->revnum1 = rev;
    cmd_baton->revnum2 = rev;

  /* Check if our diff target is a copied node. */
  SVN_ERR(svn_wc__node_get_origin(&is_copy,
                                  &cf_revision,
                                  &cf_repos_relpath,
                                  &cf_repos_root_url,
                                  NULL, NULL,
                                  ctx->wc_ctx, abspath2,
                                  FALSE, pool, pool));

  /* Use the diff editor to generate the diff. */
  SVN_ERR(svn_wc__get_diff_editor(&diff_editor, &diff_edit_baton,
                                  ignore_ancestry || is_copy,
  SVN_ERR(svn_ra_reparent(ra_session, anchor_url, pool));
  if (is_copy)
    {
      const char *copyfrom_parent_url;
      const char *copyfrom_basename;
      svn_depth_t copy_depth;

      cmd_baton->repos_wc_diff_target_is_copy = TRUE;

      /* We're diffing a locally copied/moved node.
       * Describe the copy source to the reporter instead of the copy itself.
       * Doing the latter would generate a single add_directory() call to the
       * diff editor which results in an unexpected diff (the copy would
       * be shown as deleted). */

      if (cf_repos_relpath[0] == '\0')
        {
          copyfrom_parent_url = cf_repos_root_url;
          copyfrom_basename = "";
        }
      else
        {
          const char *parent_relpath;
          svn_relpath_split(&parent_relpath, &copyfrom_basename,
                            cf_repos_relpath, scratch_pool);

          copyfrom_parent_url = svn_path_url_add_component2(cf_repos_root_url,
                                                            parent_relpath,
                                                            scratch_pool);
        }
      SVN_ERR(svn_ra_reparent(ra_session, copyfrom_parent_url, pool));

      /* Tell the RA layer we want a delta to change our txn to URL1 */
      SVN_ERR(svn_ra_do_diff3(ra_session,
                              &reporter, &reporter_baton,
                              rev,
                              target,
                              diff_depth,
                              ignore_ancestry,
                              TRUE,  /* text_deltas */
                              url1,
                              diff_editor, diff_edit_baton, pool));

      /* Report the copy source. */
      SVN_ERR(svn_wc__node_get_depth(&copy_depth, ctx->wc_ctx, abspath2,
                                     pool));

      if (copy_depth == svn_depth_unknown)
        copy_depth = svn_depth_infinity;

      SVN_ERR(reporter->set_path(reporter_baton, "",
                                 cf_revision,
                                 copy_depth, FALSE, NULL, scratch_pool));

      if (strcmp(target, copyfrom_basename) != 0)
        SVN_ERR(reporter->link_path(reporter_baton, target,
                                    svn_path_url_add_component2(
                                                cf_repos_root_url,
                                                cf_repos_relpath,
                                                scratch_pool),
                                    cf_revision,
                                    copy_depth, FALSE, NULL, scratch_pool));

      /* Finish the report to generate the diff. */
      SVN_ERR(reporter->finish_report(reporter_baton, pool));
    }
  else
    {
      /* Tell the RA layer we want a delta to change our txn to URL1 */
      SVN_ERR(svn_ra_do_diff3(ra_session,
                              &reporter, &reporter_baton,
                              rev,
                              target,
                              diff_depth,
                              ignore_ancestry,
                              TRUE,  /* text_deltas */
                              url1,
                              diff_editor, diff_edit_baton, pool));

      /* Create a txn mirror of path2;  the diff editor will print
         diffs in reverse.  :-)  */
      SVN_ERR(svn_wc_crawl_revisions5(ctx->wc_ctx, abspath2,
                                      reporter, reporter_baton,
                                      FALSE, depth, TRUE,
                                      (! server_supports_depth),
                                      FALSE,
                                      ctx->cancel_func, ctx->cancel_baton,
                                      NULL, NULL, /* notification is N/A */
                                      pool));
    }
/* This is basically just the guts of svn_client_diff[_peg]6(). */
        const char *path_or_url1,
        const char *path_or_url2,
  SVN_ERR(check_paths(&is_repos1, &is_repos2, path_or_url1, path_or_url2,
          /* ### Ignores 'show_copies_as_adds'. */
                                   path_or_url1, path_or_url2,
                                   revision1, revision2,
      else /* path_or_url2 is a working copy path */
          SVN_ERR(diff_repos_wc(path_or_url1, revision1, peg_revision,
                                path_or_url2, revision2, FALSE, depth,
                                callbacks, callback_baton, callback_baton,
                                ctx, pool));
  else /* path_or_url1 is a working copy path */
          SVN_ERR(diff_repos_wc(path_or_url2, revision2, peg_revision,
                                path_or_url1, revision1, TRUE, depth,
                                callbacks, callback_baton, callback_baton,
                                ctx, pool));
      else /* path_or_url2 is a working copy path */
          if (revision1->kind == svn_opt_revision_working
              && revision2->kind == svn_opt_revision_working)
            {
              const char *abspath1;
              const char *abspath2;

              SVN_ERR(svn_dirent_get_absolute(&abspath1, path_or_url1, pool));
              SVN_ERR(svn_dirent_get_absolute(&abspath2, path_or_url2, pool));

              SVN_ERR(svn_client__arbitrary_nodes_diff(abspath1, abspath2,
                                                       depth,
                                                       callbacks,
                                                       callback_baton,
                                                       ctx, pool));
            }
          else
            SVN_ERR(diff_wc_wc(path_or_url1, revision1,
                               path_or_url2, revision2,
                               depth, ignore_ancestry, show_copies_as_adds,
                               use_git_diff_format, changelists,
                               callbacks, callback_baton, ctx, pool));
/* Perform a diff between a repository path and a working-copy path.

   PATH_OR_URL1 may be either a URL or a working copy path.  PATH2 is a
   working copy path.  REVISION1 and REVISION2 are their respective
   revisions.  If REVERSE is TRUE, the diff will be done in reverse.
   If PEG_REVISION is specified, then PATH_OR_URL1 is the path in the peg
   revision, and the actual repository path to be compared is
   determined by following copy history.

   All other options are the same as those passed to svn_client_diff6(). */
static svn_error_t *
diff_summarize_repos_wc(svn_client_diff_summarize_func_t summarize_func,
                        void *summarize_baton,
                        const char *path_or_url1,
                        const svn_opt_revision_t *revision1,
                        const svn_opt_revision_t *peg_revision,
                        const char *path2,
                        const svn_opt_revision_t *revision2,
                        svn_boolean_t reverse,
                        svn_depth_t depth,
                        svn_boolean_t ignore_ancestry,
                        const apr_array_header_t *changelists,
                        svn_client_ctx_t *ctx,
                        apr_pool_t *pool)
{
  const char *anchor, *target;
  svn_wc_diff_callbacks4_t *callbacks;
  void *callback_baton;
  struct diff_cmd_baton cmd_baton;

  SVN_ERR_ASSERT(! svn_path_is_url(path2));

  SVN_ERR(svn_wc_get_actual_target2(&anchor, &target,
                                    ctx->wc_ctx, path2,
                                    pool, pool));

  SVN_ERR(svn_client__get_diff_summarize_callbacks(
            &callbacks, &callback_baton, target, reverse,
            summarize_func, summarize_baton, pool));

  SVN_ERR(diff_repos_wc(path_or_url1, revision1, peg_revision,
                        path2, revision2, reverse,
                        depth, FALSE, TRUE, FALSE, changelists,
                        callbacks, callback_baton, &cmd_baton,
                        ctx, pool));
  return SVN_NO_ERROR;
}

/* Perform a summary diff between two working-copy paths.

   PATH1 and PATH2 are both working copy paths.  REVISION1 and
   REVISION2 are their respective revisions.

   All other options are the same as those passed to svn_client_diff6(). */
static svn_error_t *
diff_summarize_wc_wc(svn_client_diff_summarize_func_t summarize_func,
                     void *summarize_baton,
                     const char *path1,
                     const svn_opt_revision_t *revision1,
                     const char *path2,
                     const svn_opt_revision_t *revision2,
                     svn_depth_t depth,
                     svn_boolean_t ignore_ancestry,
                     const apr_array_header_t *changelists,
                     svn_client_ctx_t *ctx,
                     apr_pool_t *pool)
{
  svn_wc_diff_callbacks4_t *callbacks;
  void *callback_baton;
  const char *abspath1, *target1;
  svn_node_kind_t kind;

  SVN_ERR_ASSERT(! svn_path_is_url(path1));
  SVN_ERR_ASSERT(! svn_path_is_url(path2));

  /* Currently we support only the case where path1 and path2 are the
     same path. */
  if ((strcmp(path1, path2) != 0)
      || (! ((revision1->kind == svn_opt_revision_base)
             && (revision2->kind == svn_opt_revision_working))))
    return unsupported_diff_error
      (svn_error_create
       (SVN_ERR_INCORRECT_PARAMS, NULL,
        _("Summarized diffs are only supported between a path's text-base "
          "and its working files at this time")));

  /* Find the node kind of PATH1 so that we know whether the diff drive will
     be anchored at PATH1 or its parent dir. */
  SVN_ERR(svn_dirent_get_absolute(&abspath1, path1, pool));
  SVN_ERR(svn_wc_read_kind2(&kind, ctx->wc_ctx, abspath1,
                            TRUE, FALSE, pool));
  target1 = (kind == svn_node_dir) ? "" : svn_dirent_basename(path1, pool);
  SVN_ERR(svn_client__get_diff_summarize_callbacks(
            &callbacks, &callback_baton, target1, FALSE,
            summarize_func, summarize_baton, pool));

  SVN_ERR(svn_wc_diff6(ctx->wc_ctx,
                       abspath1,
                       callbacks, callback_baton,
                       depth,
                       ignore_ancestry, FALSE /* show_copies_as_adds */,
                       FALSE /* use_git_diff_format */, changelists,
                       ctx->cancel_func, ctx->cancel_baton,
                       pool));
  return SVN_NO_ERROR;
}
                           const char *path_or_url1,
                           const char *path_or_url2,
  const svn_diff_tree_processor_t *diff_processor;

  svn_wc_diff_callbacks4_t *callbacks;
  void *callback_baton;
                                   ctx, path_or_url1, path_or_url2,
  /* Set up the repos_diff editor. */
  SVN_ERR(svn_client__get_diff_summarize_callbacks(
            &callbacks, &callback_baton,
            target1, FALSE, summarize_func, summarize_baton, pool));

  SVN_ERR(svn_wc__wrap_diff_callbacks(&diff_processor,
                                      callbacks, callback_baton,
                                      TRUE /* walk_deleted_dirs */,
                                      pool, pool));


  /* The repository can bring in a new working copy, but not delete
     everything. Luckily our new diff handler can just be reversed. */
  if (kind2 == svn_node_none)
      const char *str_tmp;
      svn_revnum_t rev_tmp;

      str_tmp = url2;
      url2 = url1;
      url1 = str_tmp;

      rev_tmp = rev2;
      rev2 = rev1;
      rev1 = rev_tmp;

      str_tmp = anchor2;
      anchor2 = anchor1;
      anchor1 = str_tmp;

      str_tmp = target2;
      target2 = target1;
      target1 = str_tmp;

      diff_processor = svn_diff__tree_processor_reverse_create(diff_processor,
                                                               NULL, pool);
     location for URL1.  This is used to get deleted path information.  */
  SVN_ERR(svn_client_open_ra_session2(&extra_ra_session, anchor1, NULL,
                                      ctx, pool, pool));

  SVN_ERR(svn_client__get_diff_editor2(&diff_editor, &diff_edit_baton,
                                       extra_ra_session,
                                       depth,
                                       rev1,
                                       FALSE /* text_deltas */,
                                       diff_processor,
                                       ctx->cancel_func, ctx->cancel_baton,
                                       pool));
  return svn_error_trace(reporter->finish_report(reporter_baton, pool));
                  const char *path_or_url1,
                  const char *path_or_url2,
                  const apr_array_header_t *changelists,
  SVN_ERR(check_paths(&is_repos1, &is_repos2, path_or_url1, path_or_url2,
  if (is_repos1)
    {
      if (is_repos2)
        SVN_ERR(diff_summarize_repos_repos(summarize_func, summarize_baton, ctx,
                                           path_or_url1, path_or_url2,
                                           revision1, revision2,
                                           peg_revision, depth, ignore_ancestry,
                                           pool));
      else
        SVN_ERR(diff_summarize_repos_wc(summarize_func, summarize_baton,
                                        path_or_url1, revision1,
                                        peg_revision,
                                        path_or_url2, revision2,
                                        FALSE, depth,
                                        ignore_ancestry,
                                        changelists,
                                        ctx, pool));
    }
  else /* ! is_repos1 */
    {
      if (is_repos2)
        SVN_ERR(diff_summarize_repos_wc(summarize_func, summarize_baton,
                                        path_or_url2, revision2,
                                        peg_revision,
                                        path_or_url1, revision1,
                                        TRUE, depth,
                                        ignore_ancestry,
                                        changelists,
                                        ctx, pool));
      else
        {
          if (revision1->kind == svn_opt_revision_working
              && revision2->kind == svn_opt_revision_working)
           {
             const char *abspath1;
             const char *abspath2;
             svn_wc_diff_callbacks4_t *callbacks;
             void *callback_baton;
             const char *target;
             svn_node_kind_t kind;

             SVN_ERR(svn_dirent_get_absolute(&abspath1, path_or_url1, pool));
             SVN_ERR(svn_dirent_get_absolute(&abspath2, path_or_url2, pool));

             SVN_ERR(svn_io_check_resolved_path(abspath1, &kind, pool));

             if (kind == svn_node_dir)
               target = "";
             else
               target = svn_dirent_basename(path_or_url1, NULL);

             SVN_ERR(svn_client__get_diff_summarize_callbacks(
                     &callbacks, &callback_baton, target, FALSE,
                     summarize_func, summarize_baton, pool));

             SVN_ERR(svn_client__arbitrary_nodes_diff(abspath1, abspath2,
                                                      depth,
                                                      callbacks,
                                                      callback_baton,
                                                      ctx, pool));
           }
          else
            SVN_ERR(diff_summarize_wc_wc(summarize_func, summarize_baton,
                                         path_or_url1, revision1,
                                         path_or_url2, revision2,
                                         depth, ignore_ancestry,
                                         changelists, ctx, pool));
      }
    }

  return SVN_NO_ERROR;
      svn_config_t *cfg = svn_hash_gets(config, SVN_CONFIG_CATEGORY_CONFIG);
svn_client_diff6(const apr_array_header_t *options,
                 const char *path_or_url1,
                 const char *path_or_url2,
                 svn_boolean_t no_diff_added,
                 svn_boolean_t ignore_properties,
                 svn_boolean_t properties_only,
                 svn_stream_t *outstream,
                 svn_stream_t *errstream,
  svn_opt_revision_t peg_revision;

  if (ignore_properties && properties_only)
    return svn_error_create(SVN_ERR_INCORRECT_PARAMS, NULL,
                            _("Cannot ignore properties and show only "
                              "properties at the same time"));
  diff_cmd_baton.orig_path_1 = path_or_url1;
  diff_cmd_baton.orig_path_2 = path_or_url2;
  diff_cmd_baton.outstream = outstream;
  diff_cmd_baton.errstream = errstream;
  diff_cmd_baton.ignore_properties = ignore_properties;
  diff_cmd_baton.properties_only = properties_only;
  diff_cmd_baton.no_diff_added = no_diff_added;
  diff_cmd_baton.no_diff_deleted = no_diff_deleted;
  diff_cmd_baton.no_copyfrom_on_add = show_copies_as_adds;

                 path_or_url1, path_or_url2, revision1, revision2,
                 &peg_revision,
svn_client_diff_peg6(const apr_array_header_t *options,
                     const char *path_or_url,
                     svn_boolean_t no_diff_added,
                     svn_boolean_t ignore_properties,
                     svn_boolean_t properties_only,
                     svn_stream_t *outstream,
                     svn_stream_t *errstream,

  if (ignore_properties && properties_only)
    return svn_error_create(SVN_ERR_INCORRECT_PARAMS, NULL,
                            _("Cannot ignore properties and show only "
                              "properties at the same time"));
  diff_cmd_baton.orig_path_1 = path_or_url;
  diff_cmd_baton.orig_path_2 = path_or_url;
  diff_cmd_baton.outstream = outstream;
  diff_cmd_baton.errstream = errstream;
  diff_cmd_baton.ignore_properties = ignore_properties;
  diff_cmd_baton.properties_only = properties_only;
  diff_cmd_baton.no_diff_added = no_diff_added;
  diff_cmd_baton.no_diff_deleted = no_diff_deleted;
  diff_cmd_baton.no_copyfrom_on_add = show_copies_as_adds;

                 path_or_url, path_or_url, start_revision, end_revision,
                 peg_revision,
svn_client_diff_summarize2(const char *path_or_url1,
                           const char *path_or_url2,
                           path_or_url1, path_or_url2, revision1, revision2,
                           &peg_revision,
                           depth, ignore_ancestry, changelists, pool);
svn_client_diff_summarize_peg2(const char *path_or_url,
                           path_or_url, path_or_url,
                           start_revision, end_revision, peg_revision,
                           depth, ignore_ancestry, changelists, pool);