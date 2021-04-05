/**
 * @brief Entry point for the Multicast Participant.
 */

#include <loguru.hpp>

#include "participant.h"
#include "tasks/console_task.h"

int main(int argc, char **argv) {
  char log_path[1024];

  // initialize participant logging
  loguru::init(argc, argv);
  loguru::add_file("participant_error.log", loguru::Append,
                   loguru::Verbosity_ERROR);
  loguru::add_file("participant_latest_error.log", loguru::Truncate,
                   loguru::Verbosity_ERROR);
  loguru::suggest_log_path("./logs", log_path, sizeof(log_path));
  // Turn off stderr logging for the client because it interferes with our CLI.
  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

  if (argc != 2) {
    std::cout << "\nIncorrect # of inputs. Configuration file expected";
    return 1;
  }

  participant::Participant participator = participant::Participant(argv[1]);
  participator.Start();

  return 0;
}
