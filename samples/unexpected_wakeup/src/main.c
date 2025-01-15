#include <stdio.h>
#include <zephyr/pm/pm.h>
#include <zephyr/kernel.h>

static volatile bool main_done;

static void on_state_entry(enum pm_state state)
{
}

static void on_state_exit(enum pm_state state)
{
	printf("wakeup @ %lld (post main: %d)\n", k_uptime_get(), main_done);
}

static struct pm_notifier state_notifier = {
	.state_entry = on_state_entry,
	.state_exit = on_state_exit,
};

static void my_work_handler(struct k_work *work)
{
	printf("my_work @ %lld\n", k_uptime_get());
}

int main(void)
{
	static struct k_work_delayable my_work;
	int status;

	printf("%s\n", CONFIG_BOARD_TARGET);
	printf("start @ %lld\n", k_uptime_get());

	pm_notifier_register(&state_notifier);
	k_work_init_delayable(&my_work, my_work_handler);

	k_work_schedule(&my_work, K_MSEC(2000));

	status = k_work_cancel_delayable(&my_work);
	printf("delayable work status after cancelling: 0x%08x\n", status);

	main_done = true;

	return 0;
}
