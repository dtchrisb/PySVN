//
// ====================================================================
// Copyright (c) 2003-2011 Barry A Scott.  All rights reserved.
//
// This software is licensed as described in the file LICENSE.txt,
// which you should have received as part of this distribution.
//
// ====================================================================
//
//
//  pysvn_enum_string.cpp
//
#if defined( _MSC_VER )
// disable warning C4786: symbol greater than 255 character,
// nessesary to ignore as <map> causes lots of warning
#pragma warning(disable: 4786)
#endif

#include "pysvn.hpp"

template <> EnumString< svn_opt_revision_kind >::EnumString()
: m_type_name( "opt_revision_kind" )
{
    // No revision information given.
    add( svn_opt_revision_unspecified, "unspecified" );

    // revision given as number
    add( svn_opt_revision_number, "number" );

    // revision given as date
    add( svn_opt_revision_date, "date" );

    // rev of most recent change
    add( svn_opt_revision_committed, "committed" );

    // (rev of most recent change) - 1
    add( svn_opt_revision_previous, "previous" );

    // .svn/entries current revision
    add( svn_opt_revision_base, "base" );

    // current, plus local mods
    add( svn_opt_revision_working, "working" );

    // repository youngest
    add( svn_opt_revision_head, "head" );
}

template <> EnumString< svn_wc_notify_action_t >::EnumString()
: m_type_name( "wc_notify_action" )
{
    // Adding a path to revision control.
    add( svn_wc_notify_add, "add" );

    // Copying a versioned path.
    add( svn_wc_notify_copy, "copy" );

    // Deleting a versioned path.
    add( svn_wc_notify_delete, "delete" );

    // Restoring a missing path from the pristine text-base.
    add( svn_wc_notify_restore, "restore" );

    // Reverting a modified path.
    add( svn_wc_notify_revert, "revert" );

    // A revert operation has failed.
    add( svn_wc_notify_failed_revert, "failed_revert" );

    // Resolving a conflict.
    add( svn_wc_notify_resolved, "resolved" );

    // Skipping a path.
    add( svn_wc_notify_skip, "skip" );

    // Got a delete in an update.
    add( svn_wc_notify_update_delete, "update_delete" );

    // Got an add in an update.
    add( svn_wc_notify_update_add, "update_add" );

    // Got any other action in an update.
    add( svn_wc_notify_update_update, "update_update" );

    // The last notification in an update (including updates of externals).
    add( svn_wc_notify_update_completed, "update_completed" );

    // Updating an external module.
    add( svn_wc_notify_update_external, "update_external" );

    // The last notification in a status (including status on externals).
    add( svn_wc_notify_status_completed, "status_completed" );

    // Running status on an external module.
    add( svn_wc_notify_status_external, "status_external" );

    // Committing a modification.
    add( svn_wc_notify_commit_modified, "commit_modified" );
 
    // Committing an addition.
    add( svn_wc_notify_commit_added, "commit_added" );

    // Committing a deletion.
    add( svn_wc_notify_commit_deleted, "commit_deleted" );

    // Committing a replacement.
    add( svn_wc_notify_commit_replaced, "commit_replaced" );

    // Transmitting post-fix text-delta data for a file.
    add( svn_wc_notify_commit_postfix_txdelta, "commit_postfix_txdelta" );

    // Processed a single revision's blame.
    add( svn_wc_notify_blame_revision, "annotate_revision" );

#if defined( PYSVN_HAS_CLIENT_LOCK )
    // Locking a path.
    add( svn_wc_notify_locked, "locked" );

    //Unlocking a path.
    add( svn_wc_notify_unlocked, "unlocked" );

    // Failed to lock a path.
    add( svn_wc_notify_failed_lock, "failed_lock" );

    // Failed to unlock a path.
    add( svn_wc_notify_failed_unlock, "failed_unlock" );
#endif
#if defined( PYSVN_HAS_SVN_WC_NOTIFY_ACTION_T__1_5 )
    add( svn_wc_notify_exists, "exists" );
    add( svn_wc_notify_changelist_set, "changelist_set" );
    add( svn_wc_notify_changelist_clear, "changelist_clear" );
    add( svn_wc_notify_changelist_moved, "changelist_moved" );
    add( svn_wc_notify_merge_begin, "merge_begin" );
    add( svn_wc_notify_foreign_merge_begin, "foreign_merge_begin" );
    add( svn_wc_notify_update_replace, "update_replace" );
#endif

#if defined( PYSVN_HAS_SVN_1_6 )
    add( svn_wc_notify_property_added, "property_added" );
    add( svn_wc_notify_property_modified, "property_modified" );
    add( svn_wc_notify_property_deleted, "property_deleted" );
    add( svn_wc_notify_property_deleted_nonexistent, "property_deleted_nonexistent" );
    add( svn_wc_notify_revprop_set, "revprop_set" );
    add( svn_wc_notify_revprop_deleted, "revprop_deleted" );
    add( svn_wc_notify_merge_completed, "merge_completed" );
    add( svn_wc_notify_tree_conflict, "tree_conflict" );
    add( svn_wc_notify_failed_external, "failed_external" );
#endif
#if defined( PYSVN_HAS_SVN_1_7 )
    add( svn_wc_notify_update_started, "update_started" );
    add( svn_wc_notify_update_skip_obstruction, "update_skip_obstruction" );
    add( svn_wc_notify_update_skip_working_only, "update_skip_working_only" );
    add( svn_wc_notify_update_external_removed, "update_external_removed" );
    add( svn_wc_notify_update_shadowed_add, "update_shadowed_add" );
    add( svn_wc_notify_update_shadowed_update, "update_shadowed_update" );
    add( svn_wc_notify_update_shadowed_delete, "update_shadowed_delete" );
    add( svn_wc_notify_merge_record_info, "merge_record_info" );
    add( svn_wc_notify_upgraded_path, "upgraded_path" );
    add( svn_wc_notify_merge_record_info_begin, "merge_record_info_begin" );
    add( svn_wc_notify_merge_elide_info, "merge_elide_info" );
    add( svn_wc_notify_patch, "patch" );
    add( svn_wc_notify_patch_applied_hunk, "patch_applied_hunk" );
    add( svn_wc_notify_patch_rejected_hunk, "patch_rejected_hunk" );
    add( svn_wc_notify_patch_hunk_already_applied, "patch_hunk_already_applied" );
    add( svn_wc_notify_commit_copied, "commit_copied" );
    add( svn_wc_notify_commit_copied_replaced, "commit_copied_replaced" );
    add( svn_wc_notify_url_redirect, "url_redirect" );
    add( svn_wc_notify_path_nonexistent, "path_nonexistent" );
    add( svn_wc_notify_exclude, "exclude" );
    add( svn_wc_notify_failed_conflict, "failed_conflict" );
    add( svn_wc_notify_failed_missing, "failed_missing" );
    add( svn_wc_notify_failed_out_of_date, "failed_out_of_date" );
    add( svn_wc_notify_failed_no_parent, "failed_no_parent" );
    add( svn_wc_notify_failed_locked, "failed_locked" );
    add( svn_wc_notify_failed_forbidden_by_server, "failed_forbidden_by_server" );
    add( svn_wc_notify_skip_conflicted, "skip_conflicted" );
#endif
#if defined( PYSVN_HAS_SVN_1_8 )
    add( svn_wc_notify_update_broken_lock, "update_broken_lock" );
    add( svn_wc_notify_failed_obstruction, "failed_obstruction" );
    add( svn_wc_notify_conflict_resolver_starting, "conflict_resolver_starting" );
    add( svn_wc_notify_conflict_resolver_done, "conflict_resolver_done" );
    add( svn_wc_notify_left_local_modifications, "left_local_modifications" );
    add( svn_wc_notify_foreign_copy_begin, "foreign_copy_begin" );
    add( svn_wc_notify_move_broken, "move_broken" );
#endif
#if defined( PYSVN_HAS_SVN_1_9 )
    add( svn_wc_notify_cleanup_external, "cleanup_external" );
    add( svn_wc_notify_failed_requires_target, "failed_requires_target" );
    add( svn_wc_notify_info_external, "info_external" );
    add( svn_wc_notify_commit_finalizing, "commit_finalizing" );
#endif

}

