/**
 * @brief the "MIT/X Consortium License", (adapted for EXPARX.COM)
 *
 * Copyright (C) November 22, 2021 EXPARX INCORPORATED
 *
 * Permission is hereby  granted,  free of charge,  to  any  person
 * obtaining a copy of this software and  associated  documentation
 * files   (the  "Software"),  to deal  in   the  Software  without
 * restriction, including  without  limitation the rights  to  use,
 * copy,  modify, merge,  publish,  distribute,  sublicense, and/or
 * sell copies of the  Software, and to permit persons  to whom the
 * Software  is  furnished to  do  so,  subject  to  the  following
 * conditions:
 *
 * (See LICENSE.md for complete details)
 *
 */

#include <iostream>
#include <extras_mon/monitor.hpp>

#include "../vendor/catch.hpp"
#include "../vendor/fakeit.hpp"

using namespace extras;
using namespace fakeit;

SCENARIO("Mock MonitorInterface", "[MonitorInterface]") {

    bool wasTriggered = false;
    bool wasExecuted = false;
    Mock<cpp::MonitorInterface> mock;
    When(Method(mock, event)).AlwaysDo([&wasExecuted]() {
        wasExecuted = true;
        });
    cpp::MonitorInterface& i = mock.get();
    When(Method(mock, trigger)).AlwaysDo([&wasTriggered, &i]() {
        wasTriggered = true;
        i.event();
        });

    REQUIRE(wasTriggered == false);
    REQUIRE(wasExecuted == false);
    i.trigger();
    REQUIRE(wasTriggered == true);
    REQUIRE(wasExecuted == true);
    Verify(Method(mock, event));
    Verify(Method(mock, trigger));
}