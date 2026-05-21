/*
** EPITECH PROJECT, 2026
** Plazza
** File description:
** Reception
*/

#include "plazza/reception/Reception.hpp"

#include <iostream>
#include <sstream>

#include "plazza/reception/Logger.hpp"
#include "plazza/reception/Shell.hpp"

namespace plazza::reception {

    Reception::Reception(float cookingMultiplier, std::size_t cooksPerKitchen,
                         std::size_t restockMs)
        : _cookingMultiplier(cookingMultiplier),
          _cooksPerKitchen(cooksPerKitchen),
          _restockMs(restockMs)
    {
    }

    int Reception::run()
    {
        Logger logger(_logPath, /*mirrorStdout=*/false);
        std::ostringstream banner;
        banner << "reception ready (multiplier=" << _cookingMultiplier
               << ", cooks=" << _cooksPerKitchen << ", restock_ms=" << _restockMs << ")";
        logger.info(banner.str());

        Shell shell(logger, std::cin, std::cout);
        const int code = shell.run();
        logger.info("reception shutting down");
        return code;
    }

}  // namespace plazza::reception