template <> EnumString< svn_wc_status_kind >::EnumString()
: m_type_name( "wc_status_kind" )
{ 
    // does not exist
    add( svn_wc_status_none, "none" );

    // is not a versioned thing in this wc
    add( svn_wc_status_unversioned, "unversioned" );

    // exists, but uninteresting.
    add( svn_wc_status_normal, "normal" );

    // is scheduled for addition
    add( svn_wc_status_added, "added" );

    // under v.c., but is missing
    add( svn_wc_status_missing, "missing" );

    // scheduled for deletion
    add( svn_wc_status_deleted, "deleted" );

    // was deleted and then re-added
    add( svn_wc_status_replaced, "replaced" );

    // text or props have been modified
    add( svn_wc_status_modified, "modified" );

    // local mods received repos mods
    add( svn_wc_status_merged, "merged" );

    // local mods received conflicting repos mods
    add( svn_wc_status_conflicted, "conflicted" );

    // a resource marked as ignored
    add( svn_wc_status_ignored, "ignored" );

    // an unversioned resource is in the way of the versioned resource
    add( svn_wc_status_obstructed, "obstructed" );

    // an unversioned path populated by an svn:external property
    add( svn_wc_status_external, "external" );

    // a directory doesn't contain a complete entries list 
    add( svn_wc_status_incomplete, "incomplete" );
}

