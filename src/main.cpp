#include "main.hpp"

#include <ostream>

#include <boost/stacktrace.hpp>
#include <spdlog/spdlog.h>

#include "app.hpp"
#include "config.hpp"

int main()
{
	spdlog::set_level(spdlog::level::debug);
    spdlog::info("TrainGame {}.{}.{}", TrainGame_VERSION_MAJOR, TrainGame_VERSION_MINOR, TrainGame_VERSION_PATCH);

	App app;
	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		spdlog::critical(e.what());
		spdlog::dump_backtrace();
		const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
		if (st) {
			std::ostringstream os;
			os << '\n' << *st;
			spdlog::critical(os.str());
	    }

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}