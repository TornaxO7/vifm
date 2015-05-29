#include <stic.h>

#include <stdint.h> /* uint64_t */
#include <stdio.h> /* remove() */

#include <unistd.h> /* F_OK access() */

#include "../../src/io/iop.h"
#include "../../src/io/ior.h"
#include "../../src/utils/fs.h"
#include "../../src/utils/utils.h"

#include "utils.h"

static int confirm_overwrite(io_args_t *args, const char src[],
		const char dst[]);
static int deny_overwrite(io_args_t *args, const char src[], const char dst[]);

static int confirm_called;

TEST(confirm_is_not_called_for_no_overwrite)
{
	create_empty_file("empty");

	{
		io_args_t args = {
			.arg1.src = "../read/two-lines",
			.arg2.dst = "empty",
			.arg3.crs = IO_CRS_FAIL,

			.confirm = &confirm_overwrite,
		};
		ioe_errlst_init(&args.result.errors);

		confirm_called = 0;
		assert_failure(ior_mv(&args));
		assert_int_equal(0, confirm_called);

		assert_true(args.result.errors.error_count != 0);
		ioe_errlst_free(&args.result.errors);
	}

	delete_file("empty");
}

TEST(confirm_is_called_for_overwrite)
{
	create_empty_file("empty");
	clone_file("../read/two-lines", "two-lines");

	{
		io_args_t args = {
			.arg1.src = "two-lines",
			.arg2.dst = "empty",
			.arg3.crs = IO_CRS_REPLACE_FILES,

			.confirm = &confirm_overwrite,
		};
		ioe_errlst_init(&args.result.errors);

		confirm_called = 0;
		assert_success(ior_mv(&args));
		assert_int_equal(1, confirm_called);

		assert_int_equal(0, args.result.errors.error_count);
	}

	delete_file("empty");
}

TEST(deny_to_overwrite_is_considered)
{
	create_empty_file("empty");

	{
		io_args_t args = {
			.arg1.src = "../read/two-lines",
			.arg2.dst = "empty",
			.arg3.crs = IO_CRS_REPLACE_FILES,

			.confirm = &deny_overwrite,
		};
		ioe_errlst_init(&args.result.errors);

		confirm_called = 0;
		assert_success(ior_mv(&args));
		assert_int_equal(1, confirm_called);

		assert_int_equal(0, args.result.errors.error_count);
	}

	assert_true(file_exists("../read/two-lines"));

	delete_file("empty");
}

static int
confirm_overwrite(io_args_t *args, const char src[], const char dst[])
{
	++confirm_called;
	return 1;
}

static int
deny_overwrite(io_args_t *args, const char src[], const char dst[])
{
	++confirm_called;
	return 0;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