template <> EnumString< svn_wc_merge_outcome_t >::EnumString()
: m_type_name( "wc_merge_outcome" )
{ 
    // The working copy is (or would be) unchanged.
    // The changes to be merged were already present in the working copy
    add( svn_wc_merge_unchanged, "unchanged" );

    // The working copy has been (or would be) changed.
    add( svn_wc_merge_merged, "merged" );

    // The working copy has been (or would be) changed,
    // but there was (or would be) a conflict
    add( svn_wc_merge_conflict, "conflict" );

    // No merge was performed, probably because the target
    // file was either absent or not under version control.
    add( svn_wc_merge_no_merge, "no_merge" );
}

template <> EnumString< svn_wc_notify_state_t >::EnumString()
: m_type_name( "wc_notify_state" )
{
    add( svn_wc_notify_state_inapplicable, "inapplicable" );

    // Notifier doesn't know or isn't saying.
    add( svn_wc_notify_state_unknown, "unknown" );

    // The state did not change.
    add( svn_wc_notify_state_unchanged, "unchanged" );

    // The item wasn't present.
    add( svn_wc_notify_state_missing, "missing" );

    // An unversioned item obstructed work.
    add( svn_wc_notify_state_obstructed, "obstructed" );

    // Pristine state was modified.
    add( svn_wc_notify_state_changed, "changed" );

    // Modified state had mods merged in.
    add( svn_wc_notify_state_merged, "merged" );

    // Modified state got conflicting mods.
    add( svn_wc_notify_state_conflicted, "conflicted" );

    // QQQ: When was this symbol added?
    // The source to copy the file from is missing.
    add( svn_wc_notify_state_source_missing, "source_missing" );
}

template <> EnumString< svn_wc_schedule_t >::EnumString()
: m_type_name( "wc_schedule" )
{
    // Nothing special here
    add( svn_wc_schedule_normal, "normal" );

    // Slated for addition
    add( svn_wc_schedule_add, "add" );

    // Slated for deletion
    add( svn_wc_schedule_delete, "delete" );

    // Slated for replacement (delete + add)
    add( svn_wc_schedule_replace, "replace" );
}

template <> EnumString< svn_node_kind_t >::EnumString()
: m_type_name( "node_kind" )
{
    // absent
    add( svn_node_none, "none" );

    // regular file
    add( svn_node_file, "file" );

    // directory
    add( svn_node_dir, "dir" );

    // something's here, but we don't know what
    add( svn_node_unknown, "unknown" );

#if defined( PYSVN_HAS_SVN_1_8 )
   // symbolic link
   // @note This value is not currently used by the public API.
  add( svn_node_symlink, "symlink" );
#endif
}

#if defined( PYSVN_HAS_DIFF_FILE_IGNORE_SPACE )
template <> EnumString< svn_diff_file_ignore_space_t >::EnumString()
: m_type_name( "diff_file_ignore_space" )
{
    add( svn_diff_file_ignore_space_none, "none" );
    add( svn_diff_file_ignore_space_change, "change" );
    add( svn_diff_file_ignore_space_all, "all" );
}
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
template <> EnumString< svn_client_diff_summarize_kind_t >::EnumString()
: m_type_name( "diff_summarize" )
{
    /** An item with no text modifications */
    add( svn_client_diff_summarize_kind_normal, "normal" );

    /** An added item */
    add( svn_client_diff_summarize_kind_added, "added" );

    /** An item with text modifications */
    add( svn_client_diff_summarize_kind_modified, "modified" );

    /** A deleted item */
    add( svn_client_diff_summarize_kind_deleted, "delete" );
}
#endif

