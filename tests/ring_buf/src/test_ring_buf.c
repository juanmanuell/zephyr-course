/*
 * Ring Buffer Module - Homework Test Skeleton
 *
 * test_fresh_state is provided as a worked example. Fill in the remaining
 * 7 ZTEST bodies according to TEST_SPEC.md. Stubs call ztest_test_skip()
 * so the binary builds and runs cleanly before each test is implemented.
 *
 * Run:
 *   west twister -T tests/ring_buf -p native_sim
 */

#include <zephyr/ztest.h>
#include <errno.h>

#include "ring_buf.h"

/*
 * Shared before hook: every suite reinitialises the ring buffer with a
 * capacity of 4 so tests start from a clean, known state. Capacity 4 is
 * enough to exercise FIFO order (push 1, 2, 3) and overflow (full at 4).
 */
static void before(void *f)
{
	ARG_UNUSED(f);
	rb_init(4);
}

/*
 * ============================================================================
 * Test Suite: ring_buf_init
 *
 * Initial state and re-initialization behaviour.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_init, NULL, NULL, before, NULL, NULL);

/* PROVIDED — study this test before writing the rest. */
ZTEST(ring_buf_init, test_fresh_state)
{
	zassert_true(rb_is_empty(), "Fresh buffer must be empty");
	zassert_equal(rb_count(), 0, "Fresh buffer count must be 0");
}

ZTEST(ring_buf_init, test_reinit_clears_state)
{
	zassert_equal(rb_push(99), 0, "Push must succeed");
	zassert_equal(rb_count(), 1, "Count must be 1 after push");

	zassert_equal(rb_init(4), 0, "Reinit must succeed");

	zassert_true(rb_is_empty(), "Reinit buffer must be empty");
	zassert_equal(rb_count(), 0, "Reinit buffer count must be 0");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_push_pop
 *
 * Single push/pop round-trip, FIFO order, full error path.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_push_pop, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_push_pop, test_single_push_pop)
{
	int v = 0;

	zassert_equal(rb_push(42), 0, "Push must succeed");
	zassert_equal(rb_pop(&v), 0, "Pop must succeed");
	zassert_equal(v, 42, "Popped value must be 42, got %d", v);
	zassert_true(rb_is_empty(), "Buffer must be empty after pop");
}

ZTEST(ring_buf_push_pop, test_fifo_order)
{
	int v = 0;

	zassert_equal(rb_push(1), 0, "Push 1 must succeed");
	zassert_equal(rb_push(2), 0, "Push 2 must succeed");
	zassert_equal(rb_push(3), 0, "Push 3 must succeed");

	zassert_equal(rb_pop(&v), 0, "First pop must succeed");
	zassert_equal(v, 1, "First pop must yield 1, got %d", v);
	zassert_equal(rb_pop(&v), 0, "Second pop must succeed");
	zassert_equal(v, 2, "Second pop must yield 2, got %d", v);
	zassert_equal(rb_pop(&v), 0, "Third pop must succeed");
	zassert_equal(v, 3, "Third pop must yield 3, got %d", v);

	zassert_true(rb_is_empty(), "Buffer must be empty after three pops");
}

ZTEST(ring_buf_push_pop, test_push_full_returns_enospc)
{
	for (int i = 1; i <= 4; i++) {
		zassert_equal(rb_push(i), 0, "Push %d must succeed", i);
	}
	zassert_true(rb_is_full(), "Buffer must be full after four pushes");

	zassert_equal(rb_push(99), -ENOSPC, "Push on a full buffer must return -ENOSPC");
	zassert_equal(rb_count(), 4, "Rejected push must not consume a slot");
}

/*
 * ============================================================================
 * Test Suite: ring_buf_boundaries
 *
 * Peek semantics and NULL-pointer boundary conditions.
 * ============================================================================
 */
ZTEST_SUITE(ring_buf_boundaries, NULL, NULL, before, NULL, NULL);

ZTEST(ring_buf_boundaries, test_peek_does_not_consume)
{
	int v = 0;

	zassert_equal(rb_push(7), 0, "Push must succeed");

	zassert_equal(rb_peek(&v), 0, "First peek must succeed");
	zassert_equal(v, 7, "First peek must yield 7, got %d", v);

	v = 0;
	zassert_equal(rb_peek(&v), 0, "Second peek must succeed");
	zassert_equal(v, 7, "Second peek must yield 7, got %d", v);

	zassert_equal(rb_count(), 1, "Peek must not consume the element");
}

ZTEST(ring_buf_boundaries, test_pop_null_returns_einval)
{
	zassert_equal(rb_pop(NULL), -EINVAL, "Pop with a NULL pointer must return -EINVAL");
}

ZTEST(ring_buf_boundaries, test_is_full_after_fill)
{
	for (int i = 1; i <= 4; i++) {
		zassert_equal(rb_push(i), 0, "Push %d must succeed", i);
	}

	zassert_true(rb_is_full(), "Buffer must be full after four pushes");
	zassert_equal(rb_count(), 4, "Count must be 4 when full");
}
