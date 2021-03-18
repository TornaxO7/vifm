#include <stic.h>

#include <string.h> /* strncpy() */

#include "../../src/lua/vlua.h"
#include "../../src/ui/column_view.h"
#include "../../src/ui/fileview.h"
#include "../../src/ui/statusbar.h"
#include "../../src/ui/ui.h"
#include "../../src/utils/str.h"
#include "../../src/opt_handlers.h"

#include <test-utils.h>

static void column_line_print(const void *data, int column_id, const char buf[],
		size_t offset, AlignType align, const char full_column[]);

enum { MAX_WIDTH = 30 };

static vlua_t *vlua;
static char print_buffer[MAX_WIDTH + 1];

SETUP_ONCE()
{
	stub_colmgr();
}

SETUP()
{
	vlua = vlua_init();

	view_setup(&lwin);
	curr_view = &lwin;
}

TEARDOWN()
{
	vlua_finish(vlua);

	view_teardown(&lwin);

	columns_teardown();
}

TEST(bad_args)
{
	ui_sb_msg("");
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = nil,"
				                         " handler = nil })"));
	assert_true(ends_with(ui_sb_last(), ": `name` key is mandatory"));

	ui_sb_msg("");
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'NAME',"
				                         " handler = nil })"));
	assert_true(ends_with(ui_sb_last(), ": `handler` key is mandatory"));
}

TEST(bad_name)
{
	assert_success(vlua_run_string(vlua, "function handler() end"));

	ui_sb_msg("");
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = '',"
				                         " handler = handler })"));
	assert_true(ends_with(ui_sb_last(), ": View column name can't be empty"));

	ui_sb_msg("");
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'name',"
				                         " handler = handler })"));
	assert_true(ends_with(ui_sb_last(),
				": View column name must not start with a lower case Latin letter"));

	ui_sb_msg("");
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'A-A',"
				                         " handler = handler })"));
	assert_true(ends_with(ui_sb_last(),
				": View column name must not contain non-Latin characters"));
}

TEST(column_is_registered)
{
	assert_success(vlua_run_string(vlua, "function handler() end"));

	assert_int_equal(-1, vlua_map_viewcolumn(vlua, "Test"));

	ui_sb_msg("");
	assert_success(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'Test',"
				                         " handler = handler })"));
	assert_string_equal("true", ui_sb_last());

	assert_true(vlua_map_viewcolumn(vlua, "Test") != -1);
}

TEST(duplicate_name)
{
	assert_success(vlua_run_string(vlua, "function handler() end"));

	ui_sb_msg("");
	assert_success(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'Test',"
				                         " handler = handler })"));
	assert_failure(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'Test',"
				                         " handler = handler })"));
	assert_true(ends_with(ui_sb_last(),
				": View column with such name already exists: Test"));
}

TEST(columns_are_used)
{
	opt_handlers_setup();
	lwin.columns = columns_create();
	curr_stats.vlua = vlua;

	ui_sb_msg("");
	assert_success(vlua_run_string(vlua, "function err() func() end"));
	assert_success(vlua_run_string(vlua, "function noval() end"));
	assert_success(vlua_run_string(vlua, "function good(info)\n"
	                                     "  return info.entry.name\n"
	                                     "end"));
	assert_success(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'Err',"
				                         " handler = err })"));
	assert_string_equal("true", ui_sb_last());
	assert_success(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'NoVal',"
				                         " handler = noval })"));
	assert_string_equal("true", ui_sb_last());
	assert_success(vlua_run_string(vlua,
				"print(vifm.addcolumntype{ name = 'Good',"
				                         " handler = good })"));
	assert_string_equal("true", ui_sb_last());

	process_set_args("viewcolumns=10{Err},10{NoVal},10{Good}", 0, 1);

	dir_entry_t entry = { .name = "name", .origin = "origin" };
	column_data_t cdt = { .view = &lwin, .entry = &entry };

	columns_set_line_print_func(column_line_print);
	columns_format_line(lwin.columns, &cdt, MAX_WIDTH);
	assert_string_equal("     ERROR   NOVALUE      name", print_buffer);

	opt_handlers_teardown();
	columns_free(lwin.columns);
	lwin.columns = NULL;
	curr_stats.vlua = NULL;
}

static void
column_line_print(const void *data, int column_id, const char buf[],
		size_t offset, AlignType align, const char full_column[])
{
	strncpy(print_buffer + offset, buf, strlen(buf));
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
