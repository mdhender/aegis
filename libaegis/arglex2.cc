//
//      aegis - project change supervisor
//      Copyright (C) 1997-2006, 2008, 2009, 2012 Peter Miller
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 3 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program. If not, see
//      <http://www.gnu.org/licenses/>.
//

#include <libaegis/arglex2.h>


static arglex_table_ty argtab[] =
{
    { "-ANticipate", arglex_token_anticipate, },
    { "-ASk", arglex_token_interactive, },
    { "-AS_Needed", arglex_token_as_needed, },
    { "-AUTOmatic", arglex_token_automatic, },
    { "-Automatic_Merge", arglex_token_merge_automatic, },
    { "-BaseLine", arglex_token_baseline, },
    { "-BAse_RElative", arglex_token_base_relative, },
    { "-BRanch", arglex_token_branch, },
    { "-Build", arglex_token_build, },
    { "-Change", arglex_token_change, },
    { "-Change_Attributes", arglex_token_change_attributes, },
    { "-Change_Directory", arglex_token_change_directory, },
    { "-Change_Owner", arglex_token_change_owner, },
    { "-CLEan", arglex_token_clean, },
    { "-CLOne", arglex_token_clone, },
    { "-COMMent", arglex_token_reason, },
    { "-COMPATibility", arglex_token_compatibility, },
    { "-COMPress", arglex_token_compress, },
    { "-Not_COmpress", arglex_token_compress_not, },
    { "-Compression_Algorithm", arglex_token_compression_algorithm, },
    { "-CONFIGured", arglex_token_configured, },
    { "-Not_CONFIGured", arglex_token_configured_not, },
    { "-CoPy_file", arglex_token_copy_file, },
    { "-CoPy_file_Undo", arglex_token_copy_file_undo, },
    { "-CUrrent_RElative", arglex_token_current_relative, },
    { "-DIFference", arglex_token_difference, },
    { "-DIRectory", arglex_token_directory, },
    { "-DELta", arglex_token_delta, },
    { "-DELta_Date", arglex_token_delta_date, },
    { "-Delta_From_Change", arglex_token_delta_from_change, },
    { "-Delta_Name", arglex_token_delta_name, },
    { "-DELta_NUmber", arglex_token_delta, },
    { "-Delta_Time", arglex_token_delta_date, },
    { "-Descend_Project_Tree", arglex_token_project_recursive, },
    { "-Description_Only", arglex_token_description_only, },
    { "-Develop_Begin", arglex_token_develop_begin, },
    { "-Develop_Begin_Undo", arglex_token_develop_begin_undo, },
    { "-Develop_End", arglex_token_develop_end, },
    { "-Develop_End_Undo", arglex_token_develop_end_undo, },
    { "-Development_Directory", arglex_token_development_directory, },
    { "-Edit", arglex_token_edit, },
    { "-Edit_BackGround", arglex_token_edit_bg, },
    { "-File", arglex_token_file, },
    { "-File_ATtributes", arglex_token_file_attributes, },
    { "-Fix_ARchitectures", arglex_token_fix_architecture, },
    { "-FOrce", arglex_token_force, },
    { "-GrandParent", arglex_token_grandparent, },
    { "-GrandMother", arglex_token_grandparent, },
    { "-GrandFather", arglex_token_grandparent, },
    { "-INDependent", arglex_token_independent, },
    { "-Integrate_Begin", arglex_token_integrate_begin, },
    { "-Integrate_Begin_Undo", arglex_token_integrate_begin_undo, },
    { "-Integrate_FAIL", arglex_token_integrate_fail, },
    { "-Integrate_PASS", arglex_token_integrate_pass, },
    { "-Interactive", arglex_token_interactive, },
    { "-Keep", arglex_token_keep, },
    { "-LIBrary", arglex_token_library, },
    { "-LOg", arglex_token_log, },
    { "-List", arglex_token_list, },
    { "-MAJor", arglex_token_major, },
    { "-Make_Transparent", arglex_token_make_transparent, },
    { "-Make_Transparent_Undo", arglex_token_make_transparent_undo, },
    { "-MANual", arglex_token_manual, },
    { "-MAXimum", arglex_token_maximum, },
    { "-Merge_Automatic", arglex_token_merge_automatic, },
    { "-Merge_Not", arglex_token_merge_not, },
    { "-Merge_Only", arglex_token_merge_only, },
    { "-Mime_Header", arglex_token_mime_header, },
    { "-Not_Mime_Header", arglex_token_mime_header_not, },
    { "-MINImum", arglex_token_minimum, },
    { "-MINOr", arglex_token_minor, },
    { "-MoVe_file", arglex_token_move_file, },
    { "-MoVe_file_Undo", arglex_token_move_file_undo, },
    { "-New_Administrator", arglex_token_new_administrator, },
    { "-New_BRanch", arglex_token_new_branch, },
    { "-New_BRanch_Undo", arglex_token_new_branch_undo, },
    { "-New_Change", arglex_token_new_change, },
    { "-New_Change_Undo", arglex_token_new_change_undo, },
    { "-New_Developer", arglex_token_new_developer, },
    { "-New_File", arglex_token_new_file, },
    { "-New_File_Undo", arglex_token_new_file_undo, },
    { "-New_Integrator", arglex_token_new_integrator, },
    { "-New_Project", arglex_token_new_project, },
    { "-New_Project_Alias", arglex_token_project_alias_create, },
    { "-New_ReLeaSe", arglex_token_new_release, },
    { "-New_ReViewer", arglex_token_new_reviewer, },
    { "-New_Test", arglex_token_new_test, },
    { "-New_Test_Undo", arglex_token_new_test_undo, },
    { "-Not_Keep", arglex_token_keep_not, },
    { "-Not_Logging", arglex_token_nolog, },
    { "-Not_Merge", arglex_token_merge_not, },
    { "-Not_PAGer", arglex_token_pager_not, },
    { "-Not_PErsevere", arglex_token_persevere_not, },
    { "-Not_PROGress", arglex_token_progress_not, },
    { "-Assume_Symbolic_Links", arglex_token_symbolic_links_not, },
    { "-Only_Merge", arglex_token_merge_only, },
    { "-Output", arglex_token_output, },
    { "-Output_Directory", arglex_token_output_directory },
    { "-OverWriting", arglex_token_overwriting, },
    { "-PAGer", arglex_token_pager, },
    { "-Page_Headers", arglex_token_page_headers, },
    { "-Not_Page_Headers", arglex_token_page_headers_not, },
    { "-Page_Length", arglex_token_page_length, },
    { "-Page_Width", arglex_token_page_width, },
    { "-PErsevere", arglex_token_persevere, },
    { "-PROGress", arglex_token_progress, },
    { "-Project", arglex_token_project, },
    { "-Project_Attributes", arglex_token_project_attributes, },
    { "-Read_Only", arglex_token_read_only, },
    { "-REASon", arglex_token_reason, },
    { "-REGression", arglex_token_regression, },
    { "-ReMove_file", arglex_token_remove_file, },
    { "-ReMove_file_Undo", arglex_token_remove_file_undo, },
    { "-Remove_Administrator", arglex_token_remove_administrator, },
    { "-Remove_Developer", arglex_token_remove_developer, },
    { "-Remove_Integrator", arglex_token_remove_integrator, },
    { "-ReMove_PRoject", arglex_token_remove_project, },
    { "-Remove_Project_Alias", arglex_token_project_alias_remove, },
    { "-Remove_ReViewer", arglex_token_remove_reviewer, },
    { "-RePorT", arglex_token_report, },
    { "-REScind", arglex_token_rescind, },
    { "-Review_Begin", arglex_token_review_begin, },
    { "-Review_Begin_Undo", arglex_token_review_begin_undo, },
    { "-Review_FAIL", arglex_token_review_fail, },
    { "-Review_PASS", arglex_token_review_pass, },
    { "-Review_Pass_Undo", arglex_token_review_pass_undo, },
    { "-Roll_Back", arglex_token_rescind, },
    { "-Signed_Off_By", arglex_token_signed_off_by, },
    { "-Not_Signed_Off_By", arglex_token_signed_off_by_not, },
    { "-SUGgest", arglex_token_suggest, },
    { "-SUGgest_Limit", arglex_token_suggest_limit, },
    { "-SUGgest_Noise", arglex_token_suggest_noise, },
    { "-Verify_Symbolic_Links", arglex_token_symbolic_links, },
    { "-Tab_Width", arglex_token_tab_width, },
    { "-TEMplate", arglex_token_template, },
    { "-Not_TEMplate", arglex_token_template_not, },
    { "-TERse", arglex_token_terse, },
    { "-Test", arglex_token_test, },
    { "-TOuch", arglex_token_touch, },
    { "-Not_TOuch", arglex_token_touch_not, },
    { "-TRunk", arglex_token_trunk, },
    { "-UNChanged", arglex_token_unchanged, },
    { "-UNFormatted", arglex_token_unformatted, },
    { "-User", arglex_token_user, },
    { "-Universal_Unique_IDentifier", arglex_token_uuid, },
    { "-Not_Universal_Unique_IDentifier", arglex_token_uuid_not, },
    { "-Verbose", arglex_token_verbose, },
    { "-Wait", arglex_token_wait, },
    { "-Not_Wait", arglex_token_wait_not, },
    { "-WhiteOut", arglex_token_whiteout, },
    { "-Not_WhiteOut", arglex_token_whiteout_not, },
    ARGLEX_END_MARKER
};


void
arglex2_init(int argc, char **argv)
{
    arglex_init(argc, argv, argtab);
}


void
arglex2_init3(int argc, char **argv, arglex_table_ty *tp)
{
    if (!tp)
        arglex2_init(argc, argv);
    else
        arglex_init(argc, argv, arglex_table_catenate(argtab, tp));
}


void
arglex2_retable(arglex_table_ty *tp)
{
    if (!tp)
        arglex_retable(argtab);
    else
        arglex_retable(arglex_table_catenate(argtab, tp));
}


// vim: set ts=8 sw=4 et :
