#pragma once

#include <spdlog/spdlog.h>
#include <iostream>
#include <exception>
#include <string>
#include <sstream>

class Debugger {
public:
    int steps_till_next_break = 0;


    enum debugOp {
        step,
        run,
        noOp
    };

    debugOp stringToOp(std::string str)
    {
        if (str == "step" || str == "s") return debugOp::step;
        if (str == "run" || str == "r") return debugOp::run;


        return debugOp::noOp;
    };

    void onBreakpoint()
    {
        //Get input
        std::cout << ">> ";
        std::string input;
        std::cin >> input;

        std::vector<std::string> tokens;
        std::stringstream inputStream(input);

        for (std::string token; std::getline(inputStream, token, ' '); )
        { 
            tokens.push_back(token);
            std::cout << token;
        } 

        std::cout << "Input recvd" << std::endl;
        //Manage input
        if (tokens.size() > 0)
        {
            auto op = stringToOp(tokens[0]);
            switch(op)
            {
                case debugOp::step:
                {
                    try
                    {
                        if (tokens.size() >= 2)
                        {
                            auto stepCount = std::stoi(tokens[1]);
                            setStep(stepCount);
                        }
                        else
                        {
                            std::cout << "No step count provided";
                        }
                    }
                    catch (std::exception& e)
                    {
                        std::cout << e.what() << '\n';
                    }
                    break;
                }
                case debugOp::run:
                {
                    setRun();
                    break;
                }
                case debugOp::noOp:
                {
                    std::cout << "unsupported debugger op" << std::endl;
                    break;
                }
            }


        }

    }

    void checkForBreakpoint()
    {
        if (steps_till_next_break == 0)
        {
            onBreakpoint();
            return;
        }
        else if (steps_till_next_break < 0)
        {
            return;
        }
        else
        {
            steps_till_next_break--;
        }
    }

    void setRun()
    {
        steps_till_next_break = -1;
    }

    void setStep(int x)
    {
        steps_till_next_break = x;
    }

};
