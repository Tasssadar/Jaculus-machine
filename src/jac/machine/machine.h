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


class MachineBase;


class JSModule {
    ContextRef _ctx;
    JSModuleDef *_def;

    std::vector<std::tuple<std::string, Value>> exports;
public:
    // JSModule() = default;
    JSModule(ContextRef ctx, std::string name);

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
    std::unordered_map<JSModuleDef*, JSModule> _modules;
    JSModule& findModule(JSModuleDef* m);
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

    Value eval(std::string code, std::string filename, int eval_flags);

    void registerGlobal(std::string name, Value value);

    JSModule& newModule(std::string name);

    friend class JSModule;
};


} // namespace jac
