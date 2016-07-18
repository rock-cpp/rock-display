#include <boost/test/unit_test.hpp>
#include <rock-display/Dummy.hpp>

using namespace rock-display;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    rock-display::DummyClass dummy;
    dummy.welcome();
}