#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
template <> EnumString< svn_wc_conflict_action_t >::EnumString()
: m_type_name( "conflict_action" )
{
    add( svn_wc_conflict_action_edit, "edit" );
    add( svn_wc_conflict_action_add, "add" );
    add( svn_wc_conflict_action_delete, "delete" );
}

template <> EnumString< svn_wc_conflict_kind_t >::EnumString()
: m_type_name( "conflict_kind" )
{
    add( svn_wc_conflict_kind_text, "text" );
    add( svn_wc_conflict_kind_property, "property" );
}

template <> EnumString< svn_wc_conflict_reason_t >::EnumString()
: m_type_name( "conflict_reason" )
{
    // local edits are already present
    add( svn_wc_conflict_reason_edited, "edited" );
    // another object is in the way
    add( svn_wc_conflict_reason_obstructed, "obstructed" );
    // object is already schedule-delete
    add( svn_wc_conflict_reason_deleted, "deleted" );
    // object is unknown or missing
    add( svn_wc_conflict_reason_missing, "missing" );
    // object is unversioned
    add( svn_wc_conflict_reason_unversioned, "unversioned" );
#if defined( PYSVN_HAS_SVN_1_8 )
    add( svn_wc_conflict_reason_moved_away, "moved_away" );
    add( svn_wc_conflict_reason_moved_here, "moved_here" );
#endif
}

#endif

#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
template <> EnumString< svn_depth_t >::EnumString()
: m_type_name( "depth" )
{
    add( svn_depth_unknown, "unknown" );
    add( svn_depth_exclude, "exclude" );
    add( svn_depth_empty, "empty" );
    add( svn_depth_files, "files" );
    add( svn_depth_immediates, "immediates" );
    add( svn_depth_infinity, "infinity" );
}
#endif


#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
template <> EnumString< svn_wc_conflict_choice_t >::EnumString()
: m_type_name( "wc_conflict_choice" )
{
    // Don't resolve the conflict now.  Let libsvn_wc mark the path
    // 'conflicted', so user can run 'svn resolved' later.
    add( svn_wc_conflict_choose_postpone, "postpone" );

    // If their were files to choose from, select one as a way of
    // resolving the conflict here and now.  libsvn_wc will then do the
    // work of "installing" the chosen file.
    // user chooses the original version
    add( svn_wc_conflict_choose_base, "base" );
    // user chooses incoming version
    add( svn_wc_conflict_choose_theirs_full, "theirs_full" );
    // user chooses own version
    add( svn_wc_conflict_choose_mine_full, "mine_full" );
    // user chooses incoming (for conflicted hunks)
    add( svn_wc_conflict_choose_theirs_conflict, "theirs_conflict" );
    // user chooses own (for conflicted hunks)
    add( svn_wc_conflict_choose_mine_conflict, "mine_conflict" );
    // user chooses the merged version
    add( svn_wc_conflict_choose_merged, "merged" );
#if defined( PYSVN_HAS_SVN_1_8 )
    //undecided
    add( svn_wc_conflict_choose_unspecified, "unspecified" );
#endif
}
#endif

#if defined( PYSVN_HAS_SVN_WC_OPERATION_T )
template <> EnumString< svn_wc_operation_t >::EnumString()
: m_type_name( "wc_operation" )
{
    add( svn_wc_operation_none, "none" );
    add( svn_wc_operation_update, "update" );
    add( svn_wc_operation_switch, "switch" );
    add( svn_wc_operation_merge, "merge" );
}
#endif

