#include <catch2/catch_all.hpp>
// #include "../StepMotor.h"

SCENARIO("Should be an example","[StepMotor]")
{
    GIVEN("A StepMotor object.")
    {
        // StepMotor stepMotor;
        WHEN("reset is caled")
        {
            // const auto result = stepMotor.reset();

            THEN("result must be true")
            {
                CHECK(true);
            }
        }
    }
}