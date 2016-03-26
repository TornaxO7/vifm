#include <stic.h>

#include <stdlib.h> /* free() */
#include <string.h> /* strdup() */

#include "../../src/cfg/config.h"
#include "../../src/utils/dynarray.h"
#include "../../src/utils/fs.h"
#include "../../src/utils/path.h"
#include "../../src/cmd_core.h"
#include "../../src/filelist.h"
#include "../../src/filtering.h"
#include "utils.h"

SETUP()
{
	curr_view = &lwin;
	other_view = &rwin;

	init_commands();

	cfg.slow_fs_list = strdup("");

	view_setup(&lwin);
	view_setup(&rwin);
}

TEARDOWN()
{
	reset_cmds();

	free(cfg.slow_fs_list);
	cfg.slow_fs_list = NULL;

	view_teardown(&lwin);
	view_teardown(&rwin);
}

TEST(sync_syncs_local_filter)
{
	other_view->curr_dir[0] = '\0';
	assert_true(change_directory(curr_view, ".") >= 0);
	populate_dir_list(curr_view, 0);
	local_filter_apply(curr_view, "a");

	assert_success(exec_commands("sync! location filters", curr_view,
				CIT_COMMAND));
	assert_string_equal("a", other_view->local_filter.filter.raw);
}

TEST(sync_syncs_filelist)
{
	char cwd[PATH_MAX];
	char test_data[PATH_MAX];

	assert_non_null(get_cwd(cwd, sizeof(cwd)));
	if(is_path_absolute(TEST_DATA_PATH))
	{
		snprintf(test_data, sizeof(test_data), "%s", TEST_DATA_PATH);
	}
	else
	{
		snprintf(test_data, sizeof(test_data), "%s/%s", cwd, TEST_DATA_PATH);
	}

	snprintf(curr_view->curr_dir, sizeof(curr_view->curr_dir),
			"%s/existing-files", test_data);
	flist_custom_start(curr_view, "test");
	flist_custom_add(curr_view, TEST_DATA_PATH "/existing-files/a");
	assert_true(flist_custom_finish(curr_view, 0) == 0);

	assert_success(exec_commands("sync! filelist", curr_view, CIT_COMMAND));

	assert_true(flist_custom_active(other_view));
	assert_int_equal(curr_view->list_rows, other_view->list_rows);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 filetype=c : */