//--------------------------------------------------------------------------------
template <> void pysvn_enum< svn_opt_revision_kind >::init_type(void)
{
    behaviors().name( "opt_revision_kind" );
    behaviors().doc( "opt_revision_kind enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_opt_revision_kind >::init_type(void)
{
    behaviors().name( "opt_revision_kind" );
    behaviors().doc( "opt_revision_kind value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_notify_action_t >::init_type(void)
{
    behaviors().name( "wc_notify_action" );
    behaviors().doc( "wc_notify_action enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_notify_action_t >::init_type(void)
{
    behaviors().name( "wc_notify_action" );
    behaviors().doc( "wc_notify_action value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_status_kind >::init_type(void)
{
    behaviors().name( "wc_status_kind" );
    behaviors().doc( "wc_status_kind enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_status_kind >::init_type(void)
{
    behaviors().name( "wc_status_kind" );
    behaviors().doc( "wc_status_kind value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_schedule_t >::init_type(void)
{
    behaviors().name( "wc_schedule" );
    behaviors().doc( "wc_schedule enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_schedule_t >::init_type(void)
{
    behaviors().name( "wc_schedule" );
    behaviors().doc( "wc_schedule value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_merge_outcome_t >::init_type(void)
{
    behaviors().name( "wc_merge_outcome" );
    behaviors().doc( "wc_merge_outcome enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_merge_outcome_t >::init_type(void)
{
    behaviors().name( "wc_merge_outcome" );
    behaviors().doc( "wc_merge_outcome value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_notify_state_t >::init_type(void)
{
    behaviors().name( "wc_notify_state" );
    behaviors().doc( "wc_notify_state enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_notify_state_t >::init_type(void)
{
    behaviors().name( "wc_notify_state" );
    behaviors().doc( "wc_notify_state value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_node_kind_t >::init_type(void)
{
    behaviors().name( "node_kind" );
    behaviors().doc( "node_kind enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_node_kind_t >::init_type(void)
{
    behaviors().name( "node_kind" );
    behaviors().doc( "node_kind value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

#if defined( PYSVN_HAS_DIFF_FILE_IGNORE_SPACE )
template <> void pysvn_enum< svn_diff_file_ignore_space_t >::init_type(void)
{
    behaviors().name( "diff_file_ignore_space" );
    behaviors().doc( "diff_file_ignore_space enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_diff_file_ignore_space_t >::init_type(void)
{
    behaviors().name( "diff_file_ignore_space" );
    behaviors().doc( "diff_file_ignore_space value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}
#endif

#if defined( PYSVN_HAS_CLIENT_DIFF_SUMMARIZE )
template <> void pysvn_enum< svn_client_diff_summarize_kind_t >::init_type(void)
{
    behaviors().name( "client_diff_summarize_kind" );
    behaviors().doc( "client_diff_summarize_kind enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_client_diff_summarize_kind_t >::init_type(void)
{
    behaviors().name( "client_diff_summarize_kind" );
    behaviors().doc( "client_diff_summarize_kind value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}
#endif

#if defined( PYSVN_HAS_SVN__DEPTH_PARAMETER )
template <> void pysvn_enum< svn_depth_t >::init_type(void)
{
    behaviors().name( "depth" );
    behaviors().doc( "depth enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_depth_t >::init_type(void)
{
    behaviors().name( "depth" );
    behaviors().doc( "depth value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}
#endif

#if defined( PYSVN_HAS_SVN_WC_CONFLICT_CHOICE_T )
template <> void pysvn_enum< svn_wc_conflict_choice_t >::init_type(void)
{
    behaviors().name( "wc_conflict_choice" );
    behaviors().doc( "wc_conflict_choice enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_conflict_choice_t >::init_type(void)
{
    behaviors().name( "wc_conflict_choice" );
    behaviors().doc( "wc_conflict_choice value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}


template <> void pysvn_enum< svn_wc_conflict_action_t >::init_type(void)
{
    behaviors().name( "wc_conflict_action" );
    behaviors().doc( "wc_conflict_action enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_conflict_action_t >::init_type(void)
{
    behaviors().name( "wc_conflict_action" );
    behaviors().doc( "wc_conflict_action value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}


template <> void pysvn_enum< svn_wc_conflict_kind_t >::init_type(void)
{
    behaviors().name( "wc_conflict_kind" );
    behaviors().doc( "wc_conflict_kind enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_conflict_kind_t >::init_type(void)
{
    behaviors().name( "wc_conflict_kind" );
    behaviors().doc( "wc_conflict_kind value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}

template <> void pysvn_enum< svn_wc_conflict_reason_t >::init_type(void)
{
    behaviors().name( "wc_conflict_reason" );
    behaviors().doc( "wc_conflict_reason enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_conflict_reason_t >::init_type(void)
{
    behaviors().name( "wc_conflict_reason" );
    behaviors().doc( "wc_conflict_reason value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}
#endif

#if defined( PYSVN_HAS_SVN_WC_OPERATION_T )
template <> void pysvn_enum< svn_wc_operation_t >::init_type(void)
{
    behaviors().name( "wc_operation" );
    behaviors().doc( "wc_operation enumeration" );
    behaviors().supportGetattr();
}

template <> void pysvn_enum_value< svn_wc_operation_t >::init_type(void)
{
    behaviors().name( "wc_operation" );
    behaviors().doc( "wc_operation value" );
    behaviors().supportCompare();
    behaviors().supportRichCompare();
    behaviors().supportRepr();
    behaviors().supportStr();
    behaviors().supportHash();
}
#endif
