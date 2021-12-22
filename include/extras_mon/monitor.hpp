/**
 * @file Monitor.hpp
 * @author Perry Anderson (perry@exparx.com)
 * @brief MonitorInterface
 * @version 0.1
 * @date 2021-11-30
 *
 * @copyright (C) November 22, 2021 EXPARX INCORPORATED
 *
 */

#ifndef _EXTRAS_MONITOR_HPP
#define _EXTRAS_MONITOR_HPP

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
#include <extras/interfaces.hpp>

namespace extras {
    namespace cpp {

        /**
         * @brief MonitorInterface
         *
         */
        interface MonitorInterface
        {

            /**
             * @brief event
             * @note will be called from the trigger()
             */
            virtual void event() const pure;

            /**
             * @brief trigger()
             * @note will determine if the event needs to be called
             *
             */
            virtual void trigger() const pure;

        };

        /**
         * @brief Monitor
         *
         */
        concrete class Monitor implements MonitorInterface
        {

            virtual void event() const override;
            virtual void trigger() const override;

        };

        /**
         * @brief EndOfGameReachedException
         *
         */

        concrete class EndOfGameReachedException
            extends extras::AbstractCustomException {
        public:
            EndOfGameReachedException(
                const std::string& msg,
                const extras::WhereAmI& whereAmI)
                : AbstractCustomException(msg.c_str(),
                    whereAmI._file.c_str(),
                    whereAmI._func.c_str(), whereAmI._line) {}
            static void assertion(
                int sizePGN, int sizeFEN,
                const std::string& msg,
                const extras::WhereAmI& ref);
        };

    } // end namespace 
}

#endif // _EXTRAS_MONITOR_HPP
