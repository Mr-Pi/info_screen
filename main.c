#include "main.h"

static char *TOPIC = "main";


int main(int argc, char *argv[]) {
	printf("\n");
	log_level = 5;
	log_date = true;
	log_color = false;
	do_stop = false;
	do_fork = false;
	LOG_INFO("Info Screen(https://github.com/Mr-Pi/info_screen) by Mr-Pi(contact@mr-pi.de) - Build: " __DATE__ " " __TIME__);

	parse_cmd(argc, argv);

	SetTraceLogLevel(LOG_WARNING);

	parse_config("config.json");

	if( do_fork ) {
		pid_t pid = fork();
		if( pid < 0 ) {
			LOG_FATAL("Failed to start daemon: %s", strerror(errno));
		}
		else if( pid ) {
			LOG_DEBUG("Master process finished");
			exit(EXIT_SUCCESS);
		}
		else {
			LOG_INFO("Daemon started");
		}
	}

	layout_init(config.layout);
	screen(NULL);
	//PTHREAD_CREATE(screen);
	//PTHREAD_JOIN(screen);

	printf("\nGood bye!\n");
	exit(EXIT_SUCCESS);
}
