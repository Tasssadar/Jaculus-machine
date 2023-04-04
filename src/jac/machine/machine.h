#pragma once

#include <quickjs.h>

#include <string>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <string.h>
#include "values.h"


namespace jac {


enum class EvalFlags : int {
    Global = JS_EVAL_TYPE_GLOBAL,
    Module = JS_EVAL_TYPE_MODULE,

    Strict = JS_EVAL_FLAG_STRICT,
    Strip = JS_EVAL_FLAG_STRIP,
    CompileOnly = JS_EVAL_FLAG_COMPILE_ONLY,
    BacktraceBarrier = JS_EVAL_FLAG_BACKTRACE_BARRIER
};

inline constexpr EvalFlags operator|(EvalFlags a, EvalFlags b) {
    int res = static_cast<int>(a) | static_cast<int>(b);
    if (res & JS_EVAL_TYPE_GLOBAL && res & JS_EVAL_TYPE_MODULE) {
        throw std::runtime_error("Cannot use both global and module eval flags");
    }
    return static_cast<EvalFlags>(res);
}

inline constexpr EvalFlags operator&(EvalFlags a, EvalFlags b) {
    return static_cast<EvalFlags>(static_cast<int>(a) & static_cast<int>(b));
}


class MachineBase;


class Module {
    ContextRef _ctx;
    JSModuleDef *_def;

    std::vector<std::tuple<std::string, Value>> exports;
public:
    Module(ContextRef ctx, std::string name);
    Module& operator=(const Module&) = delete;
    Module(const Module&) = delete;
    Module& operator=(Module&& other) {
        _ctx = other._ctx;
        _def = other._def;
        exports = std::move(other.exports);
        other._def = nullptr;
        return *this;
    }
    Module(Module&& other): _ctx(other._ctx), _def(other._def), exports(std::move(other.exports)) {
        other._def = nullptr;
    }

    void addExport(std::string name, Value val);

    JSModuleDef *get() {
        return _def;
    }

    inline MachineBase& base() {
        return *reinterpret_cast<MachineBase*>(JS_GetContextOpaque(_ctx));
    }

    static inline MachineBase& base(ContextRef ctx) {
        return *reinterpret_cast<MachineBase*>(JS_GetContextOpaque(ctx));
    }
};


class MachineBase {
private:
    std::unordered_map<JSModuleDef*, Module> _modules;
    Module& findModule(JSModuleDef* m);

    bool _interrupt = false;
public:
    JSRuntime* _runtime = nullptr;
    ContextRef _context = nullptr;

    void initialize();

    MachineBase() = default;
    MachineBase(const MachineBase&) = delete;
    MachineBase(MachineBase&&) = delete;
    MachineBase& operator=(const MachineBase&) = delete;
    MachineBase& operator=(MachineBase&&) = delete;

    virtual ~MachineBase() {
        _modules.clear();
        if (_context) {
            _context.free();
        }
        if (_runtime) {
            JS_FreeRuntime(_runtime);
        }
    }

    Value eval(std::string code, std::string filename, EvalFlags flags = EvalFlags::Global);

    void registerGlobal(std::string name, Value value, PropFlags flags = PropFlags::Enumerable);

    Module& newModule(std::string name);

    void interruptRuntime() {
        _interrupt = true;
    }

    friend class Module;
};


} // namespace jac
