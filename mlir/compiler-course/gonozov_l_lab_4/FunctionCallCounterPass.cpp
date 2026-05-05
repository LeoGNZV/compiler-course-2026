#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Tools/Plugins/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace mlir;

namespace {
class FunctionCallCounterPass
    : public PassWrapper<FunctionCallCounterPass, OperationPass<ModuleOp>> {
public:
  // Аргумент командной строки для вызова пасса
  StringRef getArgument() const final { return "FunctionCallCounterPass"; }

  // Описание пасса
  StringRef getDescription() const final {
    return "Counts amount of times it was called by other functions (func.func) in the module";
  }

  void runOnOperation() override {
    ModuleOp moduleOp = getOperation();

    // Словарь для хранения количества вызовов для каждой функции
    // Ключ - имя функции, значение - счётчик вызовов
    llvm::StringMap<int> callCounts;

    // Собираем все имена функций в модуле
    llvm::StringSet<> functionNames;
    moduleOp.walk([&](func::FuncOp funcOp) {
      functionNames.insert(funcOp.getName());
      callCounts[funcOp.getName()] = 0;
    });

    // Проходим по всем операциям вызова в модуле
    moduleOp.walk([&](func::CallOp callOp) {
      StringRef calleeName = callOp.getCallee();
      // Если вызываемая функция существует в нашем модуле
      if (functionNames.contains(calleeName)) {
        callCounts[calleeName]++;  // Увеличиваем счётчик для этой функции
      }
    });

    // Добавляем атрибут call_count к каждой функции
    moduleOp.walk([&](func::FuncOp funcOp) {
      StringRef funcName = funcOp.getName();
      int count = callCounts[funcName];

      // Создаём целочисленный атрибут (32-битное целое)
      IntegerAttr call_count = IntegerAttr::get(
          IntegerType::get(funcOp.getContext(), 32), count);

      // Добавляем атрибут к функции
      funcOp->setAttr("call_count", call_count);
    });

    // Подсчитываем общее количество операций в модуле
    int totalOps = 0;
    moduleOp.walk([&](Operation *op) { totalOps++; });
    llvm::outs() << "Количество операций: " << totalOps << '\n';
  }
};
} // namespace

MLIR_DECLARE_EXPLICIT_TYPE_ID(FunctionCallCounterPass)
MLIR_DEFINE_EXPLICIT_TYPE_ID(FunctionCallCounterPass)

mlir::PassPluginLibraryInfo getFunctionCallCounterPassPluginInfo() {
  return {MLIR_PLUGIN_API_VERSION, "FunctionCallCounterPass", "1.0",
          []() { mlir::PassRegistration<FunctionCallCounterPass>(); }};
}

extern "C" LLVM_ATTRIBUTE_WEAK mlir::PassPluginLibraryInfo
mlirGetPassPluginInfo() {
  return getFunctionCallCounterPassPluginInfo();
}
