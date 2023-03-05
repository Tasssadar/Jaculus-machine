#pragma once

#include <jac/machine/values.h>
#include <jac/machine/machine.h>
#include <jac/machine/functionFactory.h>
#include <noal_func.h>

template<class Next>
class TestReportFeature : public Next {
    std::vector<std::string> _reports;
public:
    void report(std::string report) {
        _reports.push_back(report);
    }

    const std::vector<std::string>& getReports() {
        return _reports;
    }

    void initialize() {
        Next::initialize();
        jac::FunctionFactory ff(this->_context);

        this->registerGlobal("report", ff.newFunction([this](jac::ValueConst val) {
            this->report(val.to<std::string>());
        }));
    }
};


jac::Value evalFile(auto& machine, std::string path_) {
    try {
        return machine.evalFile(path_);
    }
    catch (jac::Exception& e) {
        std::string message(e.what());
        CAPTURE(message);
        REQUIRE(false);
    }

    return jac::Value::undefined(machine._context);
};


jac::Value evalCode(auto& machine, std::string code, std::string filename, int eval_flags) {
    try {
        return machine.eval(code, filename, eval_flags);
    }
    catch (jac::Exception& e) {
        std::string message(e.what());
        CAPTURE(message);
        REQUIRE(false);
    }

    return jac::Value::undefined(machine._context);
};



void evalFileThrows(auto& machine, std::string path_) {
    try {
        machine.evalFile(path_);
    }
    catch (jac::Exception& e) {
        return;
    }
    catch (std::exception& e) {
        std::string message(e.what());
        INFO("Expected jac::Exception, got \"" + message + "\"");
        REQUIRE(false);
    }
    catch (...) {
        INFO("Expected jac::Exception, got unknown exception");
        REQUIRE(false);
    }

    INFO("Expected jac::Exception, but no exception was thrown");
    REQUIRE(false);
};


void evalCodeThrows(auto& machine, std::string code, std::string filename, int eval_flags) {
    try {
        machine.eval(code, filename, eval_flags);
    }
    catch (jac::Exception& e) {
        return;
    }
    catch (std::exception& e) {
        std::string message(e.what());
        INFO("Expected jac::Exception, got \"" + message + "\"");
        REQUIRE(false);
    }
    catch (...) {
        INFO("Expected jac::Exception, got unknown exception");
        REQUIRE(false);
    }

    INFO("Expected jac::Exception, but no exception was thrown");
    REQUIRE(false);
};
