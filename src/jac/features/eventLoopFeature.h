#pragma once

#include <jac/machine/machine.h>
#include <jac/machine/functionFactory.h>
#include <noal_func.h>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

#include "eventLoopTerminal.h"


template<class Next>
class EventLoopFeature : public Next {
private:
    bool _running = false;
    bool _shouldExit = false;
    int _exitCode = 1;
public:

    void eventLoop_run() {
        _running = true;

        JSRuntime* rt = JS_GetRuntime(this->_context);
        JSContext* ctx1;
        int err;

        bool didJob = true;

        while (!_shouldExit) {
            runOnEventLoop();

            auto event = this->getEvent(!didJob);
            if (event) {
                (*event)();
            }
            else if (!didJob) {
                continue;
            }

            didJob = false;
            while (!_shouldExit) {
                err = JS_ExecutePendingJob(rt, &ctx1);
                if (err <= 0) {
                    if (err < 0) {
                        // js_std_dump_error(ctx1);
                        this->stdio.err->write(std::string("Error executing job: ") + std::to_string(err) + "\n");
                    }
                    break;
                }
                didJob = true;
            }
        }
        _running = false;
    }

    void eventLoop_stop() {
        _shouldExit = true;
        this->interruptRuntime();
        this->notifyEventLoop();
    }

    void eventLoop_exit(int code = 0) {
        _exitCode = code;
        eventLoop_stop();
    }

    int eventLoop_getExitCode() {
        return _exitCode;
    }

    virtual void runOnEventLoop() = 0;

    void initialize() {
        Next::initialize();
        jac::FunctionFactory ff(this->_context);
        this->registerGlobal("exit", ff.newFunction(noal::function(&EventLoopFeature::eventLoop_exit, this)));
    }

    void onEventLoop() {
        // last in stack
    }
};
